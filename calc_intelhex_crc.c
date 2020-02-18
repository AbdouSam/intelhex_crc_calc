#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define TEST

#ifdef TEST
#include "calc_crc_file.h"
#endif

#define PROG_ADDR_STR   (uint32_t)0x1D000000 /* The program code. */
#define PROG_ADDR_END   (uint32_t)0x1D1FFFFC /* The last program mem address. */
#define PROG_MEM_SIZE   (uint32_t)0x00200000 /* Memory size of PICMZ2048. */
#define MEM_LEN         (uint32_t)0x1FFFFF   /* Length of data to calculate crc. */

/* PIC32MZ flash memory */
static uint8_t pic_mem[PROG_MEM_SIZE];
static uint32_t pic_mem_index = 0;

#ifdef TEST
/*
 * @brief : Compare CRC with a known crc of hex file.
 */
static void test_function(uint8_t *data, uint32_t len)
{
  uint16_t crc_of_test_hex = 0x065D;

  uint16_t crc = calc_crc(data, len, true);

  if (crc != crc_of_test_hex)
  {
    printf("Error calc crc = %04X, file crc = %04X.\n", crc, crc_of_test_hex);
    exit(EXIT_FAILURE);
  }

  printf("Sucess! CRC  = %04X is Correct\n", crc);
}

#endif

/*
 * @brief Rebase the first address memory to zero on 
 *        the pic memory array. And perform check to 
 *        store only Data memory. 
 *
 * @memaddr : absolute memory address got from hex file.
 */

static int set_mem_index(char *memaddr)
{
  uint32_t addr = strtol(memaddr, NULL, 16);
  

  if (addr >= PROG_ADDR_STR && addr <= PROG_ADDR_END)
  {
    /* program mem. */
    pic_mem_index = (addr  - PROG_ADDR_STR);

    return 0;
  }
  else
  {
    /* config mem. */
    return -1;
  }
}

/*
 * @brief parse Data lines of the intel hex file.
 *
 * @param line : Data line of the hex file.
 */
static void parse_data_line(char *line)
{
  /* Bytecount */
  char temp_str[5];
  char temp_bytecount[3];
  char temp_byte[3];
  int dbyte_count = 0;
  uint8_t dbyte;
  uint16_t addr_offset;
  int i;

  /* Byte count */

  strncpy(temp_bytecount, line + 1, 2);
  temp_bytecount[2] = '\0';
  dbyte_count = strtol(temp_bytecount, NULL, 16);

  /* Address offset */

  strncpy(temp_str, line + 3, 4);
  temp_str[4] = '\0';
  addr_offset = strtol(temp_str, NULL, 16);
  
  /* Data */

  for(i = 0; i < dbyte_count; i++)
  {
    strncpy(temp_byte, line + 9 + i * 2, 2);
    temp_byte[2] = '\0';
    dbyte = strtol(temp_byte, NULL, 16);
    pic_mem[pic_mem_index + addr_offset + i] = dbyte;
  }

}

/*
 * @brief Parse the hex file into bytes, in the same order 
 *        of the MCU program memory.
 *
 * @param filename : name of the hex file.
 */

void get_bytes_from_hex(char *filename)
{

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

  /* Empty memory to 0xff. */

  memset(pic_mem, 0xFF, sizeof(pic_mem));

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
        /* Second Address line. */

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
          /* Config Data, don't store. */
          skip_data = false;
        }
      }
    }

    if (strstr(line, ":02000004") != NULL)
    {
      /* First Address line */

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
      /* End of file code. */

      printf("End of Hex file.\n");
    }
    else
    {
      /* Data lines */

      if (skip_data == false)
      {
        parse_data_line(line);
      }
    }
  }

  fclose(hex);
  if (line)
  {
    free(line);
  }

}

int main(int argc, char const *argv[])
{
  get_bytes_from_hex("hex_file.hex");

  #ifdef TEST
    test_function(pic_mem, MEM_LEN);
  #endif

  return 0;
}
