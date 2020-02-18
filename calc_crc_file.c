#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "calc_crc_file.h"

static const uint16_t crc_table[16] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};


uint16_t calc_crc(uint8_t *data, uint32_t len, bool reverse)
{
    uint32_t i;
    uint16_t crc = 0;
    uint16_t crc_tmp = 0;

    while(len--)
    {
        i = (crc >> 12) ^ (*data >> 4);
        crc = crc_table[i & 0x0F] ^ (crc << 4);
        i = (crc >> 12) ^ (*data >> 0);
        crc = crc_table[i & 0x0F] ^ (crc << 4);
        data++;
    }

    if(reverse)
    {
      crc_tmp |= (crc >> 8 | crc << 8);
      crc = crc_tmp;
    }

    return (crc & 0xFFFF);
}