#include "params.h"
#include "param_generator/param_gen.h"
#include "mavlink_system.h"
#include "mavlink_transmit.h"
#include "controller.h"
#include "pid_controller.h"
#include "fix16.h"

void set_param_defaults(void) {
	init_param_uint(PARAM_BOARD_REVISION, NAZE32_REV);
	init_param_uint(PARAM_VERSION_FIRMWARE, strtoll(GIT_VERSION_FLIGHT_STR, NULL, 16));
	init_param_uint(PARAM_VERSION_SOFTWARE, strtoll(GIT_VERSION_OS_STR, NULL, 16));
	init_param_uint(PARAM_BAUD_RATE_0, 921600);
	init_param_uint(PARAM_BAUD_RATE_1, 0);
	init_param_uint(PARAM_WAIT_FOR_HEARTBEAT, 0);
	init_param_fix16(PARAM_TIMESYNC_ALPHA, fix16_from_float(0.8f));
	init_param_uint(PARAM_SYSTEM_ID, 1);
	init_param_uint(PARAM_COMPONENT_ID, 1);
	init_param_uint(PARAM_GCS_SYSTEM_ID, 1);
	init_param_uint(PARAM_GCS_COMPONENT_ID, 240);
	init_param_uint(PARAM_RELAXED_PARAM_SET, 1);
	init_param_fix16(PARAM_STREAM_RATE_HEARTBEAT_0, fix16_from_float(1.0f));
	init_param_fix16(PARAM_STREAM_RATE_SYS_STATUS_0, fix16_from_float(0.2f));
	init_param_fix16(PARAM_STREAM_RATE_HIGHRES_IMU_0, fix16_from_float(100.0f));
	init_param_fix16(PARAM_STREAM_RATE_ATTITUDE_0, fix16_from_float(0.0f));
	init_param_fix16(PARAM_STREAM_RATE_ATTITUDE_QUATERNION_0, fix16_from_float(50.0f));
	init_param_fix16(PARAM_STREAM_RATE_ATTITUDE_TARGET_0, fix16_from_float(50.0f));
	init_param_fix16(PARAM_STREAM_RATE_SERVO_OUTPUT_RAW_0, fix16_from_float(10.0f));
	init_param_fix16(PARAM_STREAM_RATE_TIMESYNC_0, fix16_from_float(10.0f));
	init_param_fix16(PARAM_STREAM_RATE_LOW_PRIORITY_0, fix16_from_float(100.0f));
	init_param_uint(PARAM_SENSOR_IMU_CBRK, 1);
	init_param_uint(PARAM_SENSOR_MAG_CBRK, 0);
	init_param_uint(PARAM_SENSOR_BARO_CBRK, 0);
	init_param_uint(PARAM_SENSOR_SONAR_CBRK, 0);
	init_param_uint(PARAM_SENSOR_EXT_POSE_CBRK, 1);
	init_param_uint(PARAM_SENSOR_SAFETY_CBRK, 0);
	init_param_fix16(PARAM_SENSOR_BARO_UPDATE_RATE, fix16_from_float(0.0f));
	init_param_fix16(PARAM_SENSOR_SONAR_UPDATE_RATE, fix16_from_float(0.0f));
	init_param_fix16(PARAM_SENSOR_MAG_UPDATE_RATE, fix16_from_float(0.0f));
	init_param_uint(PARAM_SENSOR_IMU_STRM_COUNT, 1000);
	init_param_uint(PARAM_SENSOR_BARO_STRM_COUNT, 50);
	init_param_uint(PARAM_SENSOR_SONAR_STRM_COUNT, 50);
	init_param_uint(PARAM_SENSOR_EXT_POSE_STRM_COUNT, 10);
	init_param_uint(PARAM_SENSOR_MAG_STRM_COUNT, 50);
	init_param_uint(PARAM_SENSOR_OFFB_HRBT_STRM_COUNT, 2);
	init_param_uint(PARAM_SENSOR_OFFB_CTRL_STRM_COUNT, 100);
	init_param_uint(PARAM_SENSOR_PWM_CTRL_STRM_COUNT, 100);
	init_param_uint(PARAM_SENSOR_IMU_TIMEOUT, 2000);
	init_param_uint(PARAM_SENSOR_BARO_TIMEOUT, 20000);
	init_param_uint(PARAM_SENSOR_SONAR_TIMEOUT, 20000);
	init_param_uint(PARAM_SENSOR_EXT_POSE_TIMEOUT, 500000);
	init_param_uint(PARAM_SENSOR_MAG_TIMEOUT, 20000);
	init_param_uint(PARAM_SENSOR_OFFB_HRBT_TIMEOUT, 5000000);
	init_param_uint(PARAM_SENSOR_OFFB_CTRL_TIMEOUT, 200000);
	init_param_uint(PARAM_SENSOR_PWM_CTRL_TIMEOUT, 200000);
	init_param_uint(PARAM_CAL_IMU_PASSES, 1000);
	init_param_uint(PARAM_INIT_TIME, 3000);
	init_param_uint(PARAM_EST_USE_ACC_COR, 1);
	init_param_uint(PARAM_EST_USE_MAT_EXP, 1);
	init_param_uint(PARAM_EST_USE_QUAD_INT, 1);
	init_param_uint(PARAM_EST_USE_ADPT_BIAS, 0);
	init_param_fix16(PARAM_FILTER_KP, fix16_from_float(1.0f));
	init_param_fix16(PARAM_FILTER_KI, fix16_from_float(0.05f));
	init_param_fix16(PARAM_GYRO_ALPHA, fix16_from_float(0.6f));
	init_param_fix16(PARAM_ACC_ALPHA, fix16_from_float(0.6f));
	init_param_fix16(PARAM_FUSE_EXT_HDG_W, fix16_from_float(0.2f));
	init_param_int(PARAM_GYRO_X_BIAS, 0);
	init_param_int(PARAM_GYRO_Y_BIAS, 0);
	init_param_int(PARAM_GYRO_Z_BIAS, 0);
	init_param_int(PARAM_ACC_X_BIAS, 0);
	init_param_int(PARAM_ACC_Y_BIAS, 0);
	init_param_int(PARAM_ACC_Z_BIAS, 0);
	init_param_fix16(PARAM_ACC_X_SCALE_POS, fix16_from_float(1.0f));
	init_param_fix16(PARAM_ACC_X_SCALE_NEG, fix16_from_float(1.0f));
	init_param_fix16(PARAM_ACC_Y_SCALE_POS, fix16_from_float(1.0f));
	init_param_fix16(PARAM_ACC_Y_SCALE_NEG, fix16_from_float(1.0f));
	init_param_fix16(PARAM_ACC_Z_SCALE_POS, fix16_from_float(1.0f));
	init_param_fix16(PARAM_ACC_Z_SCALE_NEG, fix16_from_float(1.0f));
	init_param_fix16(PARAM_ACC_X_TEMP_COMP, fix16_from_float(0.0f));
	init_param_fix16(PARAM_ACC_Y_TEMP_COMP, fix16_from_float(0.0f));
	init_param_fix16(PARAM_ACC_Z_TEMP_COMP, fix16_from_float(0.0f));
	init_param_fix16(PARAM_PID_ROLL_RATE_P, fix16_from_float(0.05f));
	init_param_fix16(PARAM_PID_ROLL_RATE_I, fix16_from_float(0.0f));
	init_param_fix16(PARAM_PID_ROLL_RATE_D, fix16_from_float(0.005f));
	init_param_fix16(PARAM_MAX_ROLL_RATE, fix16_from_float(3.14159f));
	init_param_fix16(PARAM_PID_PITCH_RATE_P, fix16_from_float(0.05f));
	init_param_fix16(PARAM_PID_PITCH_RATE_I, fix16_from_float(0.0f));
	init_param_fix16(PARAM_PID_PITCH_RATE_D, fix16_from_float(0.005f));
	init_param_fix16(PARAM_MAX_PITCH_RATE, fix16_from_float(3.14159f));
	init_param_fix16(PARAM_PID_YAW_RATE_P, fix16_from_float(0.2f));
	init_param_fix16(PARAM_PID_YAW_RATE_I, fix16_from_float(0.1f));
	init_param_fix16(PARAM_PID_YAW_RATE_D, fix16_from_float(0.0f));
	init_param_fix16(PARAM_MAX_YAW_RATE, fix16_from_float(1.57079f));
	init_param_fix16(PARAM_PID_ROLL_ANGLE_P, fix16_from_float(6.5f));
	init_param_fix16(PARAM_MAX_ROLL_ANGLE, fix16_from_float(0.786f));
	init_param_fix16(PARAM_PID_PITCH_ANGLE_P, fix16_from_float(6.5f));
	init_param_fix16(PARAM_MAX_PITCH_ANGLE, fix16_from_float(0.786f));
	init_param_fix16(PARAM_PID_YAW_ANGLE_P, fix16_from_float(6.5f));
	init_param_fix16(PARAM_PID_TAU, fix16_from_float(0.05f));
	init_param_uint(PARAM_MOTOR_PWM_SEND_RATE, 400);
	init_param_uint(PARAM_MOTOR_PWM_IDLE, 1150);
	init_param_uint(PARAM_MOTOR_PWM_MIN, 1000);
	init_param_uint(PARAM_MOTOR_PWM_MAX, 2000);
	init_param_uint(PARAM_DO_ESC_CAL, 0);
	init_param_fix16(PARAM_FAILSAFE_THROTTLE, fix16_from_float(0.25f));
	init_param_uint(PARAM_THROTTLE_TIMEOUT, 10000000);
	init_param_uint(PARAM_MIXER, 0);
	init_param_uint(PARAM_MAV_TYPE, 0);
	init_param_uint(PARAM_RESET_PARAMS, 0);
}

