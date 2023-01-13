#include "w25qxxConf.h"

void spi_read (spi_t spi, uint8_t* rx_buffer, size_t size){

}

void spi_write(spi_t spi, uint8_t* tx_buffer, size_t size){

}

void spi_write_read(spi_t spi, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t size){

}

void spi_write_then_read(spi_t spi, uint8_t* tx_buffer, size_t tx_size, uint8_t* rx_buffer, size_t rx_size){
    spi_write(spi, tx_buffer, tx_size);
    spi_read(spi, rx_buffer, rx_size);
}