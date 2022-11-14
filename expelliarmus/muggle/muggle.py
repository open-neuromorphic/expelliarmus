from typing import Union, Optional
import pathlib
from expelliarmus.muggle.muggle_wrapper import (
    read_dat_chunk,
    read_evt2_chunk,
    read_evt3_chunk,
)
from expelliarmus.muggle.clib import (
    dat_chunk_wrap_t,
    evt2_chunk_wrap_t,
    evt3_chunk_wrap_t,
)

from expelliarmus.utils import check_input_file, check_encoding

class Muggle:
    """
    Muggle allows you to read in chunks from a file using the read_chunk() generator.
    Args:
        - encoding: the encoding of the input file, to be chosen among DAT, EVT2 and EVT3.
    """

    def __init__(
        self, encoding: str
    ):
        self._encoding = check_encoding(encoding)
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

    def read_chunk(self, fpath: Union[str, pathlib.Path], nevents_per_chunk: int):
        """
        The Muggle generator.
        Args:
            - fpath: path to the input file.
            - nevents_per_chunk: _minimum_ number of events included in the chunk.
        Return:
            - arr: structured NumPy array of events.
        """
        assert nevents_per_chunk>0, "A positive number of events per chunk to be read has to be provided."
        nevents_read = nevents_per_chunk
        fpath = check_input_file(fpath, self._encoding)
        while nevents_read >= nevents_per_chunk:
            self._chunk_wrap, arr = self._read_fn(
                fpath=fpath,
                nevents_per_chunk=nevents_per_chunk,
                chunk_wrap=self._chunk_wrap,
            )
            if self._chunk_wrap.bytes_read == 0:
                break
            nevents_read = len(arr)
            yield arr
