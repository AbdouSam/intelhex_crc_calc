import sys
from ctypes import *
import os 

PICMZ_PROG_MEM_LEN =   0x1FFFFF

def get_file_path():

  path = os.path.realpath(__file__)

  pathlist = path.split('/')

  # Remove the name of file.
  pathlist.pop()

  path = ['/' + word for word in pathlist]

  path.pop(0)
  path.append('/')

  path = ''.join(path)

  return path

def calc_crc_hex_file(hex_file):

  #crclib = cdll.LoadLibrary('./hexcrclib.so')  
  crclib = CDLL(get_file_path() + 'hexcrclib.so')
  
  crc = crclib.get_crc_hexfile(hex_file, PICMZ_PROG_MEM_LEN)  

  print("CRC is ", hex(crc))

def main():
  # Get hex
  try:
    hex_file = str(sys.argv[1])
  except:
    raise IOError('Hex file does not exist.')

  calc_crc_hex_file(hex_file)

if __name__ == '__main__':
  main()