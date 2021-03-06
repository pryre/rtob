/*
   hmc5883l.c :  Support for HMC5883L Magnetometer

   Adapted from
   https://github.com/multiwii/baseflight/blob/master/src/drv_hmc5883l.c

   This file is part of BreezySTM32.

   BreezySTM32 is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   BreezySTM32 is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with BreezySTM32.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <breezystm32.h>
#include <drivers/breezy_naze32_common/drv_hmc5883l.h>

#include <math.h>

// HMC5883L, default address 0x1E
// PB12 connected to MAG_DRDY on rev4 hardware
// PC14 connected to MAG_DRDY on rev5 hardware

/* CTRL_REGA: Control Register A
 * Read Write
 * Default value: 0x10
 * 7:5  0   These bits must be cleared for correct operation.
 * 4:2 DO2-DO0: Data Output Rate Bits
 *             DO2 |  DO1 |  DO0 |   Minimum Data Output Rate (Hz)
 *            ------------------------------------------------------
 *              0  |  0   |  0   |            0.75
 *              0  |  0   |  1   |            1.5
 *              0  |  1   |  0   |            3
 *              0  |  1   |  1   |            7.5
 *              1  |  0   |  0   |           15 (default)
 *              1  |  0   |  1   |           30
 *              1  |  1   |  0   |           75
 *              1  |  1   |  1   |           Not Used
 * 1:0 MS1-MS0: Measurement Configuration Bits
 *             MS1 | MS0 |   MODE
 *            ------------------------------
 *              0  |  0   |  Normal
 *              0  |  1   |  Positive Bias
 *              1  |  0   |  Negative Bias
 *              1  |  1   |  Not Used
 *
 * CTRL_REGB: Control RegisterB
 * Read Write
 * Default value: 0x20
 * 7:5 GN2-GN0: Gain Configuration Bits.
 *             GN2 |  GN1 |  GN0 |   Mag Input   | Gain       | Output Range
 *                 |      |      |  Range[Ga]    | [LSB/mGa]  |
 *            ------------------------------------------------------
 *              0  |  0   |  0   |  ±0.88Ga      |   1370     | 0xF800?0x07FF
 * (-2048:2047)
 *              0  |  0   |  1   |  ±1.3Ga (def) |   1090     | 0xF800?0x07FF
 * (-2048:2047)
 *              0  |  1   |  0   |  ±1.9Ga       |   820      | 0xF800?0x07FF
 * (-2048:2047)
 *              0  |  1   |  1   |  ±2.5Ga       |   660      | 0xF800?0x07FF
 * (-2048:2047)
 *              1  |  0   |  0   |  ±4.0Ga       |   440      | 0xF800?0x07FF
 * (-2048:2047)
 *              1  |  0   |  1   |  ±4.7Ga       |   390      | 0xF800?0x07FF
 * (-2048:2047)
 *              1  |  1   |  0   |  ±5.6Ga       |   330      | 0xF800?0x07FF
 * (-2048:2047)
 *              1  |  1   |  1   |  ±8.1Ga       |   230      | 0xF800?0x07FF
 * (-2048:2047)
 *                               |Not recommended|
 *
 * 4:0 CRB4-CRB: 0 This bit must be cleared for correct operation.
 *
 * _MODE_REG: Mode Register
 * Read Write
 * Default value: 0x02
 * 7:2  0   These bits must be cleared for correct operation.
 * 1:0 MD1-MD0: Mode Select Bits
 *             MS1 | MS0 |   MODE
 *            ------------------------------
 *              0  |  0   |  Continuous-Conversion Mode.
 *              0  |  1   |  Single-Conversion Mode
 *              1  |  0   |  Negative Bias
 *              1  |  1   |  Sleep Mode
 */

#define MAG_ADDRESS 0x1E
#define MAG_DATA_REGISTER 0x03

#define HMC58X3_R_CONFA 0
#define HMC58X3_R_CONFB 1
#define HMC58X3_R_MODE 2
#define HMC58X3_X_SELF_TEST_GAUSS \
	( +1.16f ) // X axis level when bias current is applied.
#define HMC58X3_Y_SELF_TEST_GAUSS \
	( +1.16f ) // Y axis level when bias current is applied.
#define HMC58X3_Z_SELF_TEST_GAUSS \
	( +1.08f )									 // Z axis level when bias current is applied.
#define SELF_TEST_LOW_LIMIT ( 243.0f / 390.0f )  // Low limit when gain is 5.
#define SELF_TEST_HIGH_LIMIT ( 575.0f / 390.0f ) // High limit when gain is 5.
#define HMC_POS_BIAS 1
#define HMC_NEG_BIAS 2

static float magGain[3] = {1.0f, 1.0f, 1.0f};

/*bool hmc5883lDetect()
{
    bool ack = false;
    uint8_t sig = 0;

    hmc5883lInit();
    delay(100);

    ack = i2cReadBuffer(MAG_ADDRESS, 0x0A, 1, &sig);
    if (!ack || sig != 'H')
        return false;

    return true;
}*/

bool hmc5883lInit( int boardVersion ) {
	gpio_config_t gpio;
	gpio.speed = Speed_2MHz;
	gpio.mode = Mode_IN_FLOATING;
	if ( boardVersion == 4 ) {
		// PB12 - MAG_DRDY output on rev4 hardware
		gpio.pin = Pin_12;
		gpioInit( GPIOB, &gpio );
	} else {
		// PC14 - MAG_DRDY output on rev5 hardware
		gpio.pin = Pin_14;
		gpioInit( GPIOC, &gpio );
	}

	//====================================
	// XXX: TODO: CALIBRATION
	int16_t magADC[3];
	int i;
	int32_t xyz_total[3] = {0, 0, 0}; // 32 bit totals so they won't overflow.
	bool bret = true;				  // Error indicator

	delay( 50 );
	i2cWriteRegister(
		MAG_ADDRESS, HMC58X3_R_CONFA,
		0x010 + HMC_POS_BIAS ); // Reg A DOR = 0x010 + MS1, MS0 set to pos bias
	// Note that the  very first measurement after a gain change maintains the
	// same gain as the previous setting.
	// The new gain setting is effective from the second measurement and on.
	i2cWriteRegister( MAG_ADDRESS, HMC58X3_R_CONFB,
					  0x60 ); // Set the Gain to 2.5Ga (7:5->011)
	delay( 100 );
	hmc5883lRead( magADC );

	for ( i = 0; i < 10; i++ ) { // Collect 10 samples
		i2cWriteRegister( MAG_ADDRESS, HMC58X3_R_MODE, 1 );
		delay( 50 );
		hmc5883lRead( magADC ); // Get the raw values in case the scales have already
		// been changed.

		// Since the measurements are noisy, they should be averaged rather than
		// taking the max.
		xyz_total[0] += magADC[0];
		xyz_total[1] += magADC[1];
		xyz_total[2] += magADC[2];

		// Detect saturation.
		if ( -4096 >= magADC[0] || -4096 >= magADC[1] || -4096 >= magADC[2] ) {
			bret = false;
			break; // Breaks out of the for loop.  No sense in continuing if we
				   // saturated.
		}
	}

	// Apply the negative bias. (Same gain)
	i2cWriteRegister(
		MAG_ADDRESS, HMC58X3_R_CONFA,
		0x010 + HMC_NEG_BIAS ); // Reg A DOR = 0x010 + MS1, MS0 set to negative bias.
	for ( i = 0; i < 10; i++ ) {
		i2cWriteRegister( MAG_ADDRESS, HMC58X3_R_MODE, 1 );
		delay( 50 );
		hmc5883lRead( magADC ); // Get the raw values in case the scales have already
		// been changed.

		// Since the measurements are noisy, they should be averaged.
		xyz_total[0] -= magADC[0];
		xyz_total[1] -= magADC[1];
		xyz_total[2] -= magADC[2];

		// Detect saturation.
		if ( -4096 >= magADC[0] || -4096 >= magADC[1] || -4096 >= magADC[2] ) {
			bret = false;
			break; // Breaks out of the for loop.  No sense in continuing if we
				   // saturated.
		}
	}

	magGain[0] = fabsf( 660.0f * HMC58X3_X_SELF_TEST_GAUSS * 2.0f * 10.0f / xyz_total[0] );
	magGain[1] = fabsf( 660.0f * HMC58X3_Y_SELF_TEST_GAUSS * 2.0f * 10.0f / xyz_total[1] );
	magGain[2] = fabsf( 660.0f * HMC58X3_Z_SELF_TEST_GAUSS * 2.0f * 10.0f / xyz_total[2] );

	// leave test mode
	i2cWriteRegister( MAG_ADDRESS, HMC58X3_R_CONFA,
					  0x70 ); // Configuration Register A  -- 0 11 100 00  num
	// samples: 8 ; output rate: 15Hz ; normal measurement
	// mode
	i2cWriteRegister( MAG_ADDRESS, HMC58X3_R_CONFB,
					  0x20 ); // Configuration Register B  -- 001 00000
	// configuration gain 1.3Ga
	i2cWriteRegister( MAG_ADDRESS, HMC58X3_R_MODE, 0x00 ); // Mode register
	// -- 000000 00
	// continuous Conversion
	// Mode
	delay( 100 );

	if ( !bret ) { // Something went wrong so get a best guess
		magGain[0] = 1.0f;
		magGain[1] = 1.0f;
		magGain[2] = 1.0f;
	}
	//====================================

	bool ack = false;
	uint8_t sig = 0;

	ack = i2cReadBuffer( MAG_ADDRESS, 0x0A, 1, &sig ); // Check the identification
	// register to make sure it
	// is the correct device
	if ( !ack || sig != 'H' )
		return false;

	return true;
}

