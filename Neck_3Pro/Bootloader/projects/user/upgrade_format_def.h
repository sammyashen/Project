#ifndef UPGRADE_FORMAT_DEF_H_
#define UPGRADE_FORMAT_DEF_H_

#include <stdint.h>

#define UPGRADE_PACKET_HEAD_SIZE    128

#define PACKET_HEAD_FLAG            0x52617221
#define PACKET_VERSION              0x30303156

#define UPGRADE_START               0x1A1A2B2B
#define UPGRADE_DONE                0x3C3C4D4D

struct upgrade_packet_head_desc
{
    uint32_t head_flag;
    uint8_t version[4];
    uint32_t file_size;
    uint32_t file_checksum;
    //uint32_t head_check;
    //uint8_t reserve[];
};

#endif


