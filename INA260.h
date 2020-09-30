/*
 * INA260.h
 *
 *  Created on: 18 Feb 2020
 *      Author: Falk
 */
#include "./lib/utils/utils_stm32.h"
#include "./lib/utils/utils_binary.h"

#include <stdbool.h>
#include <sys/types.h>
#include <stm32f3xx_hal.h>

#ifndef INC_INA260_H_
#define INC_INA260_H_

#define INA260_I2C_NUM    6
#define INA260_TIMEOUT    10
#define INA260_RETRY_WAIT 5
#define INA260_RETRY      2

#define INA260_CONF_REG 0x00
#define INA260_I_REG    0x01
#define INA260_U_REG    0x02
#define INA260_P_REG    0x03
#define INA260_MASK_REG 0x06
#define INA260_ALIM_REG 0x07
#define INA260_MID_REG  0xFE
#define INA260_DID_REG  0xFF

#define INA260_I_FACT 001.25
#define INA260_U_FACT 001.25
#define INA260_P_FACT 010

#define INA260_AVG_OFFSET     9
#define INA260_U_CONV_OFFSET  6
#define INA260_I_CONV_OFFSET  3
#define INA260_OP_MODE_OFFSET 0

typedef uint16_t INA260_Config;

typedef enum {
	_1    = 0b000,
	_4    = 0b001,
	_16   = 0b010,
	_64   = 0b011,
	_128  = 0b100,
	_256  = 0b101,
	_512  = 0b110,
	_1024 = 0b111
} INA260_avg;

typedef enum {
	_140us  = 0b000,
	_204us  = 0b001,
	_332us  = 0b010,
	_588us  = 0b011,
	_1100us = 0b100,
	_2116us = 0b101,
	_4156us = 0b110,
	_8244us = 0b111
} INA260_conv;

typedef enum {
	pwr_dwn = 0b000,
	i_trig  = 0b001,
	u_trig  = 0b010,
	iu_trig = 0b011,
	//pwr_dwn2 = 0b100,
	i_cont  = 0b101,
	u_cont  = 0b110,
	iu_cont = 0b111
} INA260_op;

struct INA260_Handle {
	uint16_t addr;
	I2C_HandleTypeDef* iface;
	bool reversed;
};

struct INA260_Read {
	uint16_t addr;
	uint16_t data;
};

INA260_Config     INA260_get_config(struct INA260_Handle handle);
HAL_StatusTypeDef INA260_set_config(struct INA260_Handle handle, INA260_Config config);
HAL_StatusTypeDef INA260_set_config_IT(struct INA260_Handle handle, INA260_Config config);
HAL_StatusTypeDef INA260_get_config_IT(struct INA260_Handle handle);

HAL_StatusTypeDef INA260_reset(struct INA260_Handle handle);
INA260_Config INA260_put_avg(INA260_Config conf, INA260_avg avg_mode);
INA260_Config INA260_put_u_conv(INA260_Config conf, INA260_conv u_conv_t);
INA260_Config INA260_put_i_conv(INA260_Config conf, INA260_conv i_conv_t);
INA260_Config INA260_put_op(INA260_Config conf, INA260_op op_mode);

HAL_StatusTypeDef INA260_set_avg(struct INA260_Handle handle, INA260_avg avg_mode);
HAL_StatusTypeDef INA260_set_u_conv(struct INA260_Handle handle, INA260_conv u_conv_t);
HAL_StatusTypeDef INA260_set_i_conv(struct INA260_Handle handle, INA260_conv i_conv_t);
HAL_StatusTypeDef INA260_set_op(struct INA260_Handle handle, INA260_op op_mode);

double INA260_convert_u(int16_t val, bool reversed);
double INA260_convert_i(int16_t val, bool reversed);
double INA260_convert_p(int16_t val);

double INA260_get_u(struct INA260_Handle handle);
double INA260_get_i(struct INA260_Handle handle);
double INA260_get_p(struct INA260_Handle handle);

HAL_StatusTypeDef INA260_get_u_IT(struct INA260_Handle handle);
HAL_StatusTypeDef INA260_get_i_IT(struct INA260_Handle handle);
HAL_StatusTypeDef INA260_get_p_IT(struct INA260_Handle handle);

struct INA260_Read INA260_get_read(I2C_TypeDef* def);

#endif /* INC_INA260_H_ */
