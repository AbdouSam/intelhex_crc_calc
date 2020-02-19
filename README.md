# intelhex_crc_calc

Calculate crc16 of hex file (intel hex) for Microchip Bootloader Software crc check.

The `C` is integrated with `python3`'s standard library `ctypes` to use the C code as part of a python script.


### compile the C code

gcc cal_intelhex_crc.c calc_crc_file.c -o intelhex

**Run**

./intelhex

### Generate the shared library

cc -fPIC -shared -o hexcrclib.so.so cal_intelhex_crc.c calc_crc_file.c
