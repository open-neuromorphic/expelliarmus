from ctypes import c_char_p, c_size_t, byref
import pathlib
import numpy as np
from typing import Union, Optional, Callable
from expelliarmus.muggle.clib import (
    c_read_dat_chunk,
    c_read_evt2_chunk,
    c_read_evt3_chunk,
    dat_chunk_wrap_t,
    evt2_chunk_wrap_t,
    evt3_chunk_wrap_t,
)


def c_chunk_wrapper(p_fn, fpath, buff_size, chunk_wrap, nevents_per_chunk, dtype):
    assert isinstance(fpath, str) or isinstance(fpath, pathlib.Path)
    fpath = pathlib.Path(fpath).resolve()
    assert fpath.is_file(), f'Error: the file provided "{str(fpath)}" does not exist.'
    assert (
        isinstance(buff_size, int) and buff_size > 0
    ), "Error: a minimum buffer size of 1 is required."
    assert (
        isinstance(nevents_per_chunk, int) and nevents_per_chunk > 0
    ), "Error: the number of events per chunk has to be larger than 0."

    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)
    c_nevents_per_chunk = c_size_t(nevents_per_chunk)
    if p_fn == read_dat_chunk:
        c_fn = c_read_dat_chunk
    elif p_fn == read_evt2_chunk:
        c_fn = c_read_evt2_chunk
    elif p_fn == read_evt3_chunk:
        c_fn = c_read_evt3_chunk
    else:
        raise "Function not defined."
    c_fn(c_fpath, c_buff_size, byref(chunk_wrap), c_nevents_per_chunk)
    if chunk_wrap.bytes_read > 0:
        np_arr = np.empty((chunk_wrap.arr.dim,), dtype=dtype)
        np_arr["t"] = np.ctypeslib.as_array(
            chunk_wrap.arr.t_arr, shape=(chunk_wrap.arr.dim,)
        )
        np_arr["x"] = np.ctypeslib.as_array(
            chunk_wrap.arr.x_arr, shape=(chunk_wrap.arr.dim,)
        )
        np_arr["y"] = np.ctypeslib.as_array(
            chunk_wrap.arr.y_arr, shape=(chunk_wrap.arr.dim,)
        )
        np_arr["p"] = np.ctypeslib.as_array(
            chunk_wrap.arr.p_arr, shape=(chunk_wrap.arr.dim,)
        )
    else:
        np_arr = None
    return chunk_wrap, np_arr


def read_dat_chunk(
    fpath: Union[str, pathlib.Path],
    nevents_per_chunk: int,
    chunk_wrap: Union[dat_chunk_wrap_t, evt2_chunk_wrap_t, evt3_chunk_wrap_t],
    buff_size: int,
    dtype: np.dtype,
):
    assert str(fpath).endswith(
        ".dat"
    ), f'Error: the file provided "{str(fpath)}" is not valid.'
    return c_chunk_wrapper(
        p_fn=read_dat_chunk,
        fpath=fpath,
        buff_size=buff_size,
        chunk_wrap=chunk_wrap,
        nevents_per_chunk=nevents_per_chunk,
        dtype=dtype,
    )


def read_evt2_chunk(
    fpath: Union[str, pathlib.Path],
    nevents_per_chunk: int,
    chunk_wrap: Union[dat_chunk_wrap_t, evt2_chunk_wrap_t, evt3_chunk_wrap_t],
    buff_size: int,
    dtype: np.dtype,
):
    assert str(fpath).endswith(
        ".raw"
    ), f'Error: the file provided "{str(fpath)}" is not valid.'
    return c_chunk_wrapper(
        p_fn=read_evt2_chunk,
        fpath=fpath,
        buff_size=buff_size,
        chunk_wrap=chunk_wrap,
        nevents_per_chunk=nevents_per_chunk,
        dtype=dtype,
    )


def read_evt3_chunk(
    fpath: Union[str, pathlib.Path],
    nevents_per_chunk: int,
    chunk_wrap: Union[dat_chunk_wrap_t, evt2_chunk_wrap_t, evt3_chunk_wrap_t],
    buff_size: int,
    dtype: np.dtype,
):
    assert str(fpath).endswith(
        ".raw"
    ), f'Error: the file provided "{str(fpath)}" is not valid.'
    return c_chunk_wrapper(
        p_fn=read_evt3_chunk,
        fpath=fpath,
        buff_size=buff_size,
        chunk_wrap=chunk_wrap,
        nevents_per_chunk=nevents_per_chunk,
        dtype=dtype,
    )
