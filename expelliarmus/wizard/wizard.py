from typing import Union, Optional
import pathlib
import shutil
from numpy import dtype as np_dtype
from numpy import ndarray
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
        - fpath: the file to be read, either in chunks or fully.
        - buff_size: the size of the buffer used to read the binary file.
    """

    def __init__(
        self,
        encoding: str,
        fpath: Optional[Union[str, pathlib.Path]] = None,
        chunk_size: Optional[int] = 8192,
        buff_size: Optional[int] = _DEFAULT_BUFF_SIZE,
    ):
        assert (
            isinstance(buff_size, int) and buff_size > 0
        ), "ERROR: The buffer size to read the file has to be an integere and it must have a size larger than 0."
        self._buff_size = buff_size
        self._encoding = check_encoding(encoding)
        if self._encoding == "EVT3":
            assert (
                chunk_size > 12
            ), "ERROR: The chunk size has to be larger than 12 for the EVT3 encoding."
        else:
            assert chunk_size > 0, "ERROR: The chunk size has to be larger than 0."
        if not (fpath is None):
            self._fpath = check_input_file(fpath=fpath, encoding=self._encoding)
        else:
            self._fpath = None
        self._chunk_size = chunk_size
        self.cargo = self._get_cargo()
        return

    def _get_cargo(self) -> Union[dat_cargo_t, evt2_cargo_t, evt3_cargo_t]:
        """
        Generates the cargo that carries information to the C code.
        """
        events_info = events_cargo_t(
            c_size_t(self._chunk_size),
            1,
            0,
            0,
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

    def set_file(self, fpath: Union[str, pathlib.Path]) -> None:
        """
        Function that sets the input file.
        """
        self._fpath = check_input_file(fpath, encoding=self._encoding)
        return

    def set_chunk_size(self, chunk_size: int) -> None:
        """
        Function to sets the size of the chunks to be read.
        WARNING: due to the vectorized events in event files, the chunks can be longer that 'chunk_size' (at most 'chunk_size+12').
        Args:
            - fpath: path to the input file.
            - chunk_size: size of the chunks ot be read.
        """
        assert chunk_size > 0, "ERROR: The chunk size has to be larger than 0."
        if self._encoding == "EVT3":
            assert (
                chunk_size > 12
            ), "ERROR: For EVT3 at least 12 events have to be read."
        self._chunk_size = chunk_size
        self.cargo = self._get_cargo()
        return

    def reset(self) -> None:
        """
        Function used to reset the generator, so that the file can be read from the beginning.
        """
        if not (self.cargo is None):
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
        fpath_out: Union[str, pathlib.Path],
        new_duration: int,
        fpath_in: Optional[Union[str, pathlib.Path]] = None,
    ) -> int:
        """
        Cuts the duration of the recording contained in 'fpath_in' to 'new_duration' and saves the result to 'fpath_out'.
        Args:
            - fpath_in: path to input file.
            - fpath_out: path to output file.
            - new_duration: the desired duration, expressed in milliseconds.
        Returns:
            - nevents: the number of events encoded in the output file.
        """
        if fpath_in is None:
            assert not (
                self._fpath is None
            ), "ERROR: An input file must be set or provided."
            fpath_in = self._fpath
        else:
            fpath_in = check_input_file(fpath=fpath_in, encoding=self._encoding)
        fpath_out = check_output_file(fpath=fpath_out, encoding=self._encoding)
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

    def read(self, fpath: Optional[Union[str, pathlib.Path]] = None) -> ndarray:
        """
        Reads a binary file to a structured NumPy of events.
        Args:
            - fpath: path to the input file.
        Returns:
            - arr: the structured NumPy array.
        """
        if fpath is None:
            assert not (
                self._fpath is None
            ), "ERROR: An input file must be set or provided."
            fpath = self._fpath
        else:
            fpath = check_input_file(fpath, self._encoding)
        arr, status = c_read_wrapper(
            encoding=self._encoding, fpath=fpath, buff_size=self._buff_size
        )
        assert status == 0, "ERROR: Something went wrong while reading the file."
        return arr

    def read_chunk(self) -> ndarray:
        """
        Returns:
            - arr: structured NumPy array of events.
        """
        assert not (self._fpath is None), "ERROR: An input file must be set."
        self.cargo.events_info.file_size = c_size_t(self._fpath.stat().st_size)
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