const char _param_names[PARAMS_COUNT][MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN] = {
	"BOARD_REV",
	"FW_VERSION",
	"SW_VERSION",
	"BAUD_RATE_0",
	"BAUD_RATE_1",
	"COMMS_WAIT",
	"TIMESYNC_ALPHA",
	"MAV_SYS_ID",
	"MAV_COMP_ID",
	"GCS_SYS_ID",
	"GCS_COMP_ID",
	"RELAXED_SET",
	"STRM0_HRTBT",
	"STRM0_SYS_STAT",
	"STRM0_HR_IMU",
	"STRM0_ATT",
	"STRM0_ATT_Q",
	"STRM0_ATT_T",
	"STRM0_SRV_OUT",
	"STRM0_TIMESYNC",
	"STRM0_LPQ",
	"CBRK_IMU",
	"CBRK_MAG",
	"CBRK_BARO",
	"CBRK_SONAR",
	"CBRK_EXT_POSE",
	"CBRK_SAFETY",
	"CHK_RATE_BARO",
	"CHK_RATE_SONAR",
	"CHK_RATE_MAG",
	"STRM_NUM_IMU",
	"STRM_NUM_BARO",
	"STRM_NUM_SONAR",
	"STRM_NUM_EXT_P",
	"STRM_NUM_MAG",
	"STRM_NUM_OB_H",
	"STRM_NUM_OB_C",
	"STRM_NUM_RC_C",
	"TIMEOUT_IMU",
	"TIMEOUT_BARO",
	"TIMEOUT_SONAR",
	"TIMEOUT_EXT_P",
	"TIMEOUT_MAG",
	"TIMEOUT_OB_HRBT",
	"TIMEOUT_OB_CTRL",
	"TIMEOUT_RC_CTRL",
	"CAL_IMU_PASSES",
	"FILTER_INIT_T",
	"EST_ACC_COR",
	"EST_MAT_EXP",
	"EST_QUAD_INT",
	"EST_ADPT_BIAS",
	"FILTER_KP",
	"FILTER_KI",
	"EST_LPF_GYRO_A",
	"EST_LPF_ACC_A",
	"FSE_EXT_HDG_W",
	"GYRO_X_BIAS",
	"GYRO_Y_BIAS",
	"GYRO_Z_BIAS",
	"ACC_X_BIAS",
	"ACC_Y_BIAS",
	"ACC_Z_BIAS",
	"ACC_X_S_POS",
	"ACC_X_S_NEG",
	"ACC_Y_S_POS",
	"ACC_Y_S_NEG",
	"ACC_Z_S_POS",
	"ACC_Z_S_NEG",
	"ACC_X_TEMP_COMP",
	"ACC_Y_TEMP_COMP",
	"ACC_Z_TEMP_COMP",
	"PID_ROLL_R_P",
	"PID_ROLL_R_I",
	"PID_ROLL_R_D",
	"MAX_ROLL_R",
	"PID_PITCH_R_P",
	"PID_PITCH_R_I",
	"PID_PITCH_R_D",
	"MAX_PITCH_R",
	"PID_YAW_R_P",
	"PID_YAW_R_I",
	"PID_YAW_R_D",
	"MAX_YAW_R",
	"PID_ROLL_ANG_P",
	"MAX_ROLL_A",
	"PID_PITCH_ANG_P",
	"MAX_PITCH_A",
	"PID_YAW_ANG_P",
	"PID_TAU",
	"MOTOR_PWM_RATE",
	"MOTOR_PWM_IDLE",
	"MOTOR_PWM_MIN",
	"MOTOR_PWM_MAX",
	"DO_ESC_CAL",
	"FAILSAFE_THRTL",
	"TIMEOUT_THRTL",
	"SYS_AUTOSTART",
	"MAV_TYPE",
	"SYS_AUTOCONFIG",
};

