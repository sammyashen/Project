#include <app_protocol_cfg.h>

extern uint8_t data_send(uint8_t *dat, uint8_t dat_len);

static uint8_t get_checksum(uint8_t *dat, uint8_t len)
{
	uint8_t sum = 0;
	
	for(uint8_t i=0;i<len;i++)
	{
		sum += (*dat++);
	}
	sum = ~sum;

	return sum;
}

static bool check_framehead(uint8_t framehead)
{
	if(framehead == FRAME_HEAD)		return true;
	else							return false;
}

static bool check_frametail(uint8_t frametail)
{
	if(frametail == FRAME_TAIL)		return true;
	else							return false;
}

static bool check_dev_type(uint8_t dev_type)
{
	if(dev_type == DEV_TYPE)		return true;
	else							return false;
}

static bool check_version(uint8_t version)
{
	if(version == VERSION)			return true;
	else							return false;
}

bool group_pack_to_app(uint8_t cmd, const uint8_t *dat, uint8_t dat_len, bool need_subpack)
{
	uint8_t buf[DAT_BUFF_SIZE];
	uint8_t send_len;

	if(need_subpack)
	{
		uint8_t cyc_times;
		uint8_t setoff;
		uint8_t remainder;

		if(DAT_BUFF_SIZE < 20)		return false;

		if((dat_len%11) == 0)	{cyc_times = dat_len/11;remainder = 0;}
		else					{cyc_times = (dat_len/11)+1;remainder = dat_len%11;}
		for(uint8_t i=0;i<cyc_times;i++)
		{
			buf[0] = FRAME_HEAD;
			buf[1] = DEV_TYPE;
			buf[2] = VERSION;
			buf[3] = cyc_times;
			buf[4] = i;
			if(remainder != 0)
			{
				if((cyc_times - i) > 1)		buf[5] = 12;
				else 						buf[5] = (dat_len%11) + 1;
			}
			else
			{
				buf[5] = 12;
			}
			buf[6] = cmd;
			memcpy(&buf[7], dat+11*i, buf[5]-1);
			setoff = 6+buf[5];
			buf[setoff] = get_checksum(&buf[1], buf[5]+5);
			buf[setoff+1] = FRAME_TAIL;

			send_len = data_send(buf, 8+buf[5]);
			if(send_len == 0)	break;
		}

		if(send_len != 0)	return true;
		else				return false;
	}
	else
	{
		if((dat_len+9) > DAT_BUFF_SIZE)		return false;
	
		buf[0] = FRAME_HEAD;
		buf[1] = DEV_TYPE;
		buf[2] = VERSION;
		buf[3] = 0x01;
		buf[4] = 0x00;
		//dat body
		buf[5] = dat_len + 1;
		buf[6] = cmd;
		memcpy(&buf[7], dat, dat_len);
		buf[7+dat_len] = get_checksum(&buf[1], dat_len+6);
		buf[8+dat_len] = FRAME_TAIL;

		send_len = data_send(buf, 9+dat_len);
		if(send_len != 0)	return true;
		else				return false;
	}
}

err_enum unpack_from_app(uint8_t *raw_frame, uint8_t raw_frame_len, breo_app_protocol_t *unpack_dat)
{
	err_enum err = err_ok;
	uint8_t setoff;

	if(raw_frame_len < 9)	
		err = err_checksum;
	else
	{
		setoff = raw_frame_len - 1;
		if(!check_framehead(*raw_frame))		
			err = err_framehead;
		else if(!check_frametail(*(raw_frame + setoff)))		
			err = err_frametail; 
		else if(!check_dev_type(*(raw_frame+1)))		
			err = err_devtype; 
		else if(!check_version(*(raw_frame+2)))		
			err = err_version;
		else if(get_checksum(raw_frame+1, raw_frame_len-3) != *(raw_frame+setoff-1))
			err = err_checksum;
		else
		{
			unpack_dat->cmd = raw_frame[6];
			unpack_dat->dat_len = raw_frame_len-9;
			memcpy(&unpack_dat->dat, &raw_frame[7], raw_frame_len-9);
		}
	}
	
	return err;
}


