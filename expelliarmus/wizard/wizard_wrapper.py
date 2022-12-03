from pathlib import Path
from ctypes import c_char_p, c_size_t, byref
from typing import Optional, Union
from numpy import empty, ndarray
from expelliarmus.utils import _SUPPORTED_ENCODINGS
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
    c_save_evt2,
)


def c_read_wrapper(encoding: str, fpath: Union[str, Path], buff_size: int):
    # Error handling.
    if not isinstance(encoding, str):
        raise TypeError(
            "ERROR: The encoding must be a string among 'DAT', 'EVT2' and 'EVT3'."
        )
    if not (isinstance(fpath, str) or isinstance(fpath, Path)):
        raise TypeError(
            "ERROR: The file path must be a string or a pathlib.Path object."
        )
    if not isinstance(buff_size, int):
        raise TypeError("ERROR: The buffer size must be an integer larger than 0.")
    if encoding not in _SUPPORTED_ENCODINGS:
        raise ValueError("ERROR: The encoding provided is not valid.")
    if buff_size <= 0:
        raise ValueError("ERROR: The buffer size must be larger than 0.")

    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)

    events_info = events_cargo_t(0, 0, 0, 0, 0, 0)
    if encoding == "DAT":
        cargo = dat_cargo_t(events_info, 0, 0)
        c_measure_dat(c_fpath, byref(cargo), c_buff_size)
        print("I AM ALIVE.")
    elif encoding == "EVT2":
        cargo = evt2_cargo_t(events_info, 0, 0)
        dim = c_measure_evt2(c_fpath, byref(cargo), c_buff_size)
    elif encoding == "EVT3":
        last_event = event_t(0, 0, 0, 0)
        cargo = evt3_cargo_t(events_info, 0, 0, 0, 0, 0, last_event)
        c_measure_evt3(c_fpath, byref(cargo), c_buff_size)
    if cargo.events_info.dim > 0:
        arr = empty((cargo.events_info.dim,), dtype=event_t)
        if encoding == "DAT":
            status = c_read_dat(c_fpath, arr, byref(cargo), c_buff_size)
        elif encoding == "EVT2":
            status = c_read_evt2(c_fpath, arr, byref(cargo), c_buff_size)
        elif encoding == "EVT3":
            status = c_read_evt3(c_fpath, arr, byref(cargo), c_buff_size)
    return (arr, status) if cargo.events_info.dim > 0 and status == 0 else (None, status)


def c_save_wrapper(
    encoding: str, fpath: Union[str, Path], arr: ndarray, buff_size: int
):
    # Error handling.
    if not isinstance(encoding, str):
        raise TypeError(
            "ERROR: The encoding must be a string among 'DAT', 'EVT2' and 'EVT3'."
        )
    if encoding not in _SUPPORTED_ENCODINGS:
        raise ValueError("ERROR: The encoding provided is not valid.")
    if not (isinstance(fpath, str) or isinstance(fpath, Path)):
        raise TypeError(
            "ERROR: The file path must be a string or a pathlib.Path object."
        )
    if not (isinstance(arr, ndarray)):
        raise TypeError("ERROR: A NumPy array must be provided.")
    if not isinstance(buff_size, int):
        raise TypeError("ERROR: The buffer size must be an integer larger than 0.")
    if buff_size <= 0:
        raise ValueError("ERROR: The buffer size must be larger than 0.")

    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)
    dim = len(arr)
    if arr.dtype != event_t:
        loc_arr = empty((dim,), dtype=event_t)
        for k in ("t", "y", "x", "p"):
            loc_arr[k] = arr[k].astype(loc_arr[k].dtype)
    else:
        loc_arr = arr

    events_info = events_cargo_t()
    events_info.dim = dim
    events_info.start_byte = 0
    if encoding == "DAT":
        raise NotImplementedError("ERROR: The save method is not implemented for DAT encoding.")
        cargo = dat_cargo_t(events_info, 0, 0)
    elif encoding == "EVT2":
        cargo = evt2_cargo_t(events_info, 0, 0)
        status = c_save_evt2(c_fpath, loc_arr, byref(cargo), c_buff_size)
    elif encoding == "EVT3":
        raise NotImplementedError("ERROR: The save method is not implemented for EVT3 encoding.")
        last_event = event_t(0, 0, 0, 0)
        cargo = evt3_cargo_t(events_info, 0, 0, 0, 0, 0, last_event)
    return status


def c_read_chunk_wrapper(
    encoding: str,
    fpath: Union[str, Path],
    cargo: Union[dat_cargo_t, evt2_cargo_t, evt3_cargo_t],
    buff_size: int,
):
    if not (isinstance(fpath, str) or isinstance(fpath, Path)):
        raise TypeError(
            "ERROR: The file path must be a string or a pathlib.Path object."
        )
    if not isinstance(buff_size, int):
        raise TypeError("ERROR: The buffer size must be an integer larger than 0.")
    if encoding not in _SUPPORTED_ENCODINGS:
        raise ValueError("ERROR: The encoding provided is not valid.")
    if buff_size <= 0:
        raise ValueError("ERROR: The buffer size must be larger than 0.")
    if not (
        isinstance(cargo, dat_cargo_t)
        or isinstance(cargo, evt2_cargo_t)
        or isinstance(cargo, evt3_cargo_t)
    ):
        raise TypeError("ERROR: The cargo type is not supported.")

    c_fpath = c_char_p(bytes(str(fpath), "utf-8"))
    c_buff_size = c_size_t(buff_size)
    if encoding == "DAT":
        arr = empty((cargo.events_info.dim,), dtype=event_t)
        status = c_read_dat(c_fpath, arr, byref(cargo), c_buff_size)
    elif encoding == "EVT2":
        arr = empty((cargo.events_info.dim,), dtype=event_t)
        status = c_read_evt2(c_fpath, arr, byref(cargo), c_buff_size)
    elif encoding == "EVT3":
        arr = empty((cargo.events_info.dim + 12,), dtype=event_t)
        status = c_read_evt3(c_fpath, arr, byref(cargo), c_buff_size)
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
    if not (isinstance(fpath_in, str) or isinstance(fpath_in, Path)):
        raise TypeError(
            "ERROR: The input file path must be a string or a pathlib.Path object."
        )
    if not (isinstance(fpath_out, str) or isinstance(fpath_out, Path)):
        raise TypeError(
            "ERROR: The output file path must be a string or a pathlib.Path object."
        )
    if not isinstance(buff_size, int):
        raise TypeError("ERROR: The buffer size must be an integer larger than 0.")
    if buff_size <= 0:
        raise ValueError("ERROR: The buffer size must be larger than 0.")
    if encoding not in _SUPPORTED_ENCODINGS:
        raise ValueError("ERROR: The encoding provided is not valid.")
    if not isinstance(new_duration, int):
        raise TypeError("ERROR: The new duration must be a positive integer.")
    if new_duration <= 0:
        raise ValueError("ERROR: The new duration must be larger than zero.")

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
    return c_dim
