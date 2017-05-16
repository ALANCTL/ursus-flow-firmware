#include <stdint.h>

#include "stm32f7xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"
#include "spi.h"

#include "mpu9250.h"

#include "delay.h"
#include "imu.h"

__inline__ static void mpu9250_select(void);
__inline__ static void mpu9250_deselect(void);

/* mpu9250 is low active */
static void mpu9250_select(void)
{
	gpio_off(MPU9250_CHIP_SELECTOR);
}

/* mpu9250 is low active */
static void mpu9250_deselect(void)
{
	gpio_on(MPU9250_CHIP_SELECTOR);
}

static uint8_t mpu9250_read_byte(uint8_t address)
{
	mpu9250_select();

	uint8_t buffer = '\0';
	address |= 0x80;

	spi1_write_byte(address);
	spi1_read_byte(&buffer);

	mpu9250_deselect();

	return buffer;
}

static void mpu9250_write_byte(uint8_t address, uint8_t data)
{
	mpu9250_select();

	spi1_write_byte(address);
	spi1_write_byte(data);

	mpu9250_deselect();
}

static uint8_t mpu9250_read_who_am_i(void)
{
	return mpu9250_read_byte(MPU9250_WHO_AM_I);
}

static void mpu9250_read_unscaled_gyro(vector3d_16_t *unscaled_gyro_data)
{
	mpu9250_select();

	uint8_t buffer[6] = {0};

	spi1_write_byte(MPU9250_GYRO_XOUT_H | 0x80);

#if 0
	spi1_read(buffer, 6);
#else
	spi1_read_byte(&buffer[0]);
	spi1_read_byte(&buffer[1]);
	spi1_read_byte(&buffer[2]);
	spi1_read_byte(&buffer[3]);
	spi1_read_byte(&buffer[4]);
	spi1_read_byte(&buffer[5]);
#endif

	unscaled_gyro_data->x = ((uint16_t)buffer[0] << 8) | (uint16_t)buffer[1];
	unscaled_gyro_data->y = ((uint16_t)buffer[2] << 8) | (uint16_t)buffer[3];
	unscaled_gyro_data->z = ((uint16_t)buffer[4] << 8) | (uint16_t)buffer[5];

	mpu9250_deselect();
}

static void mpu9250_convert_to_scale(vector3d_16_t *unscaled_gyro_data, vector3d_f_t *scaled_gyro_data)
{
	scaled_gyro_data->x = -unscaled_gyro_data->y * MPU9250G_1000dps - MPU9250_OFFSET_X;
	scaled_gyro_data->y = -unscaled_gyro_data->x * MPU9250G_1000dps - MPU9250_OFFSET_Y;
	scaled_gyro_data->z = -unscaled_gyro_data->z * MPU9250G_1000dps - MPU9250_OFFSET_Z;
}

void mpu9250_read(vector3d_f_t *gyro_data)
{
	vector3d_16_t unscaled_gyro_data;

	mpu9250_read_unscaled_gyro(&unscaled_gyro_data);
	mpu9250_convert_to_scale(&unscaled_gyro_data, gyro_data);
}

void mpu9250_drift_error_estimate(float *drift_x, float *drift_y, float *drift_z)
{
	vector3d_f_t gyro_data;
	*drift_x = *drift_y = *drift_z = 0;

	int n = 10000;
	int count = n;

	while(count--) {
		mpu9250_read(&gyro_data);
		*drift_x += gyro_data.x / n;
		*drift_y += gyro_data.y / n;
		*drift_z += gyro_data.z / n;
		vTaskDelay(MILLI_SECOND_TICK(1));
	}
}

int mpu9250_init(void)
{
	if(mpu9250_read_who_am_i() != 0x71) {
		vTaskDelay(MILLI_SECOND_TICK(50));
		return 1;
	}
	vTaskDelay(MILLI_SECOND_TICK(50));

	mpu9250_write_byte(MPU9250_PWR_MGMT_1, 0x80);   //reset command     = 0x80
	vTaskDelay(MILLI_SECOND_TICK(50));
	mpu9250_write_byte(MPU9250_GYRO_CONFIG, 0x10);  //full scale 1000Hz = 0x10
	vTaskDelay(MILLI_SECOND_TICK(50));
	mpu9250_write_byte(MPU9250_ACCEL_CONFIG, 0x10); //full scale 8g     = 0x10
	vTaskDelay(MILLI_SECOND_TICK(50));

	return 0;
}
