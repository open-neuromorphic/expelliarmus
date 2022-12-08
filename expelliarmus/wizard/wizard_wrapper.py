from ctypes import byref, c_char_p, c_size_t
from pathlib import Path
from typing import Optional, Union

from numpy import empty, ndarray

from expelliarmus.utils import _SUPPORTED_ENCODINGS
from expelliarmus.wizard.clib import (
    c_cargos_t,
    c_cut_fns,
    c_get_time_window_fns,
    c_measure_fns,
    c_read_fns,
    c_save_fns,
    dat_cargo_t,
    event_t,
    events_cargo_t,
    evt2_cargo_t,
    evt3_cargo_t,
)


def c_read_wrapper(encoding: str, fpath: Union[str, Path], buff_size: int):
    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)
    cargo = c_cargos_t[encoding](events_info=events_cargo_t())
    c_measure_fns[encoding](c_fpath, byref(cargo), c_buff_size)
    status = 0
    if cargo.events_info.dim > 0:
        arr = empty((cargo.events_info.dim,), dtype=event_t)
        status = c_read_fns[encoding](c_fpath, arr, byref(cargo), c_buff_size)
    return (
        (arr, status) if cargo.events_info.dim > 0 and status == 0 else (None, status)
    )


def c_save_wrapper(
    encoding: str,
    fpath: Union[str, Path],
    arr: ndarray,
    buff_size: int,
):
    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)
    dim = len(arr)
    if arr.dtype != event_t:
        loc_arr = empty((dim,), dtype=event_t)
        for k in ("t", "y", "x", "p"):
            loc_arr[k] = arr[k].astype(loc_arr[k].dtype)
    else:
        loc_arr = arr
    cargo = c_cargos_t[encoding](events_info=events_cargo_t(dim=dim, start_byte=0))
    return c_save_fns[encoding](c_fpath, loc_arr, byref(cargo), c_buff_size)


def c_read_time_window_wrapper(
    encoding: str,
    fpath: Union[str, Path],
    cargo: Union[dat_cargo_t, evt2_cargo_t, evt3_cargo_t],
    buff_size: int,
):
    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)

    c_get_time_window_fns[encoding](c_fpath, byref(cargo), c_buff_size)
    status = 0
    if cargo.events_info.dim > 0:
        arr = empty((cargo.events_info.dim,), dtype=event_t)
        status = c_read_fns[encoding](c_fpath, arr, byref(cargo), c_buff_size)
    return (
        (arr, cargo, status)
        if cargo.events_info.dim > 0 and status == 0
        else (None, cargo, status)
    )


def c_read_chunk_wrapper(
    encoding: str,
    fpath: Union[str, Path],
    cargo: Union[dat_cargo_t, evt2_cargo_t, evt3_cargo_t],
    buff_size: int,
):
    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)
    arr = empty(
        (cargo.events_info.dim + (12 if encoding == "evt3" else 0),), dtype=event_t
    )
    status = c_read_fns[encoding](c_fpath, arr, byref(cargo), c_buff_size)
    return (
        (arr[: cargo.events_info.dim], cargo, status)
        if cargo.events_info.dim > 0 and status == 0
        else (None, cargo, status)
    )


def c_cut_wrapper(
    encoding: str,
    fpath_in: Union[str, Path],
    fpath_out: Union[str, Path],
    new_duration: int,
    buff_size: int,
):
    c_fpath_in = c_char_p(bytes(str(fpath_in), "utf-8"))
    c_fpath_out = c_char_p(bytes(str(fpath_out), "utf-8"))
    c_new_duration = c_size_t(new_duration)
    c_buff_size = c_size_t(buff_size)
    return c_cut_fns[encoding](c_fpath_in, c_fpath_out, c_new_duration, c_buff_size)
