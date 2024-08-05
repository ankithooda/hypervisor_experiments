# Converts Unifont's HEX format to a C-Style array

import sys

# 16 * 32 pixels
# 2 Bytes * 32
# Each char requires 64 Bytes
# Total 256 Chars

# First 32 lines are 0s
for c in range(32):          # First 32 Characters
    for i in range(32):      # 32 Rows for each line
        print("0x0000", end=', ')
    print('')


# with open(sys.argv[1]) as file:
#     lines = [line.rstrip() for line in file]

# for line in lines:
#     code = line[0:5]
#     bitmap = line[5:]

#     i = 0
#     while (i < len(bitmap)):
#         print("0x{}".format(bitmap[i:i+4]), end=', ')
#         i = i + 4
#     print('')

#     if (line[0:4] == "007F"):
#         break

