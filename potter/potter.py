from ctypes import *
import numpy as np
from numpy.lib.recfunctions import unstructured_to_structured
import re
import os
import pathlib

# Searching for the shared library. 
this_file_path = pathlib.Path(__file__).parent.parent.resolve()
lib_re = r"^potter\..*\.(so|pyd)$"
for root, dirs, files in os.walk(this_file_path):
    for f in files:
        if re.match(lib_re, f):
            lib_path = os.path.join(root, f)
            break

libpotter = cdll.LoadLibrary(lib_path)
ARGTYPES = [c_char_p, POINTER(c_size_t), c_size_t]
RESTYPE = POINTER(c_ulonglong)

c_read_dat = libpotter.read_dat
c_read_dat.restype = RESTYPE 
c_read_dat.argtypes = ARGTYPES

c_read_evt2 = libpotter.read_evt2
c_read_evt2.restype = RESTYPE
c_read_evt2.argtypes = ARGTYPES

c_read_evt3 = libpotter.read_evt3
c_read_evt3.restype = RESTYPE
c_read_evt3.argtypes = ARGTYPES

DTYPE = np.dtype([('t', np.int64), ('x', np.int32), ('y', np.int32), ('p', np.int8)])

def c_wrapper(p_fun, fpath, buff_size, dtype):
    c_fpath = c_char_p(bytes(fpath, 'utf-8'))
    c_dim = c_size_t(0)
    c_buff_size = c_size_t(buff_size)
    if p_fun == read_dat:
        c_arr = c_read_dat(c_fpath, byref(c_dim), c_buff_size)
    elif p_fun == read_evt2:
        c_arr = c_read_evt2(c_fpath, byref(c_dim), c_buff_size)
    elif p_fun == read_evt3:
        c_arr = c_read_evt3(c_fpath, byref(c_dim), c_buff_size)
    else:
        raise "Function not defined."
    np_arr = np.ctypeslib.as_array(c_arr, shape=(c_dim.value, )).reshape((c_dim.value//4, 4))
    return unstructured_to_structured(np_arr, dtype=dtype)   


def read_dat(fpath, buff_size=4096, dtype=DTYPE):
    return c_wrapper(read_dat, fpath, buff_size, dtype)

def read_evt2(fpath, buff_size=4096, dtype=DTYPE):
    return c_wrapper(read_evt2, fpath, buff_size, dtype)

def read_evt3(fpath, buff_size=4096, dtype=DTYPE):
    return c_wrapper(read_evt3, fpath, buff_size, dtype)

if __name__ == "__main__":
    print("Testing DAT.")
    arr = read_dat("./spinner.dat")
    print(arr[0], arr[-1], arr.shape)
    print("Testing EVT2.")
    arr = read_evt2("./spinner.raw")
    print(arr[0], arr[-1], arr.shape)
    print("Testing EVT3.")
    arr = read_evt3("./pedestrians.raw")
    print(arr[0], arr[-1], arr.shape)
