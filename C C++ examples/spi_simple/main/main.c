#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"

#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK  12
#define PIN_NUM_CS   10
#define SPI_NUM    SPI2_HOST

#define DECODE_MODE_REG     0x09
#define INTENSITY_REG       0x0A
#define SCAN_LIMIT_REG      0x0B
#define SHUTDOWN_REG        0x0C
#define DISPLAY_TEST_REG    0x0F

spi_device_handle_t spi;

static void write_reg(uint8_t reg, uint8_t value) {
    uint8_t tx_data[2] = { reg, value };

    spi_transaction_t t = {
    	.flags=SPI_TRANS_USE_TXDATA,
        .tx_buffer = tx_data,
        .length = 2 * 8
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));
}

uint32_t read_reg(uint8_t reg, uint8_t value) {
    uint8_t rx_data[2] = { reg, value };

    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_RXDATA,
        .rx_buffer = rx_data,
        .length = 2 * 8
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));

    spi_device_release_bus(spi);

    return *(uint32_t*)t.rx_data;
}

static void clear(void) {
  for (int i = 0; i < 8; i++) {
    write_reg(i + 1, 0x00);
  }
}

void app_main(void)
{
	esp_err_t ret;
    spi_device_handle_t spi;
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=16*320*2+8
    };
    spi_device_interface_config_t devcfg={
            .clock_speed_hz = 1000000,  // 1 MHz
            .mode = 0,                  //SPI mode 0
            .spics_io_num = PIN_NUM_CS,
            .queue_size = 1,
            .flags = SPI_DEVICE_HALFDUPLEX,
            .pre_cb = NULL,
            .post_cb = NULL,                        //We want to be able to queue 7 transactions at a time
    };
    //Initialize the SPI bus
        ret=spi_bus_initialize(SPI_NUM, &buscfg, SPI_DMA_CH_AUTO);
        ESP_ERROR_CHECK(ret);
        //Attach the LCD to the SPI bus
        ret=spi_bus_add_device(SPI_NUM, &devcfg, &spi);
        ESP_ERROR_CHECK(ret);

        write_reg(DISPLAY_TEST_REG, 0);
        write_reg(SCAN_LIMIT_REG, 7);
        write_reg(DECODE_MODE_REG, 0);
        write_reg(SHUTDOWN_REG, 1);
        clear();

    while (true) {
    	write_reg(DISPLAY_TEST_REG, 7);
        printf("Hello from app_main!\n");
//        sleep(1);
    }
}
