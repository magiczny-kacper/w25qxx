#ifndef _W25QXXCONFIG_H
#define _W25QXXCONFIG_H

#define W25qxx_Delay(delay) osDelay(delay)

#define _W25QXX_SPI                   hspi1
#define _W25QXX_USE_FREERTOS          1
#define _W25QXX_DEBUG                 0

#define port_t uint8_t
#define pin_t uint8_t
#define spi_t uint8_t*

#define MEM_SELECT (port, pin) (HAL_GPIO_WritePin(port, pin, 0))
#define MEM_DESELECT (pin, port) (HAL_GPIO_WritePin(port, pin, 1))



#endif
