#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <error.h>

#include "calc_crc_file.h"


#define PICMZ_PROG_ADDR_STR  (uint32_t)0x1D000000 /* The program code. */
#define PICMZ_PROG_ADDR_END  (uint32_t)0x1D1FFFFC /* The last program mem address. */
#define PICMZ_PROG_MEM_LEN   (uint32_t)0x1FFFFF   /* Length of data to calculate crc. */
#define PICMZ_PROG_ADDR_MAX  (uint32_t)(0x1D000000 + 0x1FFFFF)/* Maximum address. */
/*
 *
 */
static ssize_t getline_m(char **lineptr, size_t *n, FILE *stream);

/* PIC32MZ flash memory */
static uint8_t mem_mcu[PICMZ_PROG_MEM_LEN];
static uint32_t mem_mcu_index = 0;

/*
 * @brief check memory length and memory address validity check
 *
 * @mem_len : length of data memory to calculate CRC over.
 * @mem_start: starting memory address to calculate CRC from.
 * 
 * @return true if memory valid, false otherwise
 */

static bool memory_valid(int mem_len, int *mem_start)
{
  
  int  moffset;

  if (mem_len < 0 || mem_len >  PICMZ_PROG_MEM_LEN)
  {
    printf("Invalid memory length %X.\n", mem_len);
    return false;
  }

  moffset = *mem_start - PICMZ_PROG_ADDR_STR;

  if (moffset < 0 ||  (PICMZ_PROG_ADDR_STR + moffset + mem_len) > PICMZ_PROG_ADDR_MAX)
  {
    printf("Invalid memory offset %X.\n", moffset);
    return false;
  }
  
  *mem_start = moffset;
  return true;
}

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

uint16_t get_crc_hexfile(char *filename, uint32_t mem_len, uint32_t mem_start)
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

  /* Check memory validity */

  if (!memory_valid(mem_len, &mem_start))
  {
    exit(EXIT_FAILURE);
  }

  /* Empty memory to 0xff. */

  memset(mem_mcu, 0xFF, sizeof(mem_mcu));

  if (NULL == hex)
  {
    printf("Hex file Does not exist.\n");

    exit(EXIT_FAILURE);
  }

  while((readlen = getline_m(&line, &len, hex)) != -1)
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
  
  crc = calc_crc(mem_mcu + mem_start, mem_len, true);
  return crc;
}


/* The original code is public domain -- Will Hartung 4/9/09 */
/* Modifications, public domain as well, by Antti Haapala, 11/10/17
   - Switched to getc on 5/23/19 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

static ssize_t getline_m(char **lineptr, size_t *n, FILE *stream)
{
  size_t pos;
  int c;

  if (lineptr == NULL || stream == NULL || n == NULL) 
  {
    errno = EINVAL;
    return -1;
  }

  c = getc(stream);
  if (c == EOF) 
  {
    return -1;
  }

  if (*lineptr == NULL) 
  {
    *lineptr = malloc(128);
    if (*lineptr == NULL) 
    {
      return -1;
    }
    *n = 128;
  }

  pos = 0;
  while(c != EOF) 
  {
    if (pos + 1 >= *n) 
    {
      size_t new_size = *n + (*n >> 2);
      if (new_size < 128) 
      {
        new_size = 128;
      }

      char *new_ptr = realloc(*lineptr, new_size);
      if (new_ptr == NULL) 
      {
        return -1;
      }

      *n = new_size;
      *lineptr = new_ptr;
    }

    ((unsigned char *)(*lineptr))[pos ++] = c;
    if (c == '\n') 
    {
      break;
    }
    c = getc(stream);
  }

  (*lineptr)[pos] = '\0';
  return pos;
}



#ifdef TEST
int main(int argc, char const *argv[])
{
  const char *filename = NULL;
  uint32_t mem_start = PICMZ_PROG_ADDR_STR;
  uint32_t mem_len = PICMZ_PROG_MEM_LEN;

  uint16_t crc;

  if (argc > 1)
  {
    filename = argv[1];
  }

  if (argc > 3)
  {
    mem_len = (uint32_t)strtol(argv[2], NULL, 16);
    mem_start = (uint32_t)strtol(argv[3], NULL, 16);
  }


  if(filename != NULL)
  {
    crc = get_crc_hexfile((char *)filename, mem_len, mem_start);
    printf("crc = %04X\n", crc);
  }
  else
  {
    printf("Err Hex file not specified.\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}
#endif
