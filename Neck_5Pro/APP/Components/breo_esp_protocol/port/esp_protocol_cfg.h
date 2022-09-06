#ifndef __ESP_PROTOCOL_CFG_H
#define __ESP_PROTOCOL_CFG_H

#include <esp_protocol.h>

typedef struct breo_esp_protocol  breo_esp_protocol_t;

struct breo_esp_protocol
{
    uint16_t cmd;
    uint16_t dat_len;
    uint8_t dat[256];
};

typedef enum
{
	esp_err_ok = 0,
	esp_err_framehead,
	esp_err_frametail,
	esp_err_version,
	esp_err_checkcrc,
}esp_err_enum;



//USER TODO
#define ESP_DAT_BUFF_SIZE	50

#define ESP_FRAME_HEAD	0x5A
#define ESP_VERSION		0x01
#define ESP_FRAME_TAIL	0xA5


//外部接口
bool group_pack_to_esp(uint8_t pack_type, uint8_t pack_id, uint8_t ctrl, uint16_t cmd, uint8_t *dat, uint16_t dat_len);
esp_err_enum unpack_from_esp(uint8_t *raw_frame, uint16_t raw_frame_len, breo_esp_protocol_t *unpack_dat);




#endif




