#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MEM_LEN 0x1fffff

static uint8_t pic_mem[MEM_LEN] = {};

static const uint16_t crc_table[16] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};


static uint32_t calc_crc(uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint16_t crc = 0;
    
    while(len--)
    {
        i = (crc >> 12) ^ (*data >> 4);
        crc = crc_table[i & 0x0F] ^ (crc << 4);
        i = (crc >> 12) ^ (*data >> 0);
        crc = crc_table[i & 0x0F] ^ (crc << 4);
        data++;
    }

    return (crc & 0xFFFF);
}

uint32_t calc_crc_file(void)
{
  int i = 0;

  FILE *f = fopen("byte_mem.txt", "r");
  
  unsigned int hex_tempo;

  for(i = 0; i < MEM_LEN; i++)
  {
    fscanf(f, "%02X ", &hex_tempo);
    pic_mem[i] = hex_tempo;
  }

  fclose(f);

  return calc_crc(pic_mem, MEM_LEN);
}