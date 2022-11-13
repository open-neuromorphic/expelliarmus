from typing import Union, Optional
import pathlib
from .muggle_wrapper import (
        read_dat_chunk,
        )

_SUPPORTED_ENCODINGS = (
        "DAT", 
        "EVT2", 
        "EVT3", 
        )

class Muggle():
    def __init__(self, fpath: Union[str, pathlib.Path], nevents_per_chunk: int, encoding: str):
        self._check_errors(fpath, nevents_per_chunk, encoding)
        self._bytes_read = 0
        self._nevents_chunk = nevents_per_chunk
        self._fpath = fpath
        self._encoding = encoding.upper()
        self._read_fn = self._get_read_fn()
        return

    def _get_read_fn(self):
        if self._encoding == "DAT":
            read_fn = read_dat_chunk
        elif self._encoding == "EVT2":
            read_fn = read_evt2_chunk
        elif self._encoding == "EVT3":
            read_fn = read_evt3_chunk
        return read_fn
    
    def _check_errors(self, fpath: Union[str, pathlib.Path], nevents_per_chunk: int, encoding: str):
        assert isinstance(fpath, str) or isinstance(fpat, pathlib.Path), "Error: the file path provided is not valid."
        fpath = pathlib.Path(fpath).resolve()
        assert fpath.is_file(), "Error: the input file provided does not exist."
        assert isinstance(nevents_per_chunk, int) and nevents_per_chunk>0, "Error: a positive number of events per chunk must be provided."
        assert isinstance(encoding, str) 
        encoding = encoding.upper()
        assert encoding in _SUPPORTED_ENCODINGS
        if encoding=="DAT":
            assert str(fpath).endswith(".dat")
        elif encoding=="EVT2" or encoding=="EVT3":
            assert str(fpath).endswith(".raw")
        return
    
    def reset(self):
        self._bytes_read = 0
        return

    def read_chunk(self):
        nevents_read = self._nevents_chunk
        while nevents_read >= self._nevents_chunk:
            self._bytes_read, arr = self._read_fn(fpath=self._fpath, nevents_per_chunk=self._nevents_chunk, bytes_read=self._bytes_read)
            if self._bytes_read == 0:
                break
            nevents_read = len(arr)
            yield arr
