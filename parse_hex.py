"""Use C shared library to calculate the crc of a hex_file
"""
import sys
from ctypes import *
import os 

import pathlib
import platform

PICMZ_PROG_MEM_LEN    = 0x1FFFFF
PICMZ_PROG_MEM_START  = 0x1D000000 # physical addressing starting with 0x1d

def get_file_path():
  """ctypes library requires the full path to the shared
  library.
  """

  return pathlib.Path(os.path.realpath(__file__)).parents[0]

def calc_crc_hex_file(hex_file, mem_len, mem_start):
  """calculate crc16 of hex file using shared c library
  """
  try:
    if platform.system() == "Windows":
      file_path = str(get_file_path() / 'hexcrclib.dll')

    else:
      file_path = str(get_file_path() / 'hexcrclib.so')

    crclib = CDLL(file_path)
  except Exception:
    raise FileNotFoundError("Library file does not exist.")
  
  hex_file_p = create_string_buffer(hex_file.encode())

  return crclib.get_crc_hexfile(hex_file_p, mem_len, mem_start)


def main():
  # Get hex
  try:
    hex_file = str(sys.argv[1])
  except:
    raise IOError('Enter a hex file as first argument.')

  # Example of calculating hex file for PIC32MZ2048EFM's full memory
  crc = calc_crc_hex_file(hex_file, PICMZ_PROG_MEM_LEN, PICMZ_PROG_MEM_START)

  print(hex(crc))

if __name__ == '__main__':
  main()