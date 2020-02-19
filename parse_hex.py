"""Use C shared library to calculate the crc of a hex_file
"""
import sys
from ctypes import *
import os 

PICMZ_PROG_MEM_LEN =   0x1FFFFF

def get_file_path():
  """ctypes library requires the full path to the shared
  library.
  """
  pathlist = os.path.realpath(__file__).split('/')[1:-1]

  path = ['/' + word for word in pathlist]

  path = ''.join(path) + '/'

  return path

def calc_crc_hex_file(hex_file):
  """calculate crc16 of hex file using shared c library
  """

  crclib = CDLL(get_file_path() + 'hexcrclib.so')
  
  hex_file_p = create_string_buffer(hex_file.encode())

  return crclib.get_crc_hexfile(hex_file_p, PICMZ_PROG_MEM_LEN)  


def main():
  # Get hex
  try:
    hex_file = str(sys.argv[1])
  except:
    raise IOError('Enter a hex file as first argument.')

  crc = calc_crc_hex_file(hex_file)

  print(hex(crc))

if __name__ == '__main__':
  main()