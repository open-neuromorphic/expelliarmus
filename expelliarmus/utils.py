from pathlib import Path
import numpy as np

_ROOT_PATH = Path(__file__).resolve().parent.parent

_SUPPORTED_ENCODINGS = (
    "DAT",
    "EVT2",
    "EVT3",
)

_DEFAULT_DTYPE = np.dtype(
    [("t", np.int64), ("x", np.int16), ("y", np.int16), ("p", np.uint8)]
)
_DEFAULT_BUFF_SIZE = 4096


def check_file_encoding(fpath, encoding):
    if encoding == "DAT":
        if not (str(fpath).endswith(".dat")):
            raise ValueError("ERROR: The DAT encoding needs a '.dat' file to be specified.")
    elif encoding == "EVT2" or encoding == "EVT3":
        if not (str(fpath).endswith(".raw")):
            raise ValueError("ERROR: The EVT2/EVT3 encoding needs a '.raw' file to be specified.")
    return


def check_encoding(encoding):
    if not (isinstance(encoding, str)):
        raise TypeError("ERROR: The encoding must be specified as a string.")
    encoding = encoding.upper()
    if not (encoding in _SUPPORTED_ENCODINGS):
        raise ValueError("ERROR: The encoding provided is not supported.")
    return encoding


def check_input_file(fpath, encoding):
    if not (isinstance(fpath, str) or isinstance(fpath, Path)):
        raise TypeError(f"ERROR: The file path has to be provided as a string or a pathlib.Path object.")
    fpath = Path(fpath).resolve()
    if not (fpath.is_file()):
        raise ValueError("ERROR: The file provided does not exist.")
    check_file_encoding(fpath, encoding)
    return fpath


def check_output_file(fpath, encoding):
    if not (isinstance(fpath, str) or isinstance(fpath, Path)):
        raise TypeError("ERROR: The file path provided must be a string or a pathlib.Path object.")
    fpath = Path(fpath).resolve()
    if not fpath.parent.is_dir():
        raise NotADirectoryError("ERROR: The output file directory does not exist.")
    check_file_encoding(fpath, encoding)
    fpath.touch()
    if not fpath.is_file():
        raise SystemError("ERROR: The output file could not be created.")
    check_file_encoding(fpath, encoding)
    return fpath

def check_chunk_size(chunk_size, encoding):
    if not (isinstance(chunk_size, int)):
        raise TypeError("ERROR: The chunk size must be a positive integer number.")
    if encoding == "EVT3":
        if chunk_size < 12: 
            raise ValueError("ERROR: The chunk size has to be larger than 12 for the EVT3 encoding.")
    else:
        if chunk_size <= 0: 
            raise ValueError("ERROR: The chunk size has to be larger than 0.")
    return chunk_size
