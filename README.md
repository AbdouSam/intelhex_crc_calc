# intelhex_crc_calc
Calculate crc16 of hex file (intel hex) for Microchip Bootloader crc check.

The crc calculation is alot faster in C, we used shared library by compiling the 
file calc_crc_file.c with the command

cc -fPIC -shared -o libcrc.so calc_crc_file.c

This shared library is used in python