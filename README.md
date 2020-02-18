# intelhex_crc_calc

Calculate crc16 of hex file (intel hex) for Microchip Bootloader Software crc check.

The `C` is integrated with `python`'s standard library `ctypes` to use the C code as part
of a python script.

cc -fPIC -shared -o libcrc.so cal_intelhex_crc.c calc_crc_file.c
