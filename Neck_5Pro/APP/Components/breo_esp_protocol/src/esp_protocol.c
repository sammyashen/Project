#include <esp_protocol_cfg.h>

extern uint8_t data_send_to_esp(uint8_t *dat, uint8_t dat_len);

static uint16_t get_checkcrc(uint8_t *data, uint8_t length)
{
	uint8_t i;
    uint16_t crc = 0xffff;        //Initial value
    while(length--)
    {
        crc ^= (uint16_t)(*data++) << 8; // crc ^= (uint6_t)(*data)<<8; data++;
        for (i = 0; i < 8; ++i)
        {
            if ( crc & 0x8000 )
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

static bool esp_check_framehead(uint8_t framehead)
{
	if(framehead == ESP_FRAME_HEAD)		return true;
	else								return false;
}

static bool esp_check_frametail(uint8_t frametail)
{
	if(frametail == ESP_FRAME_TAIL)		return true;
	else								return false;
}

static bool esp_check_version(uint8_t version)
{
	if(version == ESP_VERSION)			return true;
	else								return false;
}

bool group_pack_to_esp(uint8_t pack_type, uint8_t pack_id, uint8_t ctrl, uint16_t cmd, uint8_t *dat, uint16_t dat_len)
{
	bool err = false;
	uint8_t buf[ESP_DAT_BUFF_SIZE];
	uint8_t send_len;

	switch(pack_type)
	{
		case 0x03:
		case 0x05:
		case 0x07:	//esp为响应方，mcu为请求方
			if((dat_len+15) > ESP_DAT_BUFF_SIZE)		err = false;
			else
			{
				buf[0] = ESP_FRAME_HEAD;
				buf[1] = ESP_VERSION;
				buf[2] = pack_id;
				buf[3] = pack_type;
				buf[4] = ctrl;
				buf[5] = 0x00;
				buf[6] = 0x00;
				//dat body
				buf[7] = (dat_len+3)>>8;
				buf[8] = (dat_len+3);
				buf[9] = 0x01;		//TODO:收到的命令包序号，这里先填充01，后续将记录收到的序号对应存储再填入。
				buf[10] = cmd>>8;
				buf[11] = cmd;
				memcpy(&buf[12], dat, dat_len);
				buf[12+dat_len] = get_checkcrc(&buf[1], dat_len+11)>>8;
				buf[13+dat_len] = get_checkcrc(&buf[1], dat_len+11);
				buf[14+dat_len] = ESP_FRAME_TAIL;

				send_len = data_send_to_esp(buf, 15+dat_len);
				if(send_len != 0)	err = true;
				else				err = false;
			}
		break;

		case 0x02:
		case 0x04:
		case 0x06:
		case 0x12:
		case 0x16:	//esp为请求方，mcu为响应方
			if((dat_len+14) > ESP_DAT_BUFF_SIZE)		err = false;
			else
			{
				buf[0] = ESP_FRAME_HEAD;
				buf[1] = ESP_VERSION;
				buf[2] = pack_id;
				buf[3] = pack_type;
				buf[4] = ctrl;
				buf[5] = 0x00;
				buf[6] = 0x00;
				//dat body
				buf[7] = (dat_len+2)>>8;
				buf[8] = (dat_len+2);
				buf[9] = cmd>>8;
				buf[10] = cmd;
				memcpy(&buf[11], dat, dat_len);
				buf[11+dat_len] = get_checkcrc(&buf[1], dat_len+10)>>8;
				buf[12+dat_len] = get_checkcrc(&buf[1], dat_len+10);
				buf[13+dat_len] = ESP_FRAME_TAIL;

				send_len = data_send_to_esp(buf, 14+dat_len);
				if(send_len != 0)	err = true;
				else				err = false;
			}
		break;
	}

	return err;
}

esp_err_enum unpack_from_esp(uint8_t *raw_frame, uint16_t raw_frame_len, breo_esp_protocol_t *unpack_dat)
{
	esp_err_enum err = esp_err_ok;
	uint8_t setoff,crc_msb,crc_lsb;
	uint16_t crc;

	switch(*(raw_frame+3))
	{
		case 0x02:
		case 0x04:
		case 0x06:
		case 0x12:
		case 0x16:	//esp为请求方，mcu为响应方
			if(raw_frame_len < 14)	
				err = esp_err_checkcrc;
			else
			{
				setoff = raw_frame_len - 1;
				crc_msb = *(raw_frame+setoff-2);
				crc_lsb = *(raw_frame+setoff-1);
				crc = (crc_msb<<8) | crc_lsb;
				if(!esp_check_framehead(*raw_frame))		
					err = esp_err_framehead;
				else if(!esp_check_frametail(*(raw_frame + setoff)))		
					err = esp_err_frametail; 
				else if(!esp_check_version(*(raw_frame+1)))		
					err = esp_err_version;
//				else if(get_checkcrc(raw_frame+1, raw_frame_len-4) != crc)
//					err = esp_err_checkcrc;
				else
				{
					unpack_dat->cmd = (raw_frame[9]<<8) | raw_frame[10];
					unpack_dat->dat_len = raw_frame_len-14;
					memcpy(&unpack_dat->dat, &raw_frame[11], raw_frame_len-14);
				}
			}
		break;

		case 0x03:
		case 0x05:
		case 0x07:	//esp为响应方，mcu为请求方
			if(raw_frame_len < 15)	
				err = esp_err_checkcrc;
			else
			{
				setoff = raw_frame_len - 1;
				crc_msb = *(raw_frame+setoff-2);
				crc_lsb = *(raw_frame+setoff-1);
				crc = (crc_msb<<8) | crc_lsb;
				if(!esp_check_framehead(*raw_frame))		
					err = esp_err_framehead;
				else if(!esp_check_frametail(*(raw_frame + setoff)))		
					err = esp_err_frametail; 
				else if(!esp_check_version(*(raw_frame+1)))		
					err = esp_err_version;
//				else if(get_checkcrc(raw_frame+1, raw_frame_len-4) != crc)
//					err = esp_err_checkcrc;
				else
				{
					unpack_dat->cmd = (raw_frame[10]<<8) | raw_frame[11];
					unpack_dat->dat_len = raw_frame_len-15;
					memcpy(&unpack_dat->dat, &raw_frame[12], raw_frame_len-15);
				}
			}
		break;

		default:
			err = esp_err_checkcrc;
		break;
	}

	return err;
}


