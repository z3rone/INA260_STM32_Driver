/*
 * INA260.h
 *
 *  Created on: 18 Feb 2020
 *      Author: Falk
 */
#include "./lib/utils/utils_stm32.h"
#include "./lib/utils/utils_binary.h"
#include "./lib/uthash/src/uthash.h"

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

#define INA260_TIMEOUT 10

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

typedef enum {
	none,
	read_conf_it, read_u_it, read_i_it, read_p_it, write_conf_it,
	read_conf, read_u, read_i, read_p, write_conf
} ina260_comop;

struct INA260_Handle {
	uint16_t addr;
	I2C_HandleTypeDef* iface;
	bool reversed;
	ina260_comop operation;
	uint8_t rx_buf[2];
	uint8_t tx_buf[3];
	double u,i,p;
	uint16_t conf;
	UT_hash_handle hh;
};
void INA260_PutAvg(struct INA260_Handle* handle, INA260_avg avg_mode);
void INA260_PutUConv(struct INA260_Handle* handle, INA260_conv u_conv_t);
void INA260_PutIConv(struct INA260_Handle* handle, INA260_conv i_conv_t);
void INA260_PutOp(struct INA260_Handle* handle, INA260_op op_mode);

double INA260_ConvertU(uint16_t val);
double INA260_ConvertI(uint16_t val);
double INA260_ConvertP(uint16_t val);


HAL_StatusTypeDef INA260_SetConfig(struct INA260_Handle* handle);
HAL_StatusTypeDef INA260_GetConfig(struct INA260_Handle* handle);

HAL_StatusTypeDef INA260_SetConfig_IT(struct INA260_Handle* handle);
HAL_StatusTypeDef INA260_GetConfig_IT(struct INA260_Handle* handle);
HAL_StatusTypeDef INA260_GetU_IT(struct INA260_Handle* handle);
HAL_StatusTypeDef INA260_GetI_IT(struct INA260_Handle* handle);
HAL_StatusTypeDef INA260_GetP_IT(struct INA260_Handle* handle);

__weak void INA260_RxUCallback(struct INA260_Handle* handle);
__weak void INA260_RxICallback(struct INA260_Handle* handle);
__weak void INA260_RxPCallback(struct INA260_Handle* handle);
__weak void INA260_RxConfCallback(struct INA260_Handle* handle);
__weak void INA260_TxConfCallback(struct INA260_Handle* handle);

HAL_StatusTypeDef INA260_I2C_RxInterruptHandler(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef INA260_I2C_TxInterruptHandler(I2C_HandleTypeDef *hi2c);

#endif /* INC_INA260_H_ */
