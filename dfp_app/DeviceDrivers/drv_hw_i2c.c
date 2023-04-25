#include "drv_hw_i2c.h"

void i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *data, uint32_t timeout)
{
	volatile static uint32_t time_cnt;
	//send START
	I2C_ConfigAck(I2C1, ENABLE);
	I2C_GenerateStart(I2C1, ENABLE);
	time_cnt = 0;
	while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_MODE_FLAG)){
		if(++time_cnt >= timeout)
			goto Failed;
	}
	//send AD+W,wait ACK
	I2C_SendAddr7bit(I2C1, dev_addr, I2C_DIRECTION_SEND);
	time_cnt = 0;
	while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_TXMODE_FLAG)){
		if(++time_cnt >= timeout)
			goto Failed;
	}
	//send reg,wait ACK
	I2C_SendData(I2C1, reg_addr);
	time_cnt = 0;
	while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDING)){
		if(++time_cnt >= timeout)
			goto Failed;
	}
	time_cnt = 0;
	while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDED)){
		if(++time_cnt >= timeout)
			goto Failed;
	}
	//send START
	I2C_ConfigAck(I2C1, ENABLE);
	I2C_GenerateStart(I2C1, ENABLE);
	time_cnt = 0;
	while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_MODE_FLAG)){
		if(++time_cnt >= timeout)
			goto Failed;
	}
	//send AD+R,wait ACK
	I2C_SendAddr7bit(I2C1, dev_addr, I2C_DIRECTION_RECV);
	time_cnt = 0;
	while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_RXMODE_FLAG)){
		if(++time_cnt >= timeout)
			goto Failed;
	}
	//read data, send NACK
	if(len == 1)
    {
        I2C_ConfigAck(I2C1, DISABLE);
        (void)(I2C1->STS1); /// clear ADDR
        (void)(I2C1->STS2);
        I2C_GenerateStop(I2C1, ENABLE);
        time_cnt = 0;
        while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_RECVD_FLAG)){
        	if(++time_cnt >= timeout)
				goto Failed;
        }
        *data++ = I2C_RecvData(I2C1);
        len--;
    }
    else if(len == 2)
    {
        I2C1->CTRL1 |= 0x0800; /// set ACKPOS
        (void)(I2C1->STS1);
        (void)(I2C1->STS2);
        I2C_ConfigAck(I2C1, DISABLE);
        time_cnt = 0;
        while (!I2C_GetFlag(I2C1, I2C_FLAG_BYTEF)){
        	if(++time_cnt >= timeout)
				goto Failed;
        }
        I2C_GenerateStop(I2C1, ENABLE);
        
        *data++ = I2C_RecvData(I2C1);
        len--;
        *data++ = I2C_RecvData(I2C1);
        len--;
    }
    else
    {
        I2C_ConfigAck(I2C1, ENABLE);
        (void)(I2C1->STS1);
        (void)(I2C1->STS2);
        
        while (len)
        {
            if (len == 3)
            {
            	time_cnt = 0;
                while(!I2C_GetFlag(I2C1, I2C_FLAG_BYTEF)){
                	if(++time_cnt >= timeout)
						goto Failed;
                }
                I2C_ConfigAck(I2C1, DISABLE);
                *data++ = I2C_RecvData(I2C1);
                len--;
                time_cnt = 0;
                while(!I2C_GetFlag(I2C1, I2C_FLAG_BYTEF)){
                	if(++time_cnt >= timeout)
						goto Failed;
                }
                I2C_GenerateStop(I2C1, ENABLE);
        
                *data++ = I2C_RecvData(I2C1);
                len--;
                *data++ = I2C_RecvData(I2C1);
                len--;
                
                break;
            }
            time_cnt = 0;
            while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_RECVD_FLAG)){
            	if(++time_cnt >= timeout)
					goto Failed;
            }
            *data++ = I2C_RecvData(I2C1);
            len--;
        }
    }
    return;

Failed:
	I2C_GenerateStop(I2C1, ENABLE);
	return;
}

void i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, uint32_t len, uint8_t *data, uint32_t timeout)
{
	volatile static uint32_t time_cnt;
	//send START
	I2C_ConfigAck(I2C1, ENABLE);
	I2C_GenerateStart(I2C1, ENABLE);
	time_cnt = 0;
	while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_MODE_FLAG)){
		if(++time_cnt >= timeout)
			goto Failed;
	}
	//send AD+W,wait ACK
	I2C_SendAddr7bit(I2C1, dev_addr, I2C_DIRECTION_SEND);
	time_cnt = 0;
	while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_TXMODE_FLAG)){
		if(++time_cnt >= timeout)
			goto Failed;
	}
	//send reg,wait ACK
	I2C_SendData(I2C1, reg_addr);
	time_cnt = 0;
	while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDING)){
		if(++time_cnt >= timeout)
			goto Failed;
	}
	time_cnt = 0;
	while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDED)){
		if(++time_cnt >= timeout)
			goto Failed;
	}
	//send data,wait ACK
	while (len-- > 0)
    {
        I2C_SendData(I2C1, *data++);
        time_cnt = 0;
        while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDING)){
        	if(++time_cnt >= timeout)
				goto Failed;
        }
    }
    time_cnt = 0;
    while(!I2C_CheckEvent(I2C1, I2C_EVT_MASTER_DATA_SENDED)){
    	if(++time_cnt >= timeout)
			goto Failed;
    }
    //send STOP
    I2C_GenerateStop(I2C1, ENABLE);
	return;

Failed:
	I2C_GenerateStop(I2C1, ENABLE);
	return;
}

void i2c_hw_init(void)
{
	I2C_InitType I2C_InitStructures;
    GPIO_InitType GPIO_InitStructures;
    
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_I2C1, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);

    /*PB8 -- SCL; PB9 -- SDA*/
    GPIO_InitStruct(&GPIO_InitStructures);
    GPIO_InitStructures.Pin               = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStructures.GPIO_Slew_Rate    = GPIO_Slew_Rate_High;
    GPIO_InitStructures.GPIO_Mode         = GPIO_Mode_AF_OD;
    GPIO_InitStructures.GPIO_Alternate    = GPIO_AF4_I2C1;
    GPIO_InitStructures.GPIO_Pull         = GPIO_Pull_Up;	  
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructures);

    I2C_DeInit(I2C1);
    I2C_InitStructures.BusMode     = I2C_BUSMODE_I2C;
    I2C_InitStructures.FmDutyCycle = I2C_FMDUTYCYCLE_2;
    I2C_InitStructures.AckEnable   = I2C_ACKEN;
    I2C_InitStructures.AddrMode    = I2C_ADDR_MODE_7BIT;
    I2C_InitStructures.ClkSpeed    = 100000; // 100K

    I2C_Init(I2C1, &I2C_InitStructures);
    I2C_Enable(I2C1, ENABLE);
}

