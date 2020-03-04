# INA260_STM32_Driver
An [INA260](http://www.ti.com/product/INA260) driver for STM32Cube's Hardware Abstraction Layer (HAL).

Example:
```
struct INA260_Handle handle;
handle.iface = &hi2c1; // I2C Interface
handle.addr = 0b1000000; // I2C Address
double voltage = INA260_get_u(handle);
double current = INA260_get_i(handle);
double power   = INA260_get_p(handle);
```
