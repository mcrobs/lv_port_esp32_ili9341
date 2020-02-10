/**
 * @file tp_spi.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "tp_spi.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <string.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
static spi_device_handle_t spi;

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void tp_spi_init(void)
{

	esp_err_t ret;
/*
	spi_bus_config_t buscfg={
		.miso_io_num=TP_SPI_MISO,
		.mosi_io_num=TP_SPI_MOSI,
		.sclk_io_num=TP_SPI_CLK,
		.quadwp_io_num=-1,
		.quadhd_io_num=-1
	};
*/
	spi_device_interface_config_t devcfg={
		.clock_speed_hz=2*1000*1000,           //Clock out at 80 MHz
		.mode=0,                                //SPI mode 0
		.spics_io_num=TP_SPI_CS,              //CS pin
		.queue_size=7,
		.pre_cb=NULL,
		.post_cb=NULL,
	};
	devcfg.flags = SPI_DEVICE_HALFDUPLEX;
/*
	//Initialize the SPI bus
	ret=spi_bus_initialize(VSPI_HOST, &buscfg, 2);
	assert(ret==ESP_OK);
*/
	//Attach the LCD to the SPI bus
	ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
	assert(ret==ESP_OK);
}

uint8_t tp_spi_xchg(uint8_t data_send)
{
    uint8_t data_recv = 0;
    
    spi_transaction_t t = {
        .length = 8, // length is in bits
        .tx_buffer = &data_send,
        .rx_buffer = &data_recv
    };

    spi_device_queue_trans(spi, &t, portMAX_DELAY);

    spi_transaction_t * rt;
    spi_device_get_trans_result(spi, &rt, portMAX_DELAY);

    return data_recv;
}

int16_t iot_xpt2046_readdata( const uint8_t command)
{
    /**
     * Half duplex mode is not compatible with DMA when both writing and reading phases exist.
     * try to use command and address field to replace the write phase.
    */
    spi_transaction_ext_t t = (spi_transaction_ext_t) {
        .base = {
            .flags = SPI_TRANS_VARIABLE_CMD | SPI_TRANS_USE_RXDATA,
            .cmd = command,
            .rxlength = 2 * 8, // Total data length received, in bits
        },
        .command_bits = 8,
    };

    assert(spi_device_transmit(spi, (spi_transaction_t *)&t) == ESP_OK);
    return (t.base.rx_data[0] << 8 | t.base.rx_data[1]) >> 3;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
