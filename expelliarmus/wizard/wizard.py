from typing import Union, Optional
import pathlib
import shutil
import numpy as np
from expelliarmus.wizard.wizard_wrapper import (
    read_dat,
    read_evt2,
    read_evt3,
    cut_dat, 
    cut_evt2,
    cut_evt3,
)

_SUPPORTED_ENCODINGS = (
    "DAT",
    "EVT2",
    "EVT3",
)


class Wizard:
    """
    Muggle allows you to read in chunks from a file using the read_chunk() generator.
    Args:
        - fpath: path to the input file.
        - nevents_per_chunk: how many events per chunk you want.
        - encoding: the encoding of the input file, to be chosen among DAT, EVT2 and EVT3.
    """

    def __init__(
        self,
        fpath: Union[str, pathlib.Path],
        encoding: str,
        buff_size: Optional[int] = 4096,
        dtype: Optional[np.dtype] = np.dtype([("t", np.int64), ("x", np.int16), ("y", np.int16), ("p", np.uint8)]),
    ):
        self._check_errors(fpath, encoding, buff_size, dtype)
        self._fpath = fpath
        self._encoding = encoding.upper()
        self._buff_size = buff_size
        self._read_fn = self._get_read_fn()
        self._cut_fn = self._get_cut_fn()
        self._dtype = dtype
        return

    def _get_read_fn(self):
        if self._encoding == "DAT":
            read_fn = read_dat
        elif self._encoding == "EVT2":
            read_fn = read_evt2
        elif self._encoding == "EVT3":
            read_fn = read_evt3
        return read_fn

    def _get_cut_fn(self):
        if self._encoding == "DAT":
            cut_fn = cut_dat
        elif self._encoding == "EVT2":
            cut_fn = cut_evt2
        elif self._encoding == "EVT3":
            cut_fn = cut_evt3
        return cut_fn

    def _check_errors(
            self, fpath: Union[str, pathlib.Path], encoding: str, buff_size: int, dtype: np.dtype,
    ):
        assert isinstance(fpath, str) or isinstance(
            fpath, pathlib.Path
        ), f"Error: the file path provided, \"{str(fpath)}\", is not valid."
        fpath = pathlib.Path(fpath).resolve()
        assert fpath.is_file(), "Error: the input file provided, \"{str(fpath)}\", does not exist."
        assert isinstance(encoding, str)
        encoding = encoding.upper()
        assert encoding in _SUPPORTED_ENCODINGS, "The encoding is not supported."
        if encoding == "DAT":
            assert str(fpath).endswith(".dat"), "The DAT encoding needs a '.dat' file to be specified."
        elif encoding == "EVT2" or encoding == "EVT3":
            assert str(fpath).endswith(".raw"), "The EVT2/EVT3 encoding needs a '.raw' file to be specified."
        assert (
            isinstance(buff_size, int) and buff_size > 0
        ), "Error: the buffer size for the binary read has to be larger than 0."
        assert isinstance(dtype, np.dtype) and len(dtype) == 4, "The dtype provided is not valid. It should be [('t', type), ('x', type), ('y', type), ('p', type)] in any order."
        return

    def _check_errors_out_file(self, fpath_out, new_duration):
        assert isinstance(new_duration, int) and new_duration > 0, "A positive duration, expressed in milliseconds, needs to be provided."
        assert isinstance(fpath_out, str) or isinstance(fpath_out, pathlib.Path), "The type of the file path provided is not valid."
        fpath_out = pathlib.Path(fpath_out).resolve()
        assert fpath_out.parent.is_dir(), "The output directory does not exist."
        if self._encoding == "DAT":
            assert str(fpath_out).endswith(".dat"), "The DAT encoding needs a '.dat' file to be specified as output."
        elif self._encoding == "EVT2" or self._encoding == "EVT3":
            assert str(fpath_out).endswith(".raw"), "The EVT2/EVT3 encoding needs a '.raw' file to be specified as output."
        fpath_out.touch()
        assert fpath_out.is_file(), "The output file specified cannot be created."
        return fpath_out

    def cut(self, fpath_out, new_duration):
        fpath_out = self._check_errors_out_file(fpath_out, new_duration)
        nevents = self._cut_fn(fpath_in=self._fpath, fpath_out=fpath_out, new_duration=new_duration, buff_size=self._buff_size)
        return nevents

    def read(self):
        """
        The Muggle generator.
        Return:
            - arr: structured NumPy array of events.
        """
        return self._read_fn(fpath=self._fpath, buff_size=self._buff_size, dtype=self._dtype)
