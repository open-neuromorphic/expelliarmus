from pathlib import Path
from ctypes import c_char_p, c_size_t, byref
from typing import Optional, Union
from numpy import zeros
from expelliarmus.wizard.clib import (
    event_t,
    events_cargo_t,
    dat_cargo_t,
    evt2_cargo_t,
    evt3_cargo_t,
    c_read_dat,
    c_read_evt2,
    c_read_evt3,
    c_measure_dat,
    c_measure_evt2,
    c_measure_evt3,
    c_cut_dat,
    c_cut_evt2,
    c_cut_evt3,
)


def c_read_wrapper(encoding: str, fpath: Union[str, Path], buff_size: int):
    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)
    if encoding == "DAT":
        dim = c_measure_dat(c_fpath, c_buff_size)
        events_info = events_cargo_t(dim, 0, 0, 0)
        cargo = dat_cargo_t(events_info, 0, 0)
    elif encoding == "EVT2":
        dim = c_measure_evt2(c_fpath, c_buff_size)
        events_info = events_cargo_t(dim, 0, 0, 0)
        cargo = evt2_cargo_t(events_info, 0, 0)
    elif encoding == "EVT3":
        dim = c_measure_evt3(c_fpath, c_buff_size)
        events_info = events_cargo_t(dim, 0, 0, 0)
        last_event = event_t(0, 0, 0, 0)
        cargo = evt3_cargo_t(events_info, 0, 0, 0, 0, 0, last_event)
    else:
        raise Exception("Encoding not valid.")
    if dim > 0:
        arr = zeros((dim,), dtype=event_t)
        if encoding == "DAT":
            status = c_read_dat(c_fpath, arr, byref(cargo), c_buff_size)
        elif encoding == "EVT2":
            status = c_read_evt2(c_fpath, arr, byref(cargo), c_buff_size)
        elif encoding == "EVT3":
            status = c_read_evt3(c_fpath, arr, byref(cargo), c_buff_size)
        else:
            raise Exception("Function not defined.")
    return (arr, status) if dim > 0 and status == 0 else (None, status)


def c_read_chunk_wrapper(
    encoding: str,
    fpath: Union[str, Path],
    cargo: Union[dat_cargo_t, evt2_cargo_t, evt3_cargo_t],
    buff_size: int,
):
    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)
    if encoding == "DAT":
        arr = zeros((cargo.events_info.dim,), dtype=event_t)
        status = c_read_dat(c_fpath, arr, byref(cargo), c_buff_size)
    elif encoding == "EVT2":
        arr = zeros((cargo.events_info.dim,), dtype=event_t)
        status = c_read_evt2(c_fpath, arr, byref(cargo), c_buff_size)
    elif encoding == "EVT3":
        arr = zeros((cargo.events_info.dim + 12,), dtype=event_t)
        status = c_read_evt3(c_fpath, arr, byref(cargo), c_buff_size)
    else:
        raise Exception("Function not defined.")
    return (
        (arr[: cargo.events_info.dim], cargo, status)
        if status == 0
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
    if encoding == "DAT":
        c_dim = c_cut_dat(c_fpath_in, c_fpath_out, c_new_duration, c_buff_size)
    elif encoding == "EVT2":
        c_dim = c_cut_evt2(c_fpath_in, c_fpath_out, c_new_duration, c_buff_size)
    elif encoding == "EVT3":
        c_dim = c_cut_evt3(c_fpath_in, c_fpath_out, c_new_duration, c_buff_size)
    else:
        raise Exception("Encoding not valid.")
    return c_dim
