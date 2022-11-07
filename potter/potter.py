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
if lib_path.endswith(".so"):
    libpotter = CDLL(lib_path)
else:
    libpotter = WinDLL(lib_path)

ARGTYPES_READ = [c_char_p, POINTER(c_size_t), c_size_t]
RESTYPE_READ = POINTER(c_ulonglong)

ARGTYPES_CUT = [c_char_p, c_char_p, c_size_t, c_size_t]
RESTYPE_CUT = c_size_t

c_read_dat = libpotter.read_dat
c_read_dat.restype = RESTYPE_READ 
c_read_dat.argtypes = ARGTYPES_READ

c_read_evt2 = libpotter.read_evt2
c_read_evt2.restype = RESTYPE_READ
c_read_evt2.argtypes = ARGTYPES_READ

c_read_evt3 = libpotter.read_evt3
c_read_evt3.restype = RESTYPE_READ
c_read_evt3.argtypes = ARGTYPES_READ

c_cut_dat = libpotter.cut_dat
c_cut_dat.restype = RESTYPE_CUT 
c_cut_dat.argtypes = ARGTYPES_CUT

c_cut_evt2 = libpotter.cut_evt2
c_cut_evt2.restype = RESTYPE_CUT
c_cut_evt2.argtypes = ARGTYPES_CUT

c_cut_evt3 = libpotter.cut_evt3
c_cut_evt3.restype = RESTYPE_CUT
c_cut_evt3.argtypes = ARGTYPES_CUT

DTYPE = np.dtype([('t', np.int64), ('x', np.int16), ('y', np.int16), ('p', np.uint8)])

def c_read_wrapper(p_fun, fpath, buff_size, dtype):
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
    np_arr = unstructured_to_structured(np_arr, dtype=dtype)   
    np_arr["t"] -= np_arr["t"][0] # Eliminating the bias of the first sample.
    return np_arr


def read_dat(fpath, buff_size=4096, dtype=DTYPE):
    return c_read_wrapper(read_dat, fpath, buff_size, dtype)

def read_evt2(fpath, buff_size=4096, dtype=DTYPE):
    return c_read_wrapper(read_evt2, fpath, buff_size, dtype)

def read_evt3(fpath, buff_size=4096, dtype=DTYPE):
    return c_read_wrapper(read_evt3, fpath, buff_size, dtype)

def c_cut_wrapper(p_fun, fpath_in, fpath_out, max_nevents, buff_size):
    c_fpath_in = c_char_p(bytes(fpath_in, 'utf-8'))
    c_fpath_out = c_char_p(bytes(fpath_out, 'utf-8'))
    c_max_nevents = c_size_t(max_nevents)
    c_buff_size = c_size_t(buff_size)
    if p_fun == cut_dat:
        c_arr = c_cut_dat(c_fpath_in, c_fpath_out, c_max_nevents, c_buff_size)
    elif p_fun == cut_evt2:
        c_arr = c_cut_evt2(c_fpath_in, c_fpath_out, c_max_nevents, c_buff_size)
    elif p_fun == cut_evt3:
        c_arr = c_cut_evt3(c_fpath_in, c_fpath_out, c_max_nevents, c_buff_size)
    else:
        raise "Function not defined."
    return 


def cut_dat(fpath_in, fpath_out, max_nevents=1000, buff_size=4096):
    return c_cut_wrapper(cut_dat, fpath_in, fpath_out, max_nevents, buff_size)

def cut_evt2(fpath_in, fpath_out, max_nevents=1000, buff_size=4096):
    return c_cut_wrapper(cut_evt2, fpath_in, fpath_out, max_nevents, buff_size)

def cut_evt3(fpath_in, fpath_out, max_nevents=1000, buff_size=4096):
    return c_cut_wrapper(cut_evt3, fpath_in, fpath_out, max_nevents, buff_size)

