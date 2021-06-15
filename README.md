# intelhex_crc_calc

Calculate crc16 of hex file (intel hex) for Microchip Bootloader Software crc check.

The `C` is integrated with `python3`'s standard library `ctypes` to use the C code as part of a python script.


### compile and Test the C code

gcc calc_intelhex_crc.c calc_crc_file.c -o intelhex

**Run**

./intelhex

### Generate the shared library

**For Linux**

gcc -fPIC -shared -o hexcrclib.so calc_intelhex_crc.c calc_crc_file.c

**For windows**

gcc -fPIC -shared -o hexcrclib.dll calc_intelhex_crc.c calc_crc_file.c

**Note:**

- the shared library used in windows should be generated on a windows machine.

- generating the shared lib from a linux machine won't be valid for a windows machine.