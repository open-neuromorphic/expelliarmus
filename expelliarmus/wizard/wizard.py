from typing import Union, Optional
import pathlib
import shutil
from numpy import dtype as np_dtype
from ctypes import c_size_t
from expelliarmus.wizard.clib import (
    event_t,
    dat_cargo_t,
    evt2_cargo_t,
    evt3_cargo_t,
    events_cargo_t,
)
from expelliarmus.wizard.wizard_wrapper import (
    c_read_wrapper,
    c_cut_wrapper,
    c_read_chunk_wrapper,
)
from expelliarmus.utils import (
    check_file_encoding,
    check_encoding,
    check_input_file,
    check_output_file,
    _DEFAULT_DTYPE,
    _DEFAULT_BUFF_SIZE,
)


class Wizard:
    """
    Wizard allows to cast powerful spells, such as reading entire files to NumPy arrays or cutting binary files to a certain duration.
    Args:
        - encoding: the encoding of the file, to be chosen among DAT, EVT2 and EVT3.
        - buff_size: the size of the buffer used to read the binary file.
        - dtype: the type of the desired structured NumPy array.
    """

    def __init__(
        self,
        encoding: str,
        buff_size: Optional[int] = _DEFAULT_BUFF_SIZE,
        dtype: Optional[np_dtype] = _DEFAULT_DTYPE,
    ):
        self._check_errors(buff_size, dtype)
        self._encoding = check_encoding(encoding)
        self._buff_size = buff_size
        self._dtype = dtype
        self._chunk_size = 0
        self._fpath = None
        self.cargo = None
        return

    def _get_cargo(self):
        events_info = events_cargo_t(
            c_size_t(self._chunk_size),
            1,
            0,
            c_size_t(self._fpath.stat().st_size),
        )
        if self._encoding == "DAT":
            cargo = dat_cargo_t(events_info, 0, 0)
        elif self._encoding == "EVT2":
            cargo = evt2_cargo_t(events_info, 0, 0)
        elif self._encoding == "EVT3":
            event = event_t(0, 0, 0, 0)
            cargo = evt3_cargo_t(events_info, 0, 0, 0, 0, 0, event)
        else:
            raise Exception("ERROR: Encoding not valid.")
        return cargo

    def _check_errors(
        self,
        buff_size: int,
        dtype: np_dtype,
    ):
        assert (
            isinstance(buff_size, int) and buff_size > 0
        ), "Error: the buffer size for the binary read has to be larger than 0."
        assert (
            isinstance(dtype, np_dtype) and len(dtype) == 4
        ), "The dtype provided is not valid. It should be [('t', type), ('x', type), ('y', type), ('p', type)] in any order."
        return

    def setup_chunk(self, fpath, chunk_size):
        self._fpath = check_input_file(fpath, self._encoding)
        assert chunk_size > 0, "ERROR: The chunk size has to be larger than 0."
        if self._encoding == "EVT3":
            assert chunk_size > 12, "ERROR: For EVT3 at least 12 events have to be read."
        self._chunk_size = chunk_size
        self.reset()
        return

    def reset(self):
        if self.cargo:
            del self.cargo
        self.cargo = self._get_cargo()
        return

    def set_encoding(self, encoding: Union[str, pathlib.Path]) -> None:
        """
        Sets the encoding proprierty of the class.
        Args:
            - encoding: the encoding of the file.
        """
        self._encoding = check_encoding(encoding)
        return

    def cut(
        self,
        fpath_in: Union[str, pathlib.Path],
        fpath_out: Union[str, pathlib.Path],
        new_duration: int,
    ):
        """
        Cuts the duration of the recording contained in 'fpath_in' to 'new_duration' and saves the result to 'fpath_out'.
        Args:
            - fpath_in: path to input file.
            - fpath_out: path to output file.
            - new_duration: the desired duration, expressed in milliseconds.
        Returns:
            - nevents: the number of events encoded in the output file.
        """
        fpath_in = check_input_file(fpath_in, self._encoding)
        fpath_out = check_output_file(fpath_out, self._encoding)
        assert (
            isinstance(new_duration, int) and new_duration > 0
        ), "A positive duration, expressed in milliseconds, needs to be provided."
        nevents = c_cut_wrapper(
            encoding=self._encoding,
            fpath_in=fpath_in,
            fpath_out=fpath_out,
            new_duration=new_duration,
            buff_size=self._buff_size,
        )
        return nevents

    def read(self, fpath: Union[str, pathlib.Path]):
        """
        Reads a binary file to a structured NumPy of events.
        Args:
            - fpath: path to the input file.
        Returns:
            - arr: the structured NumPy array.
        """
        fpath = check_input_file(fpath, self._encoding)
        arr, status = c_read_wrapper(encoding=self._encoding, fpath=fpath, buff_size=self._buff_size)
        assert status==0, "ERROR: Something went wrong while reading the file."
        return arr

    def read_chunk(self):
        """
        Return:
            - arr: structured NumPy array of events.
        """
        assert self._fpath, "ERROR: The file is not set."
        assert self._chunk_size > 0, "ERROR: The chunk size is not set."
        while self.cargo.events_info.dim > 0:
            arr, self.cargo, status = c_read_chunk_wrapper(
                encoding=self._encoding,
                fpath=self._fpath,
                cargo=self.cargo,
                buff_size=self._buff_size,
            )
            if arr is None or status != 0:
                break
            yield arr
