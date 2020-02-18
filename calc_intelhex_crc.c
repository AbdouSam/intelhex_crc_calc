#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "calc_crc_file.h"

#define TEST

#define PICMZ_PROG_ADDR_STR   (uint32_t)0x1D000000 /* The program code. */
#define PICMZ_PROG_ADDR_END   (uint32_t)0x1D1FFFFC /* The last program mem address. */
#define PICMZ_PROG_MEM_LEN    (uint32_t)0x1FFFFF   /* Length of data to calculate crc. */

/* PIC32MZ flash memory */
static uint8_t mem_mcu[PICMZ_PROG_MEM_LEN];
static uint32_t mem_mcu_index = 0;

/*
 * @brief Rebase the first address memory to zero on 
 *        the pic memory array. And perform check to 
 *        store only Data. 
 *
 * @memaddr : absolute address got from hex file.
 * @return : true, skip next address line, false, don't skip.
 */

static bool set_mem_index(char *memaddr)
{
  uint32_t addr = strtol(memaddr, NULL, 16);
  

  if (addr >= PICMZ_PROG_ADDR_STR && addr <= PICMZ_PROG_ADDR_END)
  {
    /* program mem. */
    mem_mcu_index = (addr  - PICMZ_PROG_ADDR_STR);

    return false;
  }
  else
  {
    /* config mem. */
    return true;
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
    mem_mcu[mem_mcu_index + addr_offset + i] = dbyte;
  }

}


/*
 * @brief Parse the hex file into bytes, in the same order 
 *        of the MCU program memory. Calculate its CRC.
 *
 * @param filename : name of the hex file.
 *
 * @return CRC16 of the hex file.
 */

uint16_t get_crc_hexfile(char *filename, uint32_t mem_len)
{

  FILE *hex = fopen(filename, "r");

  char *line = NULL;
  size_t len = 0;
  ssize_t readlen;
  bool addrline = false;
  bool skip_data = false;
  char temp_str_low[9];
  char temp_str_high[9];
  char * mem_addr_full;
  uint16_t crc;

  /* Empty memory to 0xff. */

  memset(mem_mcu, 0xFF, sizeof(mem_mcu));

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
        
        if (set_mem_index(mem_addr_full))
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

        if (set_mem_index(mem_addr_full))
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
  
  crc = calc_crc(mem_mcu, mem_len, true);
  return crc;
}

#ifdef TEST
int main(int argc, char const *argv[])
{
  const char *filename = NULL;
  uint16_t crc;

  if (argc > 1)
  {
    filename = argv[1];
  }

  if(filename != NULL)
  {
    crc = get_crc_hexfile((char *)filename, PICMZ_PROG_MEM_LEN);
  }
  else
  {
    printf("Err Hex file not specified.\n");

    exit(EXIT_FAILURE);
  }

  return 0;
}
#endif