void param_change_callback(param_id_t id) {
	switch(id) {
		case PARAM_STREAM_RATE_HEARTBEAT_0:
			communication_calc_period_update(COMM_CH_0, MAVLINK_STREAM_ID_HEARTBEAT);
			break;
		case PARAM_STREAM_RATE_SYS_STATUS_0:
			communication_calc_period_update(COMM_CH_0, MAVLINK_STREAM_ID_SYS_STATUS);
			break;
		case PARAM_STREAM_RATE_HIGHRES_IMU_0:
			communication_calc_period_update(COMM_CH_0, MAVLINK_STREAM_ID_HIGHRES_IMU);
			break;
		case PARAM_STREAM_RATE_ATTITUDE_0:
			communication_calc_period_update(COMM_CH_0, MAVLINK_STREAM_ID_ATTITUDE);
			break;
		case PARAM_STREAM_RATE_ATTITUDE_QUATERNION_0:
			communication_calc_period_update(COMM_CH_0, MAVLINK_STREAM_ID_ATTITUDE_QUATERNION);
			break;
		case PARAM_STREAM_RATE_ATTITUDE_TARGET_0:
			communication_calc_period_update(COMM_CH_0, MAVLINK_STREAM_ID_ATTITUDE_TARGET);
			break;
		case PARAM_STREAM_RATE_SERVO_OUTPUT_RAW_0:
			communication_calc_period_update(COMM_CH_0, MAVLINK_STREAM_ID_SERVO_OUTPUT_RAW);
			break;
		case PARAM_STREAM_RATE_TIMESYNC_0:
			communication_calc_period_update(COMM_CH_0, MAVLINK_STREAM_ID_TIMESYNC);
			break;
		case PARAM_STREAM_RATE_LOW_PRIORITY_0:
			communication_calc_period_update(COMM_CH_0, MAVLINK_STREAM_ID_LOW_PRIORITY);
			break;
		case PARAM_PID_ROLL_RATE_P:
			pid_set_gain_p(&_pid_roll_rate, get_param_fix16(PARAM_PID_ROLL_RATE_P));
			break;
		case PARAM_PID_ROLL_RATE_I:
			pid_set_gain_i(&_pid_roll_rate, get_param_fix16(PARAM_PID_ROLL_RATE_I));
			break;
		case PARAM_PID_ROLL_RATE_D:
			pid_set_gain_d(&_pid_roll_rate, get_param_fix16(PARAM_PID_ROLL_RATE_D));
			break;
		case PARAM_MAX_ROLL_RATE:
			pid_set_min_max(&_pid_roll_rate, -get_param_fix16(PARAM_MAX_ROLL_RATE), get_param_fix16(PARAM_MAX_ROLL_RATE));
			break;
		case PARAM_PID_PITCH_RATE_P:
			pid_set_gain_p(&_pid_pitch_rate, get_param_fix16(PARAM_PID_PITCH_RATE_P));
			break;
		case PARAM_PID_PITCH_RATE_I:
			pid_set_gain_i(&_pid_pitch_rate, get_param_fix16(PARAM_PID_PITCH_RATE_I));
			break;
		case PARAM_PID_PITCH_RATE_D:
			pid_set_gain_d(&_pid_pitch_rate, get_param_fix16(PARAM_PID_PITCH_RATE_D));
			break;
		case PARAM_MAX_PITCH_RATE:
			pid_set_min_max(&_pid_pitch_rate, -get_param_fix16(PARAM_MAX_PITCH_RATE), get_param_fix16(PARAM_MAX_PITCH_RATE));
			break;
		case PARAM_PID_YAW_RATE_P:
			pid_set_gain_p(&_pid_yaw_rate, get_param_fix16(PARAM_PID_YAW_RATE_P));
			break;
		case PARAM_PID_YAW_RATE_I:
			pid_set_gain_i(&_pid_yaw_rate, get_param_fix16(PARAM_PID_YAW_RATE_I));
			break;
		case PARAM_PID_YAW_RATE_D:
			pid_set_gain_d(&_pid_yaw_rate, get_param_fix16(PARAM_PID_YAW_RATE_D));
			break;
		case PARAM_MAX_YAW_RATE:
			pid_set_min_max(&_pid_yaw_rate, -get_param_fix16(PARAM_MAX_YAW_RATE), get_param_fix16(PARAM_MAX_YAW_RATE));
			break;
		case PARAM_PID_TAU:
			pid_set_gain_tau(&_pid_roll_rate, get_param_fix16(PARAM_PID_TAU));
			pid_set_gain_tau(&_pid_pitch_rate, get_param_fix16(PARAM_PID_TAU));
			pid_set_gain_tau(&_pid_yaw_rate, get_param_fix16(PARAM_PID_TAU));
			break;
		default:
			//No action needed for this param ID
			break;
	}

	mavlink_message_t msg_out;
	mavlink_prepare_param_value( &msg_out, id );
	lpq_queue_broadcast_msg( &msg_out );
}
