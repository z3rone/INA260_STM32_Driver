#include "./INA260.h"

static I2C_TypeDef* INA260_i2c_order[INA260_I2C_NUM] = {0};
static uint8_t  INA260_rx_buf[INA260_I2C_NUM][2];
static uint8_t  INA260_tx_buf[INA260_I2C_NUM][3];
static uint16_t INA260_lastAddr[INA260_I2C_NUM];

static int16_t INA260_get_reg(struct INA260_Handle handle, uint8_t reg);
static HAL_StatusTypeDef INA260_sel_reg(struct INA260_Handle handle, uint8_t reg);
static HAL_StatusTypeDef INA260_set_reg(struct INA260_Handle handle, uint8_t reg, int16_t val);

static HAL_StatusTypeDef INA260_get_reg_IT(struct INA260_Handle handle, uint8_t reg);
static HAL_StatusTypeDef INA260_sel_reg_IT(struct INA260_Handle handle, uint8_t reg);
static HAL_StatusTypeDef INA260_set_reg_IT(struct INA260_Handle handle, uint8_t reg, int16_t val);

static int INA260_GetIfaceIndex(I2C_TypeDef* iface);

HAL_StatusTypeDef INA260_set_config(struct INA260_Handle handle, INA260_Config config) {
	return INA260_set_reg(handle, INA260_CONF_REG, config);
}

