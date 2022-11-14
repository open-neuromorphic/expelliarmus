from ctypes import (
    CDLL,
    c_char_p,
    POINTER,
    c_size_t,
    c_int64,
    c_int16,
    c_uint8,
    Structure,
)
import pathlib
import os
import re
from expelliarmus.utils import _ROOT_PATH

# Searching for the shared library.
lib_re = r"^expelliarmus\..*\.(so|pyd)$"
for root, dirs, files in os.walk(_ROOT_PATH):
    for f in files:
        if re.match(lib_re, f):
            lib_path = pathlib.Path(os.path.join(root, f))
            break
clib = CDLL(str(lib_path))


class event_t(Structure):
    _fields_ = [
        ("t", c_int64),
        ("x", c_int16),
        ("y", c_int16),
        ("p", c_uint8),
    ]


class event_array_t(Structure):
    _fields_ = [
        ("t_arr", POINTER(c_int64)),
        ("x_arr", POINTER(c_int16)),
        ("y_arr", POINTER(c_int16)),
        ("p_arr", POINTER(c_uint8)),
        ("dim", c_size_t),
        ("allocated_space", c_size_t),
    ]


# Setting up C wrappers.
ARGTYPES_READ = [c_char_p, c_size_t]
RESTYPE_READ = event_array_t

c_read_dat = clib.read_dat
c_read_evt2 = clib.read_evt2
c_read_evt3 = clib.read_evt3
for c_read_fn in (c_read_dat, c_read_evt2, c_read_evt3):
    c_read_fn.restype = RESTYPE_READ
    c_read_fn.argtypes = ARGTYPES_READ

ARGTYPES_CUT = [c_char_p, c_char_p, c_size_t, c_size_t]
RESTYPE_CUT = c_size_t

c_cut_dat = clib.cut_dat
c_cut_evt2 = clib.cut_evt2
c_cut_evt3 = clib.cut_evt3
for c_cut_fn in (c_cut_dat, c_cut_evt2, c_cut_evt3):
    c_cut_fn.restype = RESTYPE_CUT
    c_cut_fn.argtypes = ARGTYPES_CUT