void hmc5883lRead( int16_t* magData ) {
	uint8_t buf[6];

	i2cReadBuffer( MAG_ADDRESS, MAG_DATA_REGISTER, 6, buf );
	// During calibration, magGain is 1.0, so the read returns normal
	// non-calibrated values.
	// After calibration is done, magGain is set to calculated gain values.
	magData[0] = ( int16_t )( buf[0] << 8 | buf[1] ) * magGain[0];
	magData[2] = ( int16_t )( buf[2] << 8 | buf[3] ) * magGain[1];
	magData[1] = ( int16_t )( buf[4] << 8 | buf[5] ) * magGain[2];
}

/* =================================================================
 * Asynchronous Method
 */
static uint8_t mag_buffer[6];
// static int16_t mag_data[3];
static int16_t* mag_data;

void mag_read_CB( void ) {
	// XXX: Packet is structured XZY
	mag_data[0] = ( mag_buffer[0] << 8 | mag_buffer[1] ); // X Data
	mag_data[2] = ( mag_buffer[2] << 8 | mag_buffer[3] ); // Z Data
	mag_data[1] = ( mag_buffer[4] << 8 | mag_buffer[5] ); // Y Data
}

// XXX: Maximum 160 Hz update rate (datasheet)
void hmc5883l_request_async_read( int16_t* magData, volatile uint8_t* status ) {
	mag_data = magData;

	i2c_queue_job( READ, MAG_ADDRESS, MAG_DATA_REGISTER, mag_buffer, 6, status,
				   &mag_read_CB );
}

/*
void hmc5883l_read_magnetometer(int16_t *magData)
{
    magData[0] = mag_data[X];
    magData[1] = mag_data[Y];
    magData[2] = mag_data[Z];
    return;
}
*/
