import os
import pathlib
from ctypes import c_char_p, c_size_t
from typing import Optional, Union
import numpy as np
from .clib_expelliarmus import (
        c_read_dat, 
        c_read_evt2, 
        c_read_evt3, 
        c_cut_dat, 
        c_cut_evt2, 
        c_cut_evt3,
        )


# Default data type for structured array.
DTYPE = np.dtype([("t", np.int64), ("x", np.int16), ("y", np.int16), ("p", np.uint8)])

def c_read_wrapper(p_fun, fpath, buff_size, dtype):
    assert isinstance(fpath, str) or isinstance(fpath, pathlib.Path)
    fpath = pathlib.Path(fpath).resolve()
    assert fpath.is_file(), f'Error: the file provided "{str(fpath)}" does not exist.'
    assert isinstance(buff_size, int) and buff_size > 0, "Error: a minimum buffer size of 1 is required."

    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)
    if p_fun == read_dat:
        c_arr = c_read_dat(c_fpath, c_buff_size)
    elif p_fun == read_evt2:
        c_arr = c_read_evt2(c_fpath, c_buff_size)
    elif p_fun == read_evt3:
        c_arr = c_read_evt3(c_fpath, c_buff_size)
    else:
        raise "Function not defined."
    np_arr = np.empty((c_arr.dim,), dtype=dtype)
    np_arr["t"] = np.ctypeslib.as_array(c_arr.t_arr, shape=(c_arr.dim,))
    np_arr["x"] = np.ctypeslib.as_array(c_arr.x_arr, shape=(c_arr.dim,))
    np_arr["y"] = np.ctypeslib.as_array(c_arr.y_arr, shape=(c_arr.dim,))
    np_arr["p"] = np.ctypeslib.as_array(c_arr.p_arr, shape=(c_arr.dim,))
    return np_arr


def c_cut_wrapper(p_fun, fpath_in, fpath_out, new_duration, buff_size):
    assert isinstance(fpath_in, str) or isinstance(fpath_in, pathlib.Path)
    assert isinstance(fpath_out, str) or isinstance(fpath_out, pathlib.Path)
    fpath_in = pathlib.Path(fpath_in).resolve()
    fpath_out = pathlib.Path(fpath_out).resolve()
    assert (
        fpath_in.is_file()
    ), f'Error: the input file provided "{str(fpath_in)}" does not exist.'
    assert (
        fpath_out.parent.is_dir()
    ), f'Error: the output file path provided "{str(fpath_out)}" does not exist.'
    assert isinstance(buff_size, int) and buff_size > 0, "Error: a minimum buffer size of 1 is required."
    assert isinstance(new_duration, int) and new_duration > 0, "Error: the new time duration of the recording must be larger than or equal to 1ms."

    c_fpath_in = c_char_p(bytes(str(fpath_in), "utf-8"))
    c_fpath_out = c_char_p(bytes(str(fpath_out), "utf-8"))
    c_new_duration = c_size_t(new_duration)
    c_buff_size = c_size_t(buff_size)
    if p_fun == cut_dat:
        c_dim = c_cut_dat(c_fpath_in, c_fpath_out, c_new_duration, c_buff_size)
    elif p_fun == cut_evt2:
        c_dim = c_cut_evt2(c_fpath_in, c_fpath_out, c_new_duration, c_buff_size)
    elif p_fun == cut_evt3:
        c_dim = c_cut_evt3(c_fpath_in, c_fpath_out, c_new_duration, c_buff_size)
    else:
        raise "Function not defined."
    return c_dim


# Actual stuff you should care about.

def read_dat(
    fpath: Union[pathlib.Path, str],
    buff_size: Optional[int] = 4096,
    dtype: Optional[np.dtype] = DTYPE,
) -> np.ndarray:
    """
    Function that reads a DAT binary file to a structured NumPy array.
    Args:
        - fpath: path to the DAT file.
        - buff_size: size of the buffer used to read the binary file.
        - dtype: the types for the structured array.
    Returns:
        - arr: a structured NumPy array that encodes (timestamp, x_address, y_address, polarity).
    """
    assert str(fpath).endswith(".dat"), f"Error: the file provided \"{str(fpath)}\" is not a DAT file."
    return c_read_wrapper(read_dat, fpath, buff_size, dtype)


