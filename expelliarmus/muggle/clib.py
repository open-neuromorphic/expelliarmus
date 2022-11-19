from ctypes import (
    CDLL,
    c_char_p,
    POINTER,
    c_size_t,
    c_uint64,
    c_int64,
    c_int16,
    c_uint8,
    Structure,
    c_uint16,
)
import pathlib
import os
import re
from expelliarmus.wizard.clib import event_array_t, event_t
from expelliarmus.utils import _ROOT_PATH
from typing import Union

# Searching for the shared library.
lib_re = r"^expelliarmus\..*\.(so|pyd)$"
for root, dirs, files in os.walk(_ROOT_PATH):
    for f in files:
        if re.match(lib_re, f):
            lib_path = pathlib.Path(os.path.join(root, f))
            break
clib = CDLL(str(lib_path))

# Setting up arguments and return types.
class dat_chunk_t(Structure):
    _fields_ = [
        ("arr", event_array_t),
        ("bytes_read", c_size_t),
        ("file_size", c_size_t),
    ]


class evt2_chunk_t(Structure):
    _fields_ = [
        ("arr", event_array_t),
        ("bytes_read", c_size_t),
        ("file_size", c_size_t),
        ("time_high", c_uint64),
    ]


class evt3_chunk_t(Structure):
    _fields_ = [
        ("arr", event_array_t),
        ("bytes_read", c_size_t),
        ("file_size", c_size_t),
        ("base_x", c_uint16),
        ("time_high", c_uint64),
        ("time_low", c_uint64),
        ("time_high_ovfs", c_uint64),
        ("time_low_ovfs", c_uint64),
        ("event_tmp", event_t),
    ]


def set_file_size(
    chunk: Union[dat_chunk_t, evt2_chunk_t, evt3_chunk_t],
    file_size: int,
):
    chunk.file_size = c_size_t(file_size)
    return chunk


def set_bytes_read(
    chunk: Union[dat_chunk_t, evt2_chunk_t, evt3_chunk_t],
    bytes_read: int,
):
    chunk.bytes_read = c_size_t(bytes_read)
    return chunk


c_read_dat_chunk = clib.read_dat_chunk
c_read_evt2_chunk = clib.read_evt2_chunk
c_read_evt3_chunk = clib.read_evt3_chunk
for c_chunk_fn, chunk_t in (
    (c_read_dat_chunk, dat_chunk_t),
    (c_read_evt2_chunk, evt2_chunk_t),
    (c_read_evt3_chunk, evt3_chunk_t),
):
    c_chunk_fn.argtypes = [c_char_p, c_size_t, POINTER(chunk_t), c_size_t]
    c_chunk_fn.restype = None
