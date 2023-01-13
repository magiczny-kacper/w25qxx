#ifndef W25QXXCONF_H_
#define W25QXXCONF_H_

#include <stdint.h>

#define W25QXX_DUMMY_BYTE 0xA5

#define W25qxx_Delay(delay) osDelay(delay)

#define _W25QXX_SPI                   hspi1
#define _W25QXX_USE_FREERTOS          1
// #define _W25QXX_DEBUG

#define port_t uint8_t
#define pin_t uint8_t
#define spi_t uint8_t*

#define MEM_SELECT(port, pin) (HAL_GPIO_WritePin(port, pin, 0))
#define MEM_DESELECT(port, pin) (HAL_GPIO_WritePin(port, pin, 1))

void spi_read (spi_t spi, uint8_t* rx_buffer, size_t size);

void spi_write(spi_t spi, uint8_t* tx_buffer, size_t size);

void spi_write_read(spi_t spi, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t size);

void spi_write_then_read(spi_t spi, uint8_t* tx_buffer, size_t tx_size, uint8_t* rx_buffer, size_t rx_size);

#endif /* W25QXXCONF_H_ */
