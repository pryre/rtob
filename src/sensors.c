#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "sensors.h"
#include "params.h"
#include "mavlink_system.h"
#include "drivers/mpu.h"
#include "breezystm32.h"

#include "fix16.h"
#include "fixvector3d.h"

//Number of itterations of averaging to use with IMU calibrations
#define SENSOR_CAL_IMU_PASSES 1000

//==-- Local Variables

static const fix16_t GRAVITY_CONST = 0x0009CE80; //Is equal to 9.80665 (Positive!) in Q16.16
//static int32_t last_check_imu = 0;
/*static float accel_scale; // converts to units of m/s^2

static int16_t accel_data[3];
static int16_t gyro_data[3];
static int16_t temp_data;
*/
static uint8_t accel_status = 0;
static uint8_t gyro_status = 0;
static uint8_t temp_status = 0;
sensor_readings_t _sensors;
uint8_t _sensor_calibration;
sensor_calibration_data_t _sensor_cal_data;
volatile bool imu_interrupt;


//==-- Functions
static void time_init(void) {
	_sensors.time.present = true;
	_sensors.time.dt = 0;
	_sensors.time.counter = 0;
	_sensors.time.start = 0;
	_sensors.time.end = 0;
	_sensors.time.max = 0;
	_sensors.time.min = 1000;
}
/*
static void imu_init(void) {
	_sensors.imu.present = true;
	_sensors.imu.temperature = 0;
	uint32_t time;		//Time measured
	float accel_scale;	//Scale to correct raw accel data
	float gyro_scale;	//Scale to correct raw gyro data
}

static void baro_init(void) {
	_sensor_baro.present = false;
}

static void sonar_init(void) {
	_sensor_sonar.present = false;
}*/

static void sensors_imu_poll(void) {
		imu_interrupt = true;
		mpu6050_request_async_accel_read(_sensors.imu.accel_raw, &accel_status);
		mpu6050_request_async_gyro_read(_sensors.imu.gyro_raw, &gyro_status);
		mpu6050_request_async_temp_read(&(_sensors.imu.temp_raw), &temp_status);
}

void sensors_init(void) {
	//==-- IMU-MPU6050
	//TODO: Set IMU to be calibrated if not already
    mpu6050_register_interrupt_cb(&sensors_imu_poll, get_param_int(PARAM_BAUD_RATE));
	_sensor_cal_data.accel.acc1G = mpu6050_init(INV_FSR_8G, INV_FSR_2000DPS);	//Get the 1g gravity scale (raw->g's)
	//TODO: Note that this is just an estimate (?) in the docs
//	acc1G = 4096;
	_sensors.imu.accel_scale = fix16_sdiv(GRAVITY_CONST, fix16_from_int(_sensor_cal_data.accel.acc1G));	//Get the m/s scale (raw->g's->m/s/s)
	_sensors.imu.gyro_scale = fix16_from_float(MPU_GYRO_SCALE);	//Get radians scale (raw->rad/s)

	_sensor_cal_data.accel.temp_scale = fix16_from_float(340.0f);
	_sensor_cal_data.accel.temp_shift = fix16_from_float(36.53f);

	_sensor_cal_data.gyro.count = 0;
	_sensor_cal_data.gyro.sum_x = 0;
	_sensor_cal_data.gyro.sum_y = 0;
	_sensor_cal_data.gyro.sum_z = 0;

	_sensor_cal_data.accel.count = 0;
	_sensor_cal_data.accel.sum_x = 0;
	_sensor_cal_data.accel.sum_y = 0;
	_sensor_cal_data.accel.sum_z = 0;
	_sensor_cal_data.accel.sum_t = 0;

	//==-- Timer
	time_init();
}

bool sensors_read(void) {
	bool imu_job_complete = false;

	//Check IMU status
	if(accel_status == I2C_JOB_COMPLETE && gyro_status == I2C_JOB_COMPLETE && temp_status == I2C_JOB_COMPLETE) {
		imu_interrupt = false;	//TODO: There might be a better place to have this
		imu_job_complete = true;
		accel_status = I2C_JOB_DEFAULT;
		gyro_status = I2C_JOB_DEFAULT;
		temp_status = I2C_JOB_DEFAULT;

		//TODO: Maybe the imu data should be aggregated so it can be called at a lower Hz?
	}

	//TODO: Check other status

	//TODO: May need to offset these so they don't all check at once(?)

	//Return the results
	return imu_job_complete;
}

