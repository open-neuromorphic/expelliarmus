from typing import Union, Optional
import pathlib
from .muggle_wrapper import (
    read_dat_chunk,
    read_evt2_chunk,
    read_evt3_chunk,
)
from .clib_muggle import dat_chunk_wrap_t, evt2_chunk_wrap_t, evt3_chunk_wrap_t

_SUPPORTED_ENCODINGS = (
    "DAT",
    "EVT2",
    "EVT3",
)


class Muggle:
    def __init__(
        self, fpath: Union[str, pathlib.Path], nevents_per_chunk: int, encoding: str
    ):
        self._check_errors(fpath, nevents_per_chunk, encoding)
        self._nevents_chunk = nevents_per_chunk
        self._fpath = fpath
        self._encoding = encoding.upper()
        self._read_fn = self._get_read_fn()
        self._chunk_wrap = self._get_chunk_wrap()
        return

    def _get_chunk_wrap(self):
        if self._encoding == "DAT":
            chunk_wrap = dat_chunk_wrap_t()
        elif self._encoding == "EVT2":
            chunk_wrap = evt2_chunk_wrap_t()
        elif self._encoding == "EVT3":
            chunk_wrap = evt3_chunk_wrap_t()
        chunk_wrap.bytes_read = 0
        return chunk_wrap

    def _get_read_fn(self):
        if self._encoding == "DAT":
            read_fn = read_dat_chunk
        elif self._encoding == "EVT2":
            read_fn = read_evt2_chunk
        elif self._encoding == "EVT3":
            read_fn = read_evt3_chunk
        return read_fn

    def _check_errors(
        self, fpath: Union[str, pathlib.Path], nevents_per_chunk: int, encoding: str
    ):
        assert isinstance(fpath, str) or isinstance(
            fpath, pathlib.Path
        ), "Error: the file path provided is not valid."
        fpath = pathlib.Path(fpath).resolve()
        assert fpath.is_file(), "Error: the input file provided does not exist."
        assert (
            isinstance(nevents_per_chunk, int) and nevents_per_chunk > 0
        ), "Error: a positive number of events per chunk must be provided."
        assert isinstance(encoding, str)
        encoding = encoding.upper()
        assert encoding in _SUPPORTED_ENCODINGS
        if encoding == "DAT":
            assert str(fpath).endswith(".dat")
        elif encoding == "EVT2" or encoding == "EVT3":
            assert str(fpath).endswith(".raw")
        return

    def reset(self):
        self._chunk_wrap.bytes_read = 0
        return

    def read_chunk(self):
        nevents_read = self._nevents_chunk
        while nevents_read >= self._nevents_chunk:
            self._chunk_wrap, arr = self._read_fn(
                fpath=self._fpath,
                nevents_per_chunk=self._nevents_chunk,
                chunk_wrap=self._chunk_wrap,
            )
            if self._chunk_wrap.bytes_read == 0:
                break
            nevents_read = len(arr)
            yield arr
