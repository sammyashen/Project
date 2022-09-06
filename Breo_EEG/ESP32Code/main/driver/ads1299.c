#include "include.h"

#define BYTE0(dwTemp)	(*((uint8_t *)(&dwTemp) + 0))
#define BYTE1(dwTemp)	(*((uint8_t *)(&dwTemp) + 1))
#define BYTE2(dwTemp)	(*((uint8_t *)(&dwTemp) + 2))
#define BYTE3(dwTemp)	(*((uint8_t *)(&dwTemp) + 3))

#define ano_uart_tx		37
#define ano_uart_rx		38
#define ano_uart_num	2
#define ANO_SEND_BIT	BIT0
static EventGroupHandle_t s_ano_event_group;

spi_device_handle_t spi_ads1299;
static EventGroupHandle_t s_ads1299_event_group;
static float CH1_Vol = 0.0;

static void IRAM_ATTR ads1299_pin_drdy_isr_handler(void *arg)
{
	gpio_intr_disable(ADS1299_PIN_DRDY);
	xEventGroupSetBits(s_ads1299_event_group, DATA_READY_BIT);
}

static void ADS1299_ReadData(uint8_t *rxdata)
{
	esp_err_t ret;
	spi_transaction_t t;
	uint8_t txdata[27];

	memset(&txdata, 0, sizeof(txdata));
	memset(&t, 0, sizeof(t));
	t.length = 27*8;
	t.tx_buffer = txdata;
	t.rx_buffer = rxdata;
	ret = spi_device_transmit(spi_ads1299, &t);
	ESP_ERROR_CHECK(ret);
}

static void ADS1299_ReadReg(uint8_t addr, uint8_t *rxdata)
{
	esp_err_t ret;
	spi_transaction_t t;
	uint8_t txdata[3];

	txdata[0] = _RREG | addr;
	txdata[1] = 0x00;
	txdata[2] = 0x00;
	
	memset(&t, 0, sizeof(t));
	t.length = 3*8;
	t.tx_buffer = txdata;
	t.rx_buffer = rxdata;
	ret = spi_device_transmit(spi_ads1299, &t);
	ESP_ERROR_CHECK(ret);
}

static void ADS1299_WriteReg(uint8_t addr, uint8_t data)
{
	esp_err_t ret;
	spi_transaction_t t;
	uint8_t txdata[3];

	txdata[0] = _WREG | addr;
	txdata[1] = 0x00;
	txdata[2] = data;
	
	memset(&t, 0, sizeof(t));
	t.length = 3*8;
	t.tx_buffer = txdata;
	t.rx_buffer = NULL;
	ret = spi_device_transmit(spi_ads1299, &t);
	ESP_ERROR_CHECK(ret);
}

static void ADS1299_SendCMD(uint8_t cmd)
{
	esp_err_t ret;
	spi_transaction_t t;
	
	memset(&t, 0, sizeof(t));
	t.length = 8;
	t.flags = SPI_TRANS_USE_TXDATA;
	t.tx_data[0] = cmd;
	ret = spi_device_transmit(spi_ads1299, &t);
	ESP_ERROR_CHECK(ret);
}