INA260_Config INA260_get_config(struct INA260_Handle handle) {
	INA260_get_config_IT(handle);

	HAL_I2C_StateTypeDef state;
	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(HAL_I2C_GetState(handle.iface) == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	return INA260_get_read(handle.iface->Instance).data;
}

HAL_StatusTypeDef INA260_set_config_IT(struct INA260_Handle handle, INA260_Config config) {
	HAL_StatusTypeDef status = INA260_set_reg_IT(handle, INA260_CONF_REG, config);
	HAL_I2C_StateTypeDef state;
	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(HAL_I2C_GetState(handle.iface) == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}
	return status;
}

HAL_StatusTypeDef INA260_get_config_IT(struct INA260_Handle handle) {
	return INA260_get_reg_IT(handle, INA260_CONF_REG);
}

HAL_StatusTypeDef INA260_reset(struct INA260_Handle handle) {
	return INA260_set_reg(handle, INA260_CONF_REG, 1<<15);
}

INA260_Config INA260_put_avg(INA260_Config conf, INA260_avg avg_mode) {
	conf = del_bits(conf, INA260_AVG_OFFSET, 3);
	conf |= avg_mode << INA260_AVG_OFFSET;
	return conf;
}

INA260_Config INA260_put_u_conv(INA260_Config conf, INA260_conv u_conv_t) {
	conf = del_bits(conf, INA260_U_CONV_OFFSET, 3);
	conf |= u_conv_t << INA260_U_CONV_OFFSET;
	return conf;
}

INA260_Config INA260_put_i_conv(INA260_Config conf, INA260_conv i_conv_t) {
	conf = del_bits(conf, INA260_I_CONV_OFFSET, 3);
	conf |= i_conv_t << INA260_I_CONV_OFFSET;
	return conf;
}

INA260_Config INA260_put_op(INA260_Config conf, INA260_op op_mode) {
	conf = del_bits(conf, INA260_OP_MODE_OFFSET, 3);
	conf |= op_mode << INA260_OP_MODE_OFFSET;
	return conf;
}

HAL_StatusTypeDef INA260_set_avg(struct INA260_Handle handle, INA260_avg avg_mode) {
	INA260_get_config_IT(handle);

	HAL_I2C_StateTypeDef state;
	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	INA260_Config conf = INA260_get_read(handle.iface->Instance).data;
	conf = INA260_put_avg(conf, avg_mode);
	INA260_set_config_IT(handle, conf);

	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef INA260_set_u_conv(struct INA260_Handle handle, INA260_conv u_conv_t) {
	INA260_get_config_IT(handle);

	HAL_I2C_StateTypeDef state;
	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	INA260_Config conf = INA260_get_read(handle.iface->Instance).data;
	conf = INA260_put_u_conv(conf, u_conv_t);
	INA260_set_config_IT(handle, conf);

	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef INA260_set_i_conv(struct INA260_Handle handle, INA260_conv i_conv_t) {
	INA260_get_config_IT(handle);

	HAL_I2C_StateTypeDef state;
	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	INA260_Config conf = INA260_get_read(handle.iface->Instance).data;
	conf = INA260_put_i_conv(conf, i_conv_t);
	INA260_set_config_IT(handle, conf);

	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef INA260_set_op(struct INA260_Handle handle, INA260_op op_mode) {
	INA260_get_config_IT(handle);

	HAL_I2C_StateTypeDef state;
	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	INA260_Config conf = INA260_get_read(handle.iface->Instance).data;
	conf = INA260_put_op(conf, op_mode);
	INA260_set_config_IT(handle, conf);

	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	return HAL_OK;
}


double INA260_convert_u(int16_t val, bool reversed) {
	return val*INA260_U_FACT * (reversed ? (-1) : 1);
}

double INA260_convert_i(int16_t val, bool reversed) {
	return val * INA260_I_FACT * (reversed ? (-1) : 1);
}

double INA260_convert_p(int16_t val) {
	return val*INA260_P_FACT;
}

HAL_StatusTypeDef INA260_get_u_IT(struct INA260_Handle handle) {
	return INA260_get_reg_IT(handle, INA260_U_REG);
}

HAL_StatusTypeDef INA260_get_i_IT(struct INA260_Handle handle) {
	return INA260_get_reg_IT(handle, INA260_I_REG);
}

HAL_StatusTypeDef INA260_get_p_IT(struct INA260_Handle handle) {
	return INA260_get_reg_IT(handle, INA260_P_REG);
}

double INA260_get_u(struct INA260_Handle handle) {
	return INA260_convert_u(INA260_get_reg(handle, INA260_U_REG), handle.reversed);
}

double INA260_get_i(struct INA260_Handle handle) {
	return INA260_convert_i(INA260_get_reg(handle, INA260_I_REG), handle.reversed);
}

double INA260_get_p(struct INA260_Handle handle) {
	return INA260_convert_p(INA260_get_reg(handle, INA260_P_REG));
}

static HAL_StatusTypeDef INA260_get_reg_IT(struct INA260_Handle handle, uint8_t reg) {
	INA260_sel_reg_IT(handle, reg);
	HAL_I2C_StateTypeDef state;

	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	int index = INA260_GetIfaceIndex(handle.iface->Instance);
	return HAL_I2C_Master_Receive_IT(handle.iface, handle.addr<<1, INA260_rx_buf[index], 2);
}

static int16_t INA260_get_reg(struct INA260_Handle handle, uint8_t reg) {
	INA260_get_reg_IT(handle, reg);
	HAL_I2C_StateTypeDef state;
	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}
	struct INA260_Read read = INA260_get_read(handle.iface->Instance);
	return read.data;
}

static HAL_StatusTypeDef INA260_sel_reg_IT(struct INA260_Handle handle, uint8_t reg) {
	HAL_StatusTypeDef status;

	status = HAL_I2C_Master_Transmit_IT(handle.iface, handle.addr<<1, &reg, 1);

	HAL_I2C_StateTypeDef state;
	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(HAL_I2C_GetState(handle.iface) == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	return status;
}

static HAL_StatusTypeDef INA260_sel_reg(struct INA260_Handle handle, uint8_t reg) {
	HAL_StatusTypeDef status;

	status = HAL_I2C_Master_Transmit_IT(handle.iface, handle.addr<<1, &reg, 1);

	HAL_I2C_StateTypeDef state;
	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	return status;
}

static HAL_StatusTypeDef INA260_set_reg_IT(struct INA260_Handle handle, uint8_t reg, int16_t val) {
	int index = INA260_GetIfaceIndex(handle.iface->Instance);

	uint8_t* data = INA260_tx_buf[index];
	set_bits_lsbf(data, reg, 0, 8);
	set_bits_lsbf(data, val, 8, 16);
	change_byte_order(data+1, 2);

	return HAL_I2C_Master_Transmit_IT(handle.iface, handle.addr<<1, data, 3);
}

static HAL_StatusTypeDef INA260_set_reg(struct INA260_Handle handle, uint8_t reg, int16_t val) {
	INA260_set_reg_IT(handle, reg, val);

	HAL_I2C_StateTypeDef state;
	while((state = HAL_I2C_GetState(handle.iface)) != HAL_I2C_STATE_READY) {
		if(state == HAL_I2C_STATE_ERROR) return HAL_ERROR;
	}

	return HAL_OK;
}

struct INA260_Read INA260_get_read(I2C_TypeDef* def) {
	int index = INA260_GetIfaceIndex(def);
	uint16_t data = get_bits(INA260_rx_buf[index], 0, 16);
	change_byte_order(&data, 2);
	struct INA260_Read read = {
		INA260_lastAddr[index],
		data
	};
	return read;
}

static int INA260_GetIfaceIndex(I2C_TypeDef* iface) {
	for(int i = 0; i < INA260_I2C_NUM; i++) {
		if(INA260_i2c_order[i] == 0) INA260_i2c_order[i] = iface;
		if(INA260_i2c_order[i] == iface) return i;
	}
	return 0;
}