def read_evt2(
    fpath: Union[pathlib.Path, str],
    buff_size: Optional[int] = 4096,
    dtype: Optional[np.dtype] = DTYPE,
) -> np.ndarray:
    """
    Function that reads a EVT2 binary file to a structured NumPy array.
    Args:
        - fpath: path to the EVT2 file.
        - buff_size: size of the buffer used to read the binary file.
        - dtype: the types for the structured array.
    Returns:
        - arr: a structured NumPy array that encodes (timestamp, x_address, y_address, polarity).
    """
    assert str(fpath).endswith(".raw"), "Error: the file provided \"{str(fpath)}\" is not a RAW file."
    return c_read_wrapper(read_evt2, fpath, buff_size, dtype)


def read_evt3(
    fpath: Union[pathlib.Path, str],
    buff_size: Optional[int] = 4096,
    dtype: Optional[np.dtype] = DTYPE,
) -> np.ndarray:
    """
    Function that reads a EVT3 binary file to a structured NumPy array.
    Args:
        - fpath: path to the DAT file.
        - buff_size: size of the buffer used to read the binary file.
        - dtype: the types for the structured array.
    Returns:
        - arr: a structured NumPy array that encodes (timestamp, x_address, y_address, polarity).
    """
    assert str(fpath).endswith(".raw"), "Error: the file provided \"{str(fpath)}\" is not a RAW file."
    return c_read_wrapper(read_evt3, fpath, buff_size, dtype)


def cut_dat(
    fpath_in: Union[pathlib.Path, str],
    fpath_out: Union[pathlib.Path, str],
    new_duration: Optional[int] = 10,
    buff_size: Optional[int] = 4096,
) -> int:
    """
    Function that reads a DAT binary file and cuts it to a limited number of events.
    Args:
        - fpath_in: path to the input DAT file.
        - fpath_out: path to the output DAT file.
        - new_duration: new time duration of the recording expressed in milliseconds.
        - buff_size: size of the buffer used to read the binary file.
    Returns:
        - dim: the number of events encoded in the output file.
    """
    assert str(fpath_in).endswith(".dat"), f"Error: the input file provided \"{str(fpath_in)}\" is not a DAT file."
    assert str(fpath_out).endswith(".dat"), f"Error: the output file provided \"{str(fpath_out)}\" is not a DAT file."
    return c_cut_wrapper(cut_dat, fpath_in, fpath_out, new_duration, buff_size)


def cut_evt2(
    fpath_in: Union[pathlib.Path, str],
    fpath_out: Union[pathlib.Path, str],
    new_duration: Optional[int] = 10,
    buff_size: Optional[int] = 4096,
) -> int:
    """
    Function that reads a EVT2 binary file and cuts it to a limited number of events.
    Args:
        - fpath_in: path to the input EVT2 file.
        - fpath_out: path to the output EVT2 file.
        - new_duration: new time duration of the recording expressed in milliseconds.
        - max_nevents: number of events to be written in the output file.
        - buff_size: size of the buffer used to read the binary file.
    Returns:
        - dim: the number of events encoded in the output file.
    """
    assert str(fpath_in).endswith(".raw"), f"Error: the input file provided \"{str(fpath_in)}\" is not a RAW file."
    assert str(fpath_out).endswith(".raw"), f"Error: the output file provided \"{str(fpath_out)}\" is not a RAW file."
    return c_cut_wrapper(cut_evt2, fpath_in, fpath_out, new_duration, buff_size)


def cut_evt3(
    fpath_in: Union[pathlib.Path, str],
    fpath_out: Union[pathlib.Path, str],
    new_duration: Optional[int] = 10,
    buff_size: Optional[int] = 4096,
) -> int:
    """
    Function that reads a EVT3 binary file and cuts it to a limited number of events.
    Args:
        - fpath_in: path to the input EVT3 file.
        - fpath_out: path to the output EVT3 file.
        - new_duration: new time duration of the recording expressed in milliseconds.
        - buff_size: size of the buffer used to read the binary file.
    Returns:
        - dim: the number of events encoded in the output file.
    """
    assert str(fpath_in).endswith(".raw"), f"Error: the input file provided \"{str(fpath_in)}\" is not a RAW file."
    assert str(fpath_out).endswith(".raw"), f"Error: the output file provided \"{str(fpath_out)}\" is not a RAW file."
    return c_cut_wrapper(cut_evt3, fpath_in, fpath_out, new_duration, buff_size)

