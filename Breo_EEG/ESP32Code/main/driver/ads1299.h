#ifndef _ADS1299_H
#define _ADS1299_H

//spi
#define ADS1299_HOST    		SPI2_HOST
#define ADS1299_PIN_MISO 		10
#define ADS1299_PIN_MOSI 		11
#define ADS1299_PIN_CLK  		12
#define ADS1299_PIN_CS   		13

#define ADS1299_PIN_START		7
#define ADS1299_PIN_RST			8
#define ADS1299_PIN_DRDY		9

#define DATA_READY_BIT			BIT0


// SPI Command Definitions (Datasheet, 35)
#define _WAKEUP  		0x02 // Wake-up from standby mode
#define _STANDBY 		0x04 // Enter Standby mode
#define _RESET   		0x06 // Reset the device registers to default
#define _START   		0x08 // Start and restart (synchronize) conversions
#define _STOP    		0x0A // Stop conversion
#define _RDATAC  		0x10 // Enable Read Data Continuous mode (default mode at power-up)
#define _SDATAC  		0x11 // Stop Read Data Continuous mode
#define _RDATA   		0x12 // Read data by command; supports multiple read back
#define _RREG    		0x20 // Read Register
#define _WREG    		0x40 // Write to Register

// Register Addresses
#define ID         		0x00
#define CONFIG1    		0x01
#define CONFIG2    		0x02
#define CONFIG3    		0x03
#define LOFF       		0x04
#define CH1SET     		0x05
#define CH2SET     		0x06
#define CH3SET     		0x07
#define CH4SET     		0x08
#define CH5SET     		0x09
#define CH6SET     		0x0A
#define CH7SET     		0x0B
#define CH8SET     		0x0C
#define BIAS_SENSP 		0x0D
#define BIAS_SENSN 		0x0E
#define LOFF_SENSP 		0x0F
#define LOFF_SENSN 		0x10
#define LOFF_FLIP  		0x11
#define LOFF_STATP 		0x12
#define LOFF_STATN 		0x13
#define GPIO       		0x14
#define MISC1      		0x15
#define MISC2      		0x16
#define CONFIG4    		0x17

// Gains
#define ADS1299_PGA_GAIN01 			(0b00000000)
#define ADS1299_PGA_GAIN02 			(0b00010000)
#define ADS1299_PGA_GAIN04 			(0b00100000)
#define ADS1299_PGA_GAIN06 			(0b00110000)
#define ADS1299_PGA_GAIN08 			(0b01000000)
#define ADS1299_PGA_GAIN12 			(0b01010000)
#define ADS1299_PGA_GAIN24 			(0b01100000)

// Input Modes - Channels
#define ADS1299_INPUT_PWR_DOWN   	(0b10000000)
#define ADS1299_INPUT_PWR_UP     	(0b00000000)
#define ADS1299_INPUT_NORMAL     	(0b00000000)
#define ADS1299_INPUT_SHORTED    	(0b00000001)
#define ADS1299_INPUT_MEAS_BIAS  	(0b00000010)
#define ADS1299_INPUT_SUPPLY     	(0b00000011)
#define ADS1299_INPUT_TEMP       	(0b00000100)
#define ADS1299_INPUT_TESTSIGNAL 	(0b00000101)
#define ADS1299_INPUT_SET_BIASP  	(0b00000110)
#define ADS1299_INPUT_SET_BIASN  	(0b00000111)
#define ADS1299_INPUT_SRB2_OPEN		(0b00000000)
#define ADS1299_INPUT_SRB2_CLOSE	(0b00001000)


// Test Signal Choices - p41
#define ADS1299_TEST_INT              		(0b00010000)
#define ADS1299_TESTSIGNAL_AMP_1X     		(0b00000000)
#define ADS1299_TESTSIGNAL_AMP_2X     		(0b00000100)
#define ADS1299_TESTSIGNAL_PULSE_SLOW 		(0b00000000)
#define ADS1299_TESTSIGNAL_PULSE_FAST 		(0b00000001)
#define ADS1299_TESTSIGNAL_DCSIG      		(0b00000011)
#define ADS1299_TESTSIGNAL_NOCHANGE   		(0b11111111)
#define ADS1299_TESTSIGNAL_NOTUSE			(0b00000010)

//Lead-off Signal Choices
#define LOFF_MAG_6NA 				(0b00000000)
#define LOFF_MAG_24NA 				(0b00000100)
#define LOFF_MAG_6UA 				(0b00001000)
#define LOFF_MAG_24UA 				(0b00001100)
#define LOFF_FREQ_DC 				(0b00000000)
#define LOFF_FREQ_7p8HZ 			(0b00000001)
#define LOFF_FREQ_31p2HZ 			(0b00000010)
#define LOFF_FREQ_FS_4 				(0b00000011)
#define PCHAN 						(1)
#define NCHAN 						(2)
#define BOTHCHAN 					(3)

void ADS1299_Init(void);


#endif

