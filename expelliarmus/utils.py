import pathlib
import numpy as np

_ROOT_PATH = pathlib.Path(__file__).resolve().parent.parent

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
        assert str(fpath).endswith(
            ".dat"
        ), "The DAT encoding needs a '.dat' file to be specified."
    elif encoding == "EVT2" or encoding == "EVT3":
        assert str(fpath).endswith(
            ".raw"
        ), "The EVT2/EVT3 encoding needs a '.raw' file to be specified."
    return


def check_encoding(encoding):
    assert isinstance(encoding, str), "Error: the encoding must be a string."
    encoding = encoding.upper()
    assert (
        encoding in _SUPPORTED_ENCODINGS
    ), "Error: the encoding provided is not supported."
    return encoding


def check_input_file(fpath, encoding):
    assert isinstance(fpath, str) or isinstance(
        fpath, pathlib.Path
    ), f'Error: the file path provided, "{str(fpath)}", is not valid.'
    fpath = pathlib.Path(fpath).resolve()
    assert (
        fpath.is_file()
    ), 'Error: the input file provided, "{str(fpath)}", does not exist.'
    check_file_encoding(fpath, encoding)
    return fpath


def check_output_file(fpath, encoding):
    assert isinstance(fpath, str) or isinstance(
        fpath, pathlib.Path
    ), "The type of the file path provided is not valid."
    fpath = pathlib.Path(fpath).resolve()
    assert fpath.parent.is_dir(), "The output directory does not exist."
    check_file_encoding(fpath, encoding)
    fpath.touch()
    assert fpath.is_file(), "The output file specified cannot be created."
    check_file_encoding(fpath, encoding)
    return fpath
