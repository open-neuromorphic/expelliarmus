from ctypes import CDLL, c_char_p, POINTER, c_size_t, c_int64, c_int16, c_uint8, Structure
import pathlib
import os
import re
from .clib_expelliarmus import event_array_t

# Searching for the shared library.
this_file_path = pathlib.Path(__file__).resolve().parent.parent
lib_re = r"^expelliarmus\..*\.(so|pyd)$"
for root, dirs, files in os.walk(this_file_path):
    for f in files:
        if re.match(lib_re, f):
            lib_path = pathlib.Path(os.path.join(root, f))
            break
clib = CDLL(str(lib_path))

# Setting up arguments and return types.
ARGTYPES_CHUNK = [c_char_p, c_size_t, c_size_t, c_size_t]


class dat_chunk_wrap_t(Structure):
    _fields_ = [
            ("arr", event_array_t), 
            ("bytes_read", c_size_t),
            ]

RESTYPE_CHUNK = dat_chunk_wrap_t

c_read_dat_chunk = clib.read_dat_chunk
# c_read_evt2_chunk = clib.read_evt2_chunk
# c_read_evt3_chunk = clib.read_evt3_chunk
for c_chunk_fn in (c_read_dat_chunk, ): #, c_read_evt2_chunk, c_read_evt3_chunk):
    c_chunk_fn.argtypes = ARGTYPES_CHUNK
    c_chunk_fn.restype = RESTYPE_CHUNK


