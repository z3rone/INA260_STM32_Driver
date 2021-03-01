#include "./INA260.h"


static struct INA260_Handle *occupations;

static HAL_StatusTypeDef INA260_OccupyBus(struct INA260_Handle* dev);
static HAL_StatusTypeDef INA260_ReleaseBus(struct INA260_Handle* dev);

void INA260_PutAvg(struct INA260_Handle* handle, INA260_avg avg_mode) {
	handle->conf = del_bits(handle->conf, INA260_AVG_OFFSET, 3);
	handle->conf |= avg_mode << INA260_AVG_OFFSET;
}

void INA260_PutUConv(struct INA260_Handle* handle, INA260_conv u_conv_t) {
	handle->conf = del_bits(handle->conf, INA260_U_CONV_OFFSET, 3);
	handle->conf |= u_conv_t << INA260_U_CONV_OFFSET;
}

void INA260_PutIConv(struct INA260_Handle* handle, INA260_conv i_conv_t) {
	handle->conf = del_bits(handle->conf, INA260_I_CONV_OFFSET, 3);
	handle->conf |= i_conv_t << INA260_I_CONV_OFFSET;
}

void INA260_PutOp(struct INA260_Handle* handle, INA260_op op_mode) {
	handle->conf = del_bits(handle->conf, INA260_OP_MODE_OFFSET, 3);
	handle->conf |= op_mode << INA260_OP_MODE_OFFSET;
}

double INA260_ConvertU(uint16_t val) {
	return val*INA260_U_FACT;
}

double INA260_ConvertI(uint16_t val) {
	return val * INA260_I_FACT;
}

double INA260_ConvertP(uint16_t val) {
	return val*INA260_P_FACT;
}

HAL_StatusTypeDef INA260_SetConfig(struct INA260_Handle* handle) {
	HAL_StatusTypeDef status = INA260_OccupyBus(handle);
	if(status!=HAL_OK) {
		return status;
	}
	handle->operation = write_conf;

	handle->tx_buf[0] = INA260_CONF_REG;
	handle->tx_buf[1] = handle->conf >> 8;
	handle->tx_buf[2] = handle->conf && 0xFF;

	status = HAL_I2C_Master_Transmit(handle->iface, handle->addr << 1, handle->tx_buf, 3, INA260_TIMEOUT);

	handle->operation = none;
	INA260_ReleaseBus(handle);

	return status;
}

HAL_StatusTypeDef INA260_GetConfig(struct INA260_Handle* handle) {
	HAL_StatusTypeDef status = INA260_OccupyBus(handle);
	if(status!=HAL_OK) {
		return status;
	}
	handle->operation = read_conf;

	handle->tx_buf[0] = INA260_CONF_REG;

	status = HAL_I2C_Master_Transmit_IT(handle->iface, handle->addr << 1, handle->tx_buf, 1);

	if(status!=HAL_OK) {
		return status;
	}

	status = HAL_I2C_Master_Receive_IT(handle->iface, handle->addr << 1, handle->rx_buf, 1);

	handle->conf = handle->rx_buf[0]<<8 | handle->rx_buf[1];

	handle->operation = none;
	INA260_ReleaseBus(handle);

	return status;
}

HAL_StatusTypeDef INA260_SetConfig_IT(struct INA260_Handle* handle) {
	HAL_StatusTypeDef status = INA260_OccupyBus(handle);
	if(status!=HAL_OK) {
		return status;
	}

	handle->operation = write_conf_it;

	handle->tx_buf[0] = INA260_CONF_REG;
	handle->tx_buf[1] = handle->conf >> 8;
	handle->tx_buf[2] = handle->conf && 0xFF;

	return HAL_I2C_Master_Receive_IT(handle->iface, handle->addr << 1, handle->tx_buf, 3);
}

