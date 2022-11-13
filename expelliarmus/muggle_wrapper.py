from ctypes import c_char_p, c_size_t
import pathlib
import numpy as np
from typing import Union, Optional, Callable
from .expelliarmus_wrapper import DTYPE
from .clib_muggle import (
        c_read_dat_chunk, 
        )

def c_chunk_wrapper(p_fun, fpath, buff_size, bytes_read, nevents_per_chunk, dtype):
    assert isinstance(fpath, str) or isinstance(fpath, pathlib.Path)
    fpath = pathlib.Path(fpath).resolve()
    assert fpath.is_file(), f'Error: the file provided "{str(fpath)}" does not exist.'
    assert isinstance(buff_size, int) and buff_size > 0, "Error: a minimum buffer size of 1 is required."
    assert isinstance(bytes_read, int) and bytes_read>=0, "Error: a non-negative number of bytes read has to be provided."
    assert isinstance(nevents_per_chunk, int) and nevents_per_chunk>0, "Error: the number of events per chunk has to be larger than 0."

    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)
    c_bytes_read = c_size_t(bytes_read)
    c_nevents_per_chunk = c_size_t(nevents_per_chunk)
    if p_fun == read_dat_chunk:
        ret = c_read_dat_chunk(c_fpath, c_buff_size, c_bytes_read, c_nevents_per_chunk)
    # elif p_fun == read_evt2_chunk:
        # c_arr = c_read_evt2_chunk(c_fpath, byref(c_dim), c_buff_size, byref(c_bytes_read), c_nevents_per_chunk)
    # elif p_fun == read_evt3_chunk:
        # c_arr = c_read_evt3_chunk(c_fpath, byref(c_dim), c_buff_size, byref(c_bytes_read), c_nevents_per_chunk)
    else:
        raise "Function not defined."
    bytes_read = ret.bytes_read
    print(bytes_read)
    if bytes_read > 0:
        np_arr = np.empty((ret.arr.dim,), dtype=dtype)
        np_arr["t"] = np.ctypeslib.as_array(ret.arr.t_arr, shape=(ret.arr.dim,))
        np_arr["x"] = np.ctypeslib.as_array(ret.arr.x_arr, shape=(ret.arr.dim,))
        np_arr["y"] = np.ctypeslib.as_array(ret.arr.y_arr, shape=(ret.arr.dim,))
        np_arr["p"] = np.ctypeslib.as_array(ret.arr.p_arr, shape=(ret.arr.dim,))
    else:
        np_arr = None
    return bytes_read, np_arr

def read_dat_chunk(
        fpath: Union[str, pathlib.Path], 
        bytes_read: int, 
        nevents_per_chunk: int, 
        buff_size: Optional[int]=4096, 
        dtype: Optional[np.dtype] = DTYPE
        ):
    assert str(fpath).endswith(".dat"), f"Error: the file provided \"{str(fpath)}\" is not valid."
    return c_chunk_wrapper(p_fun=read_dat_chunk, fpath=fpath, buff_size=buff_size, bytes_read=bytes_read, nevents_per_chunk=nevents_per_chunk, dtype=dtype)

# def read_evt2_chunk(
#         fpath: Union[str, pathlib.Path], 
#         bytes_read: int, 
#         nevents_per_chunk: int, 
#         buff_size: Optional[int]=4096, 
#         dtype: Optional[np.dtype] = DTYPE
#         ):
#     assert str(fpath).endswith(".raw"), f"Error: the file provided \"{str(fpath)}\" is not valid."
#     return c_chunk_wrapper(p_fun=read_evt2_chunk, fpath=fpath, buff_size=buff_size, bytes_read=bytes_read, nevents_per_chunk=nevents_per_chunk, dtype=dtype)
# 
# def read_evt3_chunk(
#         fpath: Union[str, pathlib.Path], 
#         bytes_read: int, 
#         nevents_per_chunk: int, 
#         buff_size: Optional[int]=4096, 
#         dtype: Optional[np.dtype] = DTYPE
#         ):
#     assert str(fpath).endswith(".raw"), f"Error: the file provided \"{str(fpath)}\" is not valid."
#     return c_chunk_wrapper(p_fun=read_evt3_chunk, fpath=fpath, buff_size=buff_size, bytes_read=bytes_read, nevents_per_chunk=nevents_per_chunk, dtype=dtype)
