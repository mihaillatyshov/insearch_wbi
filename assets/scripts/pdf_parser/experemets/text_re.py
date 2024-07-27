import re

from custom_regex import full_regex

# txt = "The rain in Spain"
# x = re.search("^The.*Spain$", txt)

# strs = [
#     "M161600400-I2.0M",
#     "M161600400-I2.5M",
#     "M181710410-I2.5M",
#     "M161600420-I3.0M",
#     "M202000450-I3.0M",
# ]

# for string in strs:
#     res = re.search(full_regex["mill_M"], string)

#     if res is None:
#         print(res)
#     else:
#         print(res.group())

list_low = [[25, 13], [25, 11], [24, 11]]

for llv_d, llv_p in list_low:
    if llv_d == 24 and llv_p == 12:
        print("find!!!")
        break
