#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef TEST
#include "calc_crc_file.h"
#endif

#define PROG_ADDR_STR   (uint32_t)0x1D000000 /* The program code. */
#define PROG_ADDR_END   (uint32_t)0x1D1FFFFC /* The last program mem address. */
#define PROG_MEM_SIZE   (uint32_t)0x00200000
#define EMPTY_MEM_WORD  (uint32_t)0xFFFFFFFF

static uint8_t pic_mem[PROG_MEM_SIZE];
static uint32_t pic_mem_addr[PROG_MEM_SIZE];
static uint32_t pic_mem_index = 0;
static uint32_t pic_mem_offset = 0;

#ifdef TEST
static void test_function(char *filename)
{
  uint32_t crc_of_tes_hex = 0x5D06;

  uint32_t crc = calc_crc_file("hex_file.hex");

  if (crc != crc_of_tes_hex)
  {
    printf("Error calc crc = %X, file crc = %X.\n", crc, crc_of_tes_hex);
    exit(EXIT_FAILURE);
  }

  printf("Sucess! CRC  = %X is Correct\n", crc);
}
#endif

int set_mem_index(char *memaddr)
{
  uint32_t addr = strtol(memaddr, NULL, 16);
  

  if (addr >= PROG_ADDR_STR && addr <= PROG_ADDR_END)
  {
    /* program mem. */
    pic_mem_index = (addr  - PROG_ADDR_STR) / 4;

    return 0;
  }
  else
  {
    /* config mem. */
    return -1;
  }
}

void parse_data_line(char *line, uint32_t lineindex)
{
  /* Bytecount */
  char temp_str[5];
  char temp_bytecount[3];
  char temp_byte[3];
  int dbyte_count = 0;
  uint8_t dbyte;
  uint16_t addr_offset;
  int i;

  strncpy(temp_bytecount, line + 1, 2);
  temp_bytecount[2] = '\0';
  dbyte_count = strtol(temp_bytecount, NULL, 16);

  /* Address offset */
  strncpy(temp_str, line + 3, 4);
  temp_str[4] = '\0';
  addr_offset = strtol(temp_str, NULL, 16);
  addr_offset = addr_offset / 4;
  pic_mem_addr[lineindex] = addr_offset;

  /* Data */
  for(i = 0; i < dbyte_count; i++)
  {
    strncpy(temp_byte, line + 9 + i * 2, 2);
    temp_byte[2] = '\0';

    dbyte = strtol(temp_byte, NULL, 16);
    pic_mem[pic_mem_index + addr_offset + i] = dbyte;
  }
}

void get_bytes_from_hex(char *filename)
{
  /* PIC32MZ flash memory */
  //uint32_t pic_mem[PROG_MEM_SIZE];
  FILE *hex = fopen(filename, "r");
  char *line = NULL;
  size_t len = 0;
  ssize_t readlen;
  bool addrline = false;
  bool skip_data = false;
  uint32_t lineindex = 0;
  char temp_str_low[9];
  char temp_str_high[9];
  char * mem_addr_full;

  if (NULL == hex)
  {
    printf("Hex file Does not exist.\n");

    exit(EXIT_FAILURE);
  }

  while((readlen = getline(&line, &len, hex)) != -1)
  {
    if (addrline == true)
    {

      if (strstr(line, ":02000004") == NULL)
      {
        addrline = false;
        memset(temp_str_high, '0', 4);
        temp_str_high[4] = '\0';
        mem_addr_full = strcat(temp_str_low, temp_str_high);
        mem_addr_full[8] = '\0';
        
        if (set_mem_index(mem_addr_full) < 0)
        {
          skip_data = true;
        }
        else
        {
          skip_data = false;
        }
      }
    }

    if (strstr(line, ":02000004") != NULL)
    {
      /* Address line */

      if (addrline == false)
      {
        addrline = true;
        strncpy(temp_str_low, line + 9, 4);
        temp_str_low[4] = '\0';

      }
      else
      {
        addrline = false;

        strncpy(temp_str_high, line + 9, 4);
        temp_str_high[4] = '\0';
        mem_addr_full = strcat(temp_str_high, temp_str_low);
        mem_addr_full[8] = '\0';

        if (set_mem_index(mem_addr_full) < 0)
        {
          skip_data = true;
        }
        else
        {
          skip_data = false;
        }
      }

    }
    else if (strstr(line, ":00000001") != NULL)
    {
      printf("End of Hex file.\n");
    }
    else
    {
      /* Data lines */
      /* Byte count */
      if (skip_data == false)
      {
        parse_data_line(line, lineindex);
        lineindex++;
      }

    }

  }

  fclose(hex);
  if (line)
  {
    free(line);
  }

  printf("Data mem : \n");
  for(int i = 0; i < 30; i++)
  {
    printf("%02X ", pic_mem[i]);
  }
  printf("\n");
  for(int i = 0; i < 20; i++)
  {
    printf("%d\n", pic_mem_addr[i]);
  }
}

int main(int argc, char const *argv[])
{
  get_bytes_from_hex("hex_file.hex");
  
  #ifdef TEST
  test_function("hex_file.hex");
  #endif

  return 0;
}
