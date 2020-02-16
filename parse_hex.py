import sys
import collections
import ctypes
import os 

PROG_ADDR_STR  = 0x1D000000 # The program code
PROG_ADDR_END  = 0x1D1FFFFC # the last program mem address
PROG_MEM_SIZE  = 0x200000
EMPTY_MEM_WORD = [0xFF, 0xFF, 0xFF, 0xFF]

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

  with open(hex_file) as hexfile:
    hexlines = hexfile.read().splitlines()

  line_index = 0
  skip_line = False
  
  # Create an empty Flash memory for PIC32MZ of size
  pic_mem = {a:EMPTY_MEM_WORD \
             for a in range(PROG_ADDR_STR, PROG_ADDR_STR + PROG_MEM_SIZE, 4)}

  for line in hexlines:

    line_index+= 1

    if skip_line:
      skip_line = False
      continue

    if line.find(':02000004') == 0:
      # Line sets a reference address.
      mem_addr = line[9:-2]

      # Check if next line is an Address line
      if(len(hexlines[line_index]) < 16):
        mem_addr = hexlines[line_index][9:-2] + mem_addr
        skip_line = True

      else:
        # Some Address comes in one line only, fill the 16bit with 0
        mem_addr+='0000'
      
    elif line.find(':00000001') == 0:
      # End of file
      print("Read Hex file end.")

    else:
      # Data Lines Hex code '00'

      try:
        byte_count = int(line[1:3],16)
      except:
        raise ValueError('Hex Data is Corrupt.')

      addr_offset = line[3:7]
      data = line[9:9 + byte_count*2]

      try:
        data_addr   = int(mem_addr,16)
        addr_offset = int(addr_offset, 16)

      except ValueError as e:
        raise e

      if data_addr + addr_offset <= PROG_ADDR_END:
        # Get the 32 bit words
        range_b_count = int(byte_count / 4)

        for k in range(range_b_count):
          try:
            data_a = [data[k*8 + 2*x: k*8 + 2*x + 2] for x in (0, 1, 2, 3)]
            data_i = [int(x, 16) for x in data_a]

            # Write to Mem
            pic_mem[data_addr + addr_offset + k*4] = data_i

          except:
            raise KeyError('Dict key does not exist.')
      else:
        # Config registers data.
        pass

  # Order the Dict, for the CRC calculation to be correct.
  pic_mem = collections.OrderedDict(sorted(pic_mem.items()))

  # Get values in list
  mem_values = list(pic_mem.values())

  print("Generate bytes memory file.")

  with open("byte_mem.txt", "w") as f:
    for item in mem_values:
      f.write('{0:02X} {1:02X} {2:02X} {3:02X} '.format(item[0], item[1], item[2], item[3]))

  crc_fun = ctypes.CDLL(get_file_path() + 'libcrc.so')
  
  crc_fun.calc_crc_file.restype = ctypes.c_uint

  crc = crc_fun.calc_crc_file()

  print("CRC is ", hex(crc))

  #os.remove("byte_mem.txt")

  return crc

def main():
  # Get hex
  try:
    hex_file = str(sys.argv[1])
  except:
    raise IOError('Hex file does not exist.')

  calc_crc_hex_file(hex_file)

if __name__ == '__main__':
  main()