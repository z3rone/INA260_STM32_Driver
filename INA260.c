#include "INA260.h"

static int16_t INA260_get_reg(struct INA260_Handle handle, uint8_t reg);
static HAL_StatusTypeDef INA260_sel_reg(struct INA260_Handle handle, uint8_t reg);
static HAL_StatusTypeDef INA260_set_reg(struct INA260_Handle handle, uint8_t reg, int16_t val);

HAL_StatusTypeDef INA260_config(struct INA260_Handle handle,
		INA260_avg avg_mode,
		INA260_conv u_conv_t,
		INA260_conv i_conv_t,
		INA260_op op_mode) {
	HAL_StatusTypeDef status;
	if((status = INA260_set_avg(handle, avg_mode))    != HAL_OK)
		return status;
	if((status = INA260_set_u_conv(handle, u_conv_t)) != HAL_OK)
		return status;
	if((status = INA260_set_i_conv(handle, i_conv_t)) != HAL_OK)
		return status;
	if((status = INA260_set_op(handle, op_mode))      != HAL_OK)
		return status;
	return HAL_OK;
}

HAL_StatusTypeDef INA260_reset(struct INA260_Handle handle) {
	return INA260_set_reg(handle, INA260_CONF_REG, 1<<15);
}

HAL_StatusTypeDef INA260_set_avg(struct INA260_Handle handle, INA260_avg avg_mode) {
	uint16_t conf = INA260_get_reg(handle, INA260_CONF_REG);
	conf = del_bits(conf, INA260_AVG_OFFSET, 3);
	conf |= avg_mode << INA260_AVG_OFFSET;
	return INA260_set_reg(handle, INA260_CONF_REG, conf);
}

HAL_StatusTypeDef INA260_set_u_conv(struct INA260_Handle handle, INA260_conv u_conv_t) {
	uint16_t conf = INA260_get_reg(handle, INA260_CONF_REG);
	conf &= del_bits(conf, INA260_U_CONV_OFFSET, 3);
	conf |= u_conv_t << INA260_U_CONV_OFFSET;
	return INA260_set_reg(handle, INA260_CONF_REG, conf);
}

HAL_StatusTypeDef INA260_set_i_conv(struct INA260_Handle handle, INA260_conv i_conv_t) {
	__IO uint16_t conf = INA260_get_reg(handle, INA260_CONF_REG);
	conf &= del_bits(conf, INA260_I_CONV_OFFSET, 3);
	conf |= i_conv_t << INA260_I_CONV_OFFSET;
	return INA260_set_reg(handle, INA260_CONF_REG, conf);
}

HAL_StatusTypeDef INA260_set_op(struct INA260_Handle handle, INA260_op op_mode) {
	uint16_t conf = INA260_get_reg(handle, INA260_CONF_REG);
	conf &= del_bits(conf, INA260_OP_MODE_OFFSET, 3);
	conf |= op_mode << INA260_OP_MODE_OFFSET;
	return INA260_set_reg(handle, INA260_CONF_REG, conf);
}

double INA260_get_u(struct INA260_Handle handle) {
	return INA260_get_reg(handle, INA260_U_REG) * INA260_U_FACT * (handle.reversed ? (-1) : 1);
}

double INA260_get_i(struct INA260_Handle handle) {
	return INA260_get_reg(handle, INA260_I_REG) * INA260_I_FACT * (handle.reversed ? (-1) : 1);
}

double INA260_get_p(struct INA260_Handle handle) {
	return INA260_get_reg(handle, INA260_P_REG) * INA260_P_FACT;
}

static int16_t INA260_get_reg(struct INA260_Handle handle, uint8_t reg) {
	uint8_t data[3] = {0};
	int err_count = 0;
	__disable_irq();
	INA260_sel_reg(handle, reg);
	__IO HAL_StatusTypeDef s;
	while((s=HAL_I2C_Master_Receive(handle.iface, handle.addr<<1, data, 2, INA260_TIMEOUT)) != HAL_OK) {
		// Count retrys and give up
		if(++err_count > INA260_RETRY) {
			return 0;
		}
		// Wait before retry
		HAL_Delay(INA260_RETRY_WAIT);
	}
	__enable_irq();
	return (data[0] << 8) | data[1];
}

static HAL_StatusTypeDef INA260_sel_reg(struct INA260_Handle handle, uint8_t reg) {
	HAL_StatusTypeDef status;

	__disable_irq();
	status = HAL_I2C_Master_Transmit(handle.iface, handle.addr<<1, &reg, 1, 1000);
	__enable_irq();

	return status;
}

static HAL_StatusTypeDef INA260_set_reg(struct INA260_Handle handle, uint8_t reg, int16_t val) {
	uint8_t data[3];
	data[0] = reg;
	data[1] = (val >> 8) & 0xFF;
	data[2] = (val >> 0) & 0xFF;

	HAL_StatusTypeDef status;

	__disable_irq();
	status = HAL_I2C_Master_Transmit(handle.iface, handle.addr<<1, data, 3, 1000);
	__enable_irq();

	return status;
}