static void ADS1299_DataHandle_Task(void *arg)
{
	uint8_t rxdata[27];
	uint32_t ch1data;

	while(1)
	{
		xEventGroupWaitBits(s_ads1299_event_group, DATA_READY_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

		ADS1299_ReadData(rxdata);

		{
			ch1data = (uint32_t)((rxdata[3] << 16) | (rxdata[4] << 8) | rxdata[5]);
			if(ch1data & 0x800000)
				CH1_Vol = (float)(0x1000000 - ch1data)*(-0.0224);//uV
			else
				CH1_Vol = (float)ch1data*0.0224;

			CH1_Vol = IIRNotching(IIRFilter(CH1_Vol, 0), 0);
		}

		gpio_intr_enable(ADS1299_PIN_DRDY);
		
		xEventGroupSetBits(s_ano_event_group, ANO_SEND_BIT);
    }
}

static void ANO_UartSendData_Task(void *arg)
{
	uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity	   = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB
	};
	ESP_ERROR_CHECK(uart_driver_install(ano_uart_num, 1024*2, 1024*2, 0, NULL, 0));
	ESP_ERROR_CHECK(uart_param_config(ano_uart_num, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(ano_uart_num, ano_uart_tx, ano_uart_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

	while(1)
	{
		uint8_t send_data[35];
		uint8_t send_cnt = 0;
		
		xEventGroupWaitBits(s_ano_event_group, ANO_SEND_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

		send_data[send_cnt++] = 0x88;
		send_data[send_cnt++] = 0xA1;
		send_data[send_cnt++] = 0x04;
		send_data[send_cnt++] = BYTE3(CH1_Vol);
		send_data[send_cnt++] = BYTE2(CH1_Vol);
		send_data[send_cnt++] = BYTE1(CH1_Vol);
		send_data[send_cnt++] = BYTE0(CH1_Vol);

		uint8_t sum = 0;
		for(uint8_t i=0;i<send_cnt;i++)
			sum += send_data[i];

		send_data[send_cnt++] = sum;
		
		uart_write_bytes(ano_uart_num, (const char *)send_data, send_cnt);
	}
}

void ADS1299_Init(void)
{
	esp_err_t ret;
	spi_bus_config_t buscfg = {
        .miso_io_num = ADS1299_PIN_MISO,	
        .mosi_io_num = ADS1299_PIN_MOSI,	
        .sclk_io_num = ADS1299_PIN_CLK,
        .quadwp_io_num = -1,			
        .quadhd_io_num = -1,						
	};
	spi_device_interface_config_t devcfg = {
    	.clock_speed_hz = 200*1000,		
    	.mode = 1,          			//SPI mode 1(CPOL = 0, CPHA = 1)
    	.spics_io_num = ADS1299_PIN_CS,
    	.queue_size = 7,	
	};

	//ads1299 reset
	gpio_reset_pin(ADS1299_PIN_RST);
	gpio_set_direction(ADS1299_PIN_RST, GPIO_MODE_OUTPUT);

	//ads1299 start
	gpio_reset_pin(ADS1299_PIN_START);
	gpio_set_direction(ADS1299_PIN_START, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin(ADS1299_PIN_DRDY);
	gpio_set_direction(ADS1299_PIN_DRDY, GPIO_MODE_INPUT);
	gpio_set_pull_mode(ADS1299_PIN_DRDY, GPIO_PULLUP_ONLY);
	gpio_set_intr_type(ADS1299_PIN_DRDY, GPIO_INTR_NEGEDGE);
	gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
	gpio_isr_handler_add(ADS1299_PIN_DRDY, ads1299_pin_drdy_isr_handler, (void *)ADS1299_PIN_DRDY);
	s_ads1299_event_group = xEventGroupCreate();
	xTaskCreate(ADS1299_DataHandle_Task, "ads1299_data_handle_task", 2048, NULL, 8, NULL);

	s_ano_event_group = xEventGroupCreate();
	xTaskCreate(ANO_UartSendData_Task, "ano uart send data handle task", 2048, NULL, 10, NULL);

	//spi
	ret = spi_bus_initialize(ADS1299_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    ret = spi_bus_add_device(ADS1299_HOST, &devcfg, &spi_ads1299);
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "SPI configure successed!");

	//³õÊ¼»¯ÐòÁÐ
    {
    	uint8_t rxdata[3];

    	gpio_set_level(ADS1299_PIN_RST, 0);
    	vTaskDelay(50/portTICK_RATE_MS);
    	gpio_set_level(ADS1299_PIN_RST, 1);
    	vTaskDelay(150/portTICK_RATE_MS);

    	gpio_set_level(ADS1299_PIN_START, 0);
    	ADS1299_SendCMD(_SDATAC);
    	ADS1299_ReadReg(ID, rxdata);
    	printf("ID;0x%x\r\n", rxdata[2]);
    	if(rxdata[2] == 0x3E)
    	{
			ADS1299_WriteReg(CONFIG1, 0x96);	//250SPS
			
//			ADS1299_WriteReg(CONFIG2, 0xD0);	//pluse test signal
//			ADS1299_WriteReg(CH1SET, 0x65);		

//			ADS1299_WriteReg(CONFIG2, 0xD0);	//noise test signal
//			ADS1299_WriteReg(CH1SET, 0x61);	

			ADS1299_WriteReg(CONFIG3, 0xEE);//EC
			ADS1299_WriteReg(CH1SET, 0x60);
			ADS1299_WriteReg(CH2SET, 0xE1);		
			ADS1299_WriteReg(CH3SET, 0xE1);
			ADS1299_WriteReg(CH4SET, 0xE1);
			ADS1299_WriteReg(CH5SET, 0xE1);
			ADS1299_WriteReg(CH6SET, 0xE1);
			ADS1299_WriteReg(CH7SET, 0xE1);
			ADS1299_WriteReg(CH8SET, 0xE1);
			ADS1299_WriteReg(BIAS_SENSP, 0x01);
			ADS1299_WriteReg(BIAS_SENSN, 0x01);
			ADS1299_WriteReg(MISC1, 0x20);	
			
			//loff dect
//			ADS1299_WriteReg(LOFF_SENSP, 0x00);	
//			ADS1299_WriteReg(LOFF_SENSN, 0x00);
//			ADS1299_WriteReg(LOFF_FLIP, 0x00);
//			ADS1299_WriteReg(LOFF, 0x00);//00		
//			ADS1299_WriteReg(CONFIG4, 0x00);	

			ADS1299_SendCMD(_RDATAC);
			vTaskDelay(100/portTICK_RATE_MS);
			gpio_set_level(ADS1299_PIN_START, 1);
    	}
    }
}



