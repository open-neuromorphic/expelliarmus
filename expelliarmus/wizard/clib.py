from ctypes import (
    CDLL,
    c_char_p,
    POINTER,
    c_size_t,
    c_int64,
    c_int16,
    c_uint16,
    c_uint8,
    c_uint,
    c_int,
    c_uint64,
    Structure,
)
import pathlib
import os
import re
from expelliarmus.utils import _ROOT_PATH
from numpy.ctypeslib import ndpointer
from numpy import zeros

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


class events_cargo_t(Structure):
    _fields_ = [
        ("dim", c_size_t),
        ("is_chunk", c_uint8),
        ("bytes_read", c_size_t),
        ("file_size", c_size_t),
    ]


class dat_cargo_t(Structure):
    _fields_ = [
        ("events_info", events_cargo_t),
        ("last_t", c_int64),
        ("time_ovfs", c_uint64),
    ]


class evt2_cargo_t(Structure):
    _fields_ = [
        ("events_info", events_cargo_t),
        ("last_t", c_int64),
        ("time_high", c_uint64),
    ]


class evt3_cargo_t(Structure):
    _fields_ = [
        ("events_info", events_cargo_t),
        ("time_high", c_uint64),
        ("time_low", c_uint64),
        ("time_high_ovfs", c_uint64),
        ("time_low_ovfs", c_uint64),
        ("base_x", c_uint16),
        ("last_event", event_t),
    ]


# Setting up C wrappers.
# Read functions.
c_read_dat = clib.read_dat
c_read_dat.argtypes = [
    c_char_p,
    ndpointer(dtype=event_t, ndim=1),
    POINTER(dat_cargo_t),
    c_size_t,
]
c_read_dat.restype = c_int

c_read_evt2 = clib.read_evt2
c_read_evt2.argtypes = [
    c_char_p,
    ndpointer(dtype=event_t, ndim=1),
    POINTER(evt2_cargo_t),
    c_size_t,
]
c_read_evt2.restype = c_int

c_read_evt3 = clib.read_evt3
c_read_evt3.argtypes = [
    c_char_p,
    ndpointer(dtype=event_t, ndim=1),
    POINTER(evt3_cargo_t),
    c_size_t,
]
c_read_evt3.restype = c_int

# Cut functions.
ARGTYPES_CUT = [c_char_p, c_char_p, c_size_t, c_size_t]
RESTYPE_CUT = c_size_t

c_cut_dat = clib.cut_dat
c_cut_evt2 = clib.cut_evt2
c_cut_evt3 = clib.cut_evt3
for c_cut_fn in (c_cut_dat, c_cut_evt2, c_cut_evt3):
    c_cut_fn.restype = RESTYPE_CUT
    c_cut_fn.argtypes = ARGTYPES_CUT

# Measure functions.
ARGTYPES_MEASURE = [c_char_p, c_size_t]
RESTYPE_MEASURE = c_size_t

c_measure_dat = clib.measure_dat
c_measure_evt2 = clib.measure_evt2
c_measure_evt3 = clib.measure_evt3
for fn in (c_measure_dat, c_measure_evt2, c_measure_evt3):
    fn.argtypes = ARGTYPES_MEASURE
    fn.restype = RESTYPE_MEASURE
