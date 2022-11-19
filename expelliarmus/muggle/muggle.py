from typing import Union, Optional
import pathlib
import numpy as np
from expelliarmus.muggle.muggle_wrapper import (
    read_dat_chunk,
    read_evt2_chunk,
    read_evt3_chunk,
)
from expelliarmus.muggle.clib import (
    dat_chunk_t,
    evt2_chunk_t,
    evt3_chunk_t,
    set_file_size,
    set_bytes_read,
)

from expelliarmus.utils import (
    check_input_file,
    check_encoding,
    _DEFAULT_DTYPE,
    _DEFAULT_BUFF_SIZE,
)


class Muggle:
    """
    Muggle allows you to read in chunks from a file using the read_chunk() generator.
    Args:
        - encoding: the encoding of the input file, to be chosen among DAT, EVT2 and EVT3.
        - fpath: path to the input file.
        - nevents_per_chunk: _minimum_ number of events included in the chunk.
        - buff_size: the buffer size used to read the binary file.
        - dtype: the dtype of the structured NumPy array.
    """

    def __init__(
        self,
        encoding: str,
        fpath: Union[str, pathlib.Path],
        nevents_per_chunk: int,
        buff_size: Optional[int] = _DEFAULT_BUFF_SIZE,
        dtype: Optional[np.dtype] = _DEFAULT_DTYPE,
    ):
        self._fpath = check_input_file(fpath, encoding)
        self._encoding = check_encoding(encoding)
        self._check_errors(nevents_per_chunk, buff_size, dtype)
        self._nevents_per_chunk = nevents_per_chunk
        self._buff_size = buff_size
        self._dtype = dtype
        self._read_fn = self._get_read_fn()
        self.chunk = self._get_chunk()
        return

    def _get_chunk(self):
        if self._encoding == "DAT":
            chunk = dat_chunk_t()
        elif self._encoding == "EVT2":
            chunk = evt2_chunk_t()
        elif self._encoding == "EVT3":
            chunk = evt3_chunk_t()
        chunk = set_bytes_read(chunk, 0)
        chunk = set_file_size(chunk, self._fpath.stat().st_size)
        return chunk

    def _get_read_fn(self):
        if self._encoding == "DAT":
            read_fn = read_dat_chunk
        elif self._encoding == "EVT2":
            read_fn = read_evt2_chunk
        elif self._encoding == "EVT3":
            read_fn = read_evt3_chunk
        return read_fn

    def set_file(self, fpath):
        self._fpath = check_input_file(fpath, self._encoding)
        self.reset()
        return

    def set_nevents_per_chunk(self, nevents_per_chunk):
        self._check_errors(nevents_per_chunk, self._buff_size, self._dtype)
        self._nevents_per_chunk = nevents_per_chunk
        self.reset()
        return

    def set_encoding(self, encoding):
        self._encoding = check_encoding(encoding)
        self._read_fn = self._get_read_fn()
        self.reset()
        return

    def reset(self):
        del self.chunk
        self.chunk = self._get_chunk()
        return

    def _check_errors(
        self,
        nevents_per_chunk: int,
        buff_size: int,
        dtype: np.dtype,
    ):
        assert (
            nevents_per_chunk > 0
        ), "A positive number of events per chunk to be read has to be provided."
        assert (
            isinstance(buff_size, int) and buff_size > 0
        ), "Error: the buffer size for the binary read has to be larger than 0."
        assert (
            isinstance(dtype, np.dtype) and len(dtype) == 4
        ), "The dtype provided is not valid. It should be [('t', type), ('x', type), ('y', type), ('p', type)] in any order."
        return

    def read_chunk(self):
        """
        The Muggle generator.
        Return:
            - arr: structured NumPy array of events.
        """
        while self.chunk.bytes_read < self.chunk.file_size:
            self.chunk, arr = self._read_fn(
                fpath=self._fpath,
                nevents_per_chunk=self._nevents_per_chunk,
                chunk=self.chunk,
                buff_size=self._buff_size,
                dtype=self._dtype,
            )
            if arr is None:
                break
            yield arr
