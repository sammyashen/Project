#ifndef __APP_PROTOCOL_CFG_H
#define __APP_PROTOCOL_CFG_H

#include <app_protocol.h>

typedef struct breo_app_protocol  breo_app_protocol_t;

struct breo_app_protocol
{
    uint8_t cmd;
    uint8_t dat_len;
    uint8_t dat[128];
};

typedef enum
{
	err_ok = 0,
	err_framehead,
	err_frametail,
	err_devtype,
	err_version,
	err_checksum,
}err_enum;



//USER TODO
#define DAT_BUFF_SIZE	50

#define FRAME_HEAD	0x4E
#define DEV_TYPE	0x81
#define VERSION		0x01
#define FRAME_TAIL	0x4E

//外部接口
bool group_pack_to_app(uint8_t cmd, const uint8_t *dat, uint8_t dat_len, bool need_subpack);
err_enum unpack_from_app(uint8_t *raw_frame, uint8_t raw_frame_len, breo_app_protocol_t *unpack_dat);



#endif