HAL_StatusTypeDef INA260_GetConfig_IT(struct INA260_Handle* handle) {
	HAL_StatusTypeDef status = INA260_OccupyBus(handle);
	if(status!=HAL_OK) {
		return status;
	}

	handle->operation = read_conf_it;
	handle->tx_buf[0] = INA260_CONF_REG;

	return HAL_I2C_Master_Transmit_IT(handle->iface, handle->addr << 1, handle->tx_buf, 1);
}

HAL_StatusTypeDef INA260_GetU_IT(struct INA260_Handle* handle) {
	HAL_StatusTypeDef status = INA260_OccupyBus(handle);
	if(status!=HAL_OK) {
		return status;
	}

	handle->operation = read_u_it;
	handle->tx_buf[0] = INA260_U_REG;
	return HAL_I2C_Master_Transmit_IT(handle->iface, handle->addr << 1, handle->tx_buf, 1);
}

HAL_StatusTypeDef INA260_GetI_IT(struct INA260_Handle* handle) {
	HAL_StatusTypeDef status = INA260_OccupyBus(handle);
	if(status!=HAL_OK) {
		return status;
	}

	handle->operation = read_i_it;
	handle->tx_buf[0] = INA260_I_REG;
	return HAL_I2C_Master_Transmit_IT(handle->iface, handle->addr << 1, handle->tx_buf, 1);
}

HAL_StatusTypeDef INA260_GetP_IT(struct INA260_Handle* handle) {
	HAL_StatusTypeDef status = INA260_OccupyBus(handle);
	if(status!=HAL_OK) {
		return status;
	}

	handle->operation = read_p_it;
	handle->tx_buf[0] = INA260_P_REG;
	return HAL_I2C_Master_Transmit_IT(handle->iface, handle->addr << 1, handle->tx_buf, 1);
}

HAL_StatusTypeDef INA260_I2C_TxInterruptHandler(I2C_HandleTypeDef *hi2c) {
	struct INA260_Handle* dev = NULL;
	HASH_FIND_PTR(occupations, hi2c->Instance, dev);

	if(dev==NULL) {
		return HAL_ERROR;
	}

	switch(dev->operation) {
	case read_conf_it:
	case read_u_it:
	case read_i_it:
	case read_p_it: {
		return HAL_I2C_Master_Receive_IT(hi2c, dev->addr << 1, dev->rx_buf, 2);
	}
	case write_conf_it: {
		INA260_TxConfCallback(dev);
		INA260_ReleaseBus(dev);
		dev->operation = none;
		return HAL_OK;
	}
	default: break;
	}

	return HAL_OK;
}

HAL_StatusTypeDef INA260_I2C_RxInterruptHandler(I2C_HandleTypeDef *hi2c) {
	struct INA260_Handle* dev = NULL;
	HASH_FIND_PTR(occupations, hi2c->Instance, dev);

	if(dev==NULL) {
		return HAL_ERROR;
	}

	uint16_t value = dev->rx_buf[0]<<8 | dev->rx_buf[1];

	switch(dev->operation) {
	case read_conf_it: {
		dev->conf = value;
		INA260_RxConfCallback(dev);
		break;
	}
	case read_u_it: {
		dev->u = INA260_ConvertU(value);
		INA260_RxUCallback(dev);
		break;
	}
	case read_i_it: {
		dev->i = INA260_ConvertI(value);
		INA260_RxICallback(dev);
		break;
	}
	case read_p_it: {
		dev->p = INA260_ConvertP(value);
		INA260_RxPCallback(dev);
		break;
	}
	default: break;
	}

	dev->operation = none;
	INA260_ReleaseBus(dev);

	return HAL_OK;
}

static HAL_StatusTypeDef INA260_OccupyBus(struct INA260_Handle* dev) {
	struct INA260_Handle* match = NULL;
	HASH_FIND_PTR(occupations, dev->iface->Instance, match);

	if(match != NULL) {
		return HAL_BUSY;
	}

	HASH_ADD_PTR(occupations, iface->Instance, dev);

	return HAL_OK;
}

static HAL_StatusTypeDef INA260_ReleaseBus(struct INA260_Handle* dev) {
	HASH_DEL(occupations, dev);

	return HAL_OK;
}
