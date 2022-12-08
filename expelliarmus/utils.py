from pathlib import Path
import numpy as np
from typing import Union
from ctypes import (
    c_int64,
    c_int16,
    c_uint8,
)

_ROOT_PATH = Path(__file__).resolve().parent.parent

_SUPPORTED_ENCODINGS = (
    "dat",
    "evt2",
    "evt3",
)

_DTYPES = {
    "t": c_int64,
    "y": c_int16,
    "x": c_int16,
    "p": c_uint8,
}

_DEFAULT_BUFF_SIZE = 4096


def check_file_encoding(fpath: Union[str, Path], encoding: str) -> None:
    if encoding == "dat":
        if not (str(fpath).endswith(".dat")):
            raise ValueError(
                "ERROR: The DAT encoding needs a '.dat' file to be specified."
            )
    elif encoding == "evt2" or encoding == "evt3":
        if not (str(fpath).endswith(".raw")):
            raise ValueError(
                "ERROR: The EVT2/EVT3 encoding needs a '.raw' file to be specified."
            )
    return


def check_encoding(encoding: str) -> str:
    if not (isinstance(encoding, str)):
        raise TypeError("ERROR: The encoding must be specified as a string.")
    encoding = encoding.lower()
    if not (encoding in _SUPPORTED_ENCODINGS):
        raise ValueError("ERROR: The encoding provided is not supported.")
    return encoding


def check_input_file(fpath: Union[str, Path], encoding: str) -> Union[str, Path]:
    if not (isinstance(fpath, str) or isinstance(fpath, Path)):
        raise TypeError(
            f"ERROR: The file path has to be provided as a string or a pathlib.Path object."
        )
    fpath = Path(fpath).resolve()
    if not (fpath.is_file()):
        raise ValueError("ERROR: The file provided does not exist.")
    check_file_encoding(fpath, encoding)
    return fpath


def check_output_file(fpath: Union[str, Path], encoding: str) -> Union[str, Path]:
    if not (isinstance(fpath, str) or isinstance(fpath, Path)):
        raise TypeError(
            "ERROR: The file path provided must be a string or a pathlib.Path object."
        )
    fpath = Path(fpath).resolve()
    if not fpath.parent.is_dir():
        raise NotADirectoryError("ERROR: The output file directory does not exist.")
    check_file_encoding(fpath, encoding)
    fpath.touch()
    if not fpath.is_file():
        raise SystemError("ERROR: The output file could not be created.")
    check_file_encoding(fpath, encoding)
    return fpath


def check_chunk_size(chunk_size: int, encoding: str) -> int:
    if not (isinstance(chunk_size, int)):
        raise TypeError("ERROR: The chunk size must be a positive integer number.")
    if encoding == "EVT3":
        if chunk_size < 12:
            raise ValueError(
                "ERROR: The chunk size has to be larger than 12 for the EVT3 encoding."
            )
    else:
        if chunk_size <= 0:
            raise ValueError("ERROR: The chunk size has to be larger than 0.")
    return chunk_size


def check_buff_size(buff_size: int) -> int:
    if not isinstance(buff_size, int):
        raise TypeError("ERROR: The buffer size must be a positive integer.")
    if buff_size <= 0:
        raise ValueError("ERROR: The buffer size must be larger than 0.")
    return buff_size


def check_new_duration(new_duration: int) -> int:
    if not (isinstance(new_duration, int)):
        raise TypeError(
            "ERROR: The new duration must be an integer positive number (not integer)."
        )
    if new_duration <= 0:
        raise ValueError("ERROR: The new duration must be larger than 0.")
    return new_duration


def check_external_file(
    fpath: Union[str, Path], self_fpath: Union[str, Path], encoding: str
) -> Union[str, Path]:
    if fpath is None:
        if self_fpath is None:
            raise ValueError("ERROR: An input file must be set or provided.")
        return self_fpath
    else:
        return check_input_file(fpath=fpath, encoding=encoding)


def check_time_window(time_window: int) -> int:
    if not isinstance(time_window, int):
        raise TypeError("ERROR: The time window must be an integer value.")
    if time_window <= 0:
        raise ValueError("ERROR: The time window must be a positive value.")
    return time_window


def check_dtype_order(dtype_order: tuple) -> tuple:
    if not isinstance(dtype_order, tuple):
        raise TypeError("ERROR: The time window must be a tuple of strings.")
    for element in dtype_order:
        if not isinstance(element, str):
            raise ValueError("ERROR: The tuple elements must be strings.")
    if set(dtype_order) != set(("t", "x", "y", "p")):
        raise ValueError(
            "ERROR: The dtype order must be a tuple of the form ('t', 'y', 'x', 'p')."
        )
    return dtype_order
