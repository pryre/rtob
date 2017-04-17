#include "breezystm32.h"

#include "mavlink_receive.h"
#include "mavlink_system.h"
#include "safety.h"
#include "sensors.h"
#include "controller.h"

#include <stdio.h>

uint8_t _system_operation_control;
uint8_t _sensor_calibration;
mavlink_queue_t _lpq_port_0;
mavlink_queue_t _lpq_port_1;

command_input_t _command_input;
system_status_t _system_status;

static void communication_decode(uint8_t port, uint8_t c) {
	mavlink_message_t msg;
	mavlink_status_t status;

	// Try to get a new message
	if(mavlink_parse_char(port, c, &msg, &status)) {
		// Handle message
		switch(msg.msgid) {
			case MAVLINK_MSG_ID_HEARTBEAT: {
				//TODO: Log to make keep track of connected systems status'
				//LED0_TOGGLE;
				// E.g. read GCS heartbeat and go into
				// comm lost mode if timer times out
				break;
			}
			case MAVLINK_MSG_ID_PARAM_REQUEST_LIST: {
				if((mavlink_msg_param_request_list_get_target_system(&msg) == mavlink_system.sysid) &&
					(mavlink_msg_param_request_list_get_target_component(&msg) == mavlink_system.compid)) {
					//Set the new request flag
					if(port == MAVLINK_COMM_0) {
						_lpq_port_0.request_all_params = 0;
					} else if(port == MAVLINK_COMM_1) {
						_lpq_port_1.request_all_params = 0;
					}
				} //Else this message is for someone else

				break;
			}
			case MAVLINK_MSG_ID_PARAM_REQUEST_READ: {
				if((mavlink_msg_param_request_read_get_target_system(&msg) == mavlink_system.sysid) &&
					(mavlink_msg_param_request_read_get_target_component(&msg) == mavlink_system.compid)) {

					int16_t index = mavlink_msg_param_request_read_get_param_index(&msg);

					if(index < PARAMS_COUNT) {
						if(index == -1) {	//Parameter is specified with name
							char param_id[MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN + 1];
							mavlink_msg_param_request_read_get_param_id(&msg, param_id);
							index = lookup_param_id(param_id); //TODO: UNTESTED
						}

						mavlink_message_t msg_out;
						mavlink_prepare_param_value(&msg_out, index);

						lpq_queue_msg(port, &msg_out);
					}
				} //Else this message is for someone else

				break;
			}
			case MAVLINK_MSG_ID_PARAM_SET: {
				if((mavlink_msg_param_set_get_target_system(&msg) == mavlink_system.sysid) &&
					(mavlink_msg_param_set_get_target_component(&msg) == mavlink_system.compid)) {

					bool set_complete = false;

					char param_id[MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN + 1];
					mavlink_msg_param_set_get_param_id(&msg, param_id);
					param_id_t index = lookup_param_id(param_id);

					//TODO: Remember to mention in docs that the sent dat is entered as the type, regardless of what type it should be
					//TODO: Should probably have a safetly check for this though
					if(index < PARAMS_COUNT) { //If the ID is valid
						switch(mavlink_msg_param_set_get_param_type(&msg)) {
							case MAV_PARAM_TYPE_INT32: {
								union {
									float f;
									uint32_t i;
								} u;

								u.f = mavlink_msg_param_set_get_param_value(&msg);
								set_complete = set_param_by_name_int(param_id, u.i);

								break;
							}
							case MAV_PARAM_TYPE_REAL32: {
								float value = mavlink_msg_param_set_get_param_value(&msg);
								set_complete = set_param_fix16(index, fix16_from_float(value));

								break;
							}
							default:
								break;
						}

						if(set_complete) {
							mavlink_message_t msg_out;
							mavlink_prepare_param_value(&msg_out, index);
							lpq_queue_msg(port, &msg_out);
						}
					}
				} //Else this message is for someone else

				break;
			}
			case MAVLINK_MSG_ID_COMMAND_LONG: {
				//A command should always have an acknowledge
				bool need_ack = false;
				uint16_t command = mavlink_msg_command_long_get_command(&msg);
				uint8_t command_result = MAV_RESULT_FAILED;

				switch(command) {
					case MAV_CMD_PREFLIGHT_CALIBRATION: {
						if(_system_status.state == MAV_STATE_STANDBY) {
							_system_status.state = MAV_STATE_CALIBRATING;

							if((int)mavlink_msg_command_long_get_param1(&msg))
								_sensor_calibration |= SENSOR_CAL_GYRO;

							if((int)mavlink_msg_command_long_get_param2(&msg))
								_sensor_calibration |= SENSOR_CAL_MAG;

							if((int)mavlink_msg_command_long_get_param3(&msg))
								_sensor_calibration |= SENSOR_CAL_BARO;

							if((int)mavlink_msg_command_long_get_param4(&msg))
								_sensor_calibration |= SENSOR_CAL_RC;

							if((int)mavlink_msg_command_long_get_param5(&msg))
								_sensor_calibration |= SENSOR_CAL_ACCEL;

							if((int)mavlink_msg_command_long_get_param6(&msg))
								_sensor_calibration |= SENSOR_CAL_INTER;
						} else {	//We send the denied immidiately if we can't do it now
							command_result = MAV_RESULT_DENIED;
							need_ack = true;
						}


						break;
					}
					case MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES: {
						mavlink_message_t msg_out;
						mavlink_prepare_autopilot_version(&msg_out);
						lpq_queue_msg(port, &msg_out);

						break;
					}
					case MAV_CMD_PREFLIGHT_STORAGE: {
						need_ack = true;

						switch((int)mavlink_msg_command_long_get_param1(&msg)) {
							case 0:	//Read from flash
								if( read_params() ) {
									command_result = MAV_RESULT_ACCEPTED;
								} else {
									command_result = MAV_RESULT_FAILED;
								}

								break;

							//TODO: Writing to EEPROM doesn't really work after boot
							/*
							case 1:	//Write to flash

								if(write_params()) {
									command_result = MAV_RESULT_ACCEPTED;
								} else {
									command_result = MAV_RESULT_FAILED;
								}

								break;
							*/
							case 2:	//Reset to defaults
								set_param_defaults();
								command_result = MAV_RESULT_ACCEPTED;

								break;
							default://Not supported
								command_result = MAV_RESULT_UNSUPPORTED;
								break;
						}

						break;
					}
					case MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN: {
						need_ack = true;

						//TODO: Make sure mav is in preflight mode
						switch((int)mavlink_msg_command_long_get_param1(&msg)) {
							case 1:
								_system_operation_control = SYSTEM_OPERATION_REBOOT;
								command_result = MAV_RESULT_ACCEPTED;
								break;
							case 3:
								_system_operation_control = SYSTEM_OPERATION_REBOOT_BOOTLOADER;
								command_result = MAV_RESULT_ACCEPTED;
								break;
							default:
								command_result = MAV_RESULT_UNSUPPORTED;
								break;
						}

						break;
					}
					case MAV_CMD_COMPONENT_ARM_DISARM: {	//TODO: For some reason this doesn't return an acceptable result
						need_ack = true;
						command_result = MAV_RESULT_DENIED;

						if( (bool)mavlink_msg_command_long_get_param1(&msg) ) { //ARM
							if( safety_request_arm() )
								command_result = MAV_RESULT_ACCEPTED;
						} else { //DISARM
							if( safety_request_disarm() )
								command_result = MAV_RESULT_ACCEPTED;
						}
					}
					//TODO: Handle other cases?
					default: {
						need_ack = true;
						command_result = MAV_RESULT_UNSUPPORTED;
						break;
					}
				}

				if(need_ack) {
					mavlink_message_t msg_out;
					mavlink_prepare_command_ack(&msg_out, command, command_result);
					lpq_queue_msg(port, &msg_out);
				}

				break;
			}
			case MAVLINK_MSG_ID_SET_ATTITUDE_TARGET: {
				if( (mavlink_msg_set_attitude_target_get_target_system(&msg) == mavlink_system.sysid) &&
					(mavlink_msg_set_attitude_target_get_target_system(&msg) == mavlink_system.compid) ) {

					//TODO: Check timestamp was recent
					_command_input.input_mask = mavlink_msg_set_attitude_target_get_type_mask(&msg);

					_command_input.r = fix16_from_float(mavlink_msg_set_attitude_target_get_body_roll_rate(&msg));
					_command_input.p = fix16_from_float(mavlink_msg_set_attitude_target_get_body_pitch_rate(&msg));
					_command_input.y = fix16_from_float(mavlink_msg_set_attitude_target_get_body_yaw_rate(&msg));

					//TODO: Check this is correct
					float qt[4];
					if(mavlink_msg_set_attitude_target_get_q(&msg, &qt[0]) == 4) {
						_command_input.q.a = fix16_from_float(qt[0]);
						_command_input.q.b = fix16_from_float(qt[1]);
						_command_input.q.c = fix16_from_float(qt[2]);
						_command_input.q.d = fix16_from_float(qt[3]);
					}

					_command_input.T = fix16_from_float(mavlink_msg_set_attitude_target_get_thrust(&msg));

					safety_update_sensor(&_system_status.mavlink.offboard_control, 100);	//TODO: Use params here
				}

				break;
			}
			/*
			case MAVLINK_MSG_ID_TIMESYNC: {
				mavlink_timesync_t tsync;
				mavlink_msg_timesync_decode(&msg, &tsync);

				uint32_t now_ms = micros();

				//TODO: Should be in safety_check()?
				if( (now_ms - _sensors.clock.rt_sync_last) > 500000) {	//There hasn't been a sync in a while
					_sensors.clock.rt_offset_ns = 0;
					_sensors.clock.rt_drift = 1.0;
					_sensors.clock.rt_ts_last = 0;
					_sensors.clock.rt_tc_last = 0;
				}

				//Pulled from px4 firmware
				uint64_t now_ns = now_ms * 1000LL;
				uint64_t now_ns_corrected = now_ns * _sensors.clock.rt_drift;

				int64_t time_offset_new = _sensors.clock.rt_offset_ns;

				if (tsync.tc1 == 0) {
					mavlink_send_timesync(port, now_ns_corrected, tsync.ts1);
				} else if (tsync.tc1 > 0) {
					if( (_sensors.clock.rt_ts_last != 0) && (_sensors.clock.rt_ts_last != 0) ) {
						float drift = (float)(tsync.tc1 - _sensors.clock.rt_tc_last) / (float)(tsync.ts1 - _sensors.clock.rt_ts_last);
						_sensors.clock.rt_drift = sensors_clock_smooth_time_drift(_sensors.clock.rt_drift, drift);
					}

					_sensors.clock.rt_ts_last = tsync.ts1;
					_sensors.clock.rt_tc_last = tsync.tc1;

					int64_t offset_ns = (int64_t)(tsync.ts1 + now_ns_corrected - tsync.tc1 * 2) / 2;
					int64_t dt = _sensors.clock.rt_offset_ns - offset_ns;

					if ( abs(dt) > 10000000LL ) { // 10 millisecond skew
						time_offset_new = offset_ns;

						char text[MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN];
						snprintf(text, MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN, "[SENSOR] Hard setting skew: %0.9f", dt / 1e9);
						mavlink_queue_notice_broadcast( &text[0] );
					} else {
						//Filter the new time offset
						time_offset_new = sensors_clock_smooth_time_skew(_sensors.clock.rt_offset_ns, offset_ns);
					}
				}

				_sensors.clock.rt_offset_ns = time_offset_new;
				_sensors.clock.rt_sync_last = now_ms;

				break;
			}
			*/
			default:
				//TODO: Error?
				//Do nothing
				break;
		}
	}
}

void communication_receive(void) {
	//TODO: Have a check on Serial 0 for... all the same messages?
	//TODO: That would mean there is only a need to have 1 parse function, and pass the right port and buffer.

	if( get_param_int(PARAM_BAUD_RATE_0) > 0 )
		while( serialTotalRxBytesWaiting( Serial1 ) )
			if( serialTotalRxBytesWaiting( Serial1 ) )
				communication_decode( MAVLINK_COMM_0, serialRead(Serial1) );

	if( get_param_int(PARAM_BAUD_RATE_1) > 0 )
		while( serialTotalRxBytesWaiting( Serial2 ) )
			if( serialTotalRxBytesWaiting( Serial2 ) )
				communication_decode( MAVLINK_COMM_1, serialRead(Serial2) );

	//TODO: Update global packet drops counter
	//packet_drops += status.packet_rx_drop_count;
}