bool sensors_update(uint32_t time_us) {
	//bool update_success = false;
	//TODO: Remember not to expect all sensors to be ready

	//==-- Update IMU

    // convert temperature SI units (degC, m/s^2, rad/s)
    // convert to NED (first of braces)
	//Correct for biases and temperature (second braces)
	//TODO: Calibration has to be done without biases and temp factored in

	//TODO: Perhaps X is being calculated in revearse?
	//TODO:Something about the frame of reference (NED/ENU) isn't quite right
	_sensors.imu.time = time_us;

	//==-- Temperature in degC
	//value = (_sensors.imu.temp_raw/temp_scale) + temp_shift
	_sensors.imu.temperature = fix16_sadd(fix16_sdiv(fix16_from_int(_sensors.imu.temp_raw), _sensor_cal_data.accel.temp_scale), _sensor_cal_data.accel.temp_shift);

	//==-- Accel in NED
	//TODO: value = (raw - BIAS - (EMP_COMP * TEMP)) * scale
	// value = (raw - BIAS) * scale
	_sensors.imu.accel.x = fix16_smul(fix16_from_int(-(_sensors.imu.accel_raw[0] - get_param_int(PARAM_ACC_X_BIAS))), _sensors.imu.accel_scale);
	_sensors.imu.accel.y = fix16_smul(fix16_from_int(_sensors.imu.accel_raw[1] - get_param_int(PARAM_ACC_Y_BIAS)), _sensors.imu.accel_scale);
	_sensors.imu.accel.z = fix16_smul(fix16_from_int(_sensors.imu.accel_raw[2] - get_param_int(PARAM_ACC_Z_BIAS)), _sensors.imu.accel_scale);

	//==-- Gyro in NED
	// value = (raw - BIAS) * scale
	_sensors.imu.gyro.x = fix16_smul(fix16_from_int(_sensors.imu.gyro_raw[0] - get_param_int(PARAM_GYRO_X_BIAS)), _sensors.imu.gyro_scale);
	_sensors.imu.gyro.y = fix16_smul(fix16_from_int(-(_sensors.imu.gyro_raw[1] - get_param_int(PARAM_GYRO_Y_BIAS))), _sensors.imu.gyro_scale);
	_sensors.imu.gyro.z = fix16_smul(fix16_from_int(-(_sensors.imu.gyro_raw[2] - get_param_int(PARAM_GYRO_Z_BIAS))), _sensors.imu.gyro_scale);

	//TODO: This should be aware of failures
	return true;
}

//TODO: This calibration method is very basic, doesn't take into acount very much...mabye?
void sensors_calibrate(void) {
	if(_sensor_calibration & SENSOR_CAL_GYRO) {
		_sensor_cal_data.gyro.sum_x += _sensors.imu.gyro_raw[0];
		_sensor_cal_data.gyro.sum_y += _sensors.imu.gyro_raw[1];
		_sensor_cal_data.gyro.sum_z += _sensors.imu.gyro_raw[2];

	_sensor_cal_data.gyro.count++;

		if (_sensor_cal_data.gyro.count >= SENSOR_CAL_IMU_PASSES) {
			set_param_int(PARAM_GYRO_X_BIAS, (_sensor_cal_data.gyro.sum_x / _sensor_cal_data.gyro.count));
			set_param_int(PARAM_GYRO_Y_BIAS, (_sensor_cal_data.gyro.sum_y / _sensor_cal_data.gyro.count));
			set_param_int(PARAM_GYRO_Z_BIAS, (_sensor_cal_data.gyro.sum_z / _sensor_cal_data.gyro.count));

			_sensor_cal_data.gyro.count = 0;
			_sensor_cal_data.gyro.sum_x = 0;
			_sensor_cal_data.gyro.sum_y = 0;
			_sensor_cal_data.gyro.sum_z = 0;

			_sensor_calibration ^= SENSOR_CAL_GYRO;	//Turn off SENSOR_CAL_GYRO bit
			//TODO: "we could do some sanity checking here if we wanted to."
		}
	}

	if(_sensor_calibration & SENSOR_CAL_ACCEL) {
		//Compensate for the gravity in Z axis, that way bias can be relative to 0
		_sensor_cal_data.accel.sum_x += _sensors.imu.accel_raw[0];
		_sensor_cal_data.accel.sum_y += _sensors.imu.accel_raw[1];
		_sensor_cal_data.accel.sum_z += _sensors.imu.accel_raw[2] - _sensor_cal_data.accel.acc1G;
		_sensor_cal_data.accel.sum_t += _sensors.imu.temp_raw;

		_sensor_cal_data.accel.count++;

		if (_sensor_cal_data.accel.count >= SENSOR_CAL_IMU_PASSES) {
			//==-- bias = sum / count
			//==-- //TODO: bias = (sum - (temp_comp*temp_sum)) / count

			set_param_int(PARAM_ACC_X_BIAS, (_sensor_cal_data.accel.sum_x / _sensor_cal_data.accel.count));
			set_param_int(PARAM_ACC_Y_BIAS, (_sensor_cal_data.accel.sum_y / _sensor_cal_data.accel.count));
			set_param_int(PARAM_ACC_Z_BIAS, (_sensor_cal_data.accel.sum_z / _sensor_cal_data.accel.count));

			_sensor_cal_data.accel.count = 0;
			_sensor_cal_data.accel.sum_x = 0;
			_sensor_cal_data.accel.sum_y = 0;
			_sensor_cal_data.accel.sum_z = 0;
			_sensor_cal_data.accel.sum_t = 0;

			_sensor_calibration ^= SENSOR_CAL_ACCEL;	//Turn off SENSOR_CAL_ACCEL bit
			//TODO: "we could do some sanity checking here if we wanted to."
		}
	}

	//If there are no longer any sensors to calibrate
	if(_sensor_calibration == SENSOR_CAL_NONE) {
		//Inform the GCS that it has been completed
		if(check_lpq_space_free()) {
			uint8_t i = get_lpq_next_slot();
			_low_priority_queue.buffer_len[i] = mavlink_prepare_command_ack(_low_priority_queue.buffer[i], MAV_CMD_PREFLIGHT_CALIBRATION, MAV_RESULT_ACCEPTED);
			_low_priority_queue.queued_message_count++;
		}
	}
}

