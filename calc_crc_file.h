#ifndef _CALC_CRC_FILE_H
#define _CALC_CRC_FILE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief : Calculate crc16 of a hex file Data memory.
 * 
 * @param data : Data byte array to calculate crc
 * @param len : length of data in bytes
 * @param reverse: to reverse the byte's order of the 16bit crc.
 */
uint16_t calc_crc(uint8_t *data, uint32_t len, bool reverse);

#endif /* _CALC_CRC_FILE_H */