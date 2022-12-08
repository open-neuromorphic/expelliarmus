import pathlib
import shutil
from ctypes import Structure, c_size_t
from typing import Optional, Union

from numpy import dtype as np_dtype
from numpy import ndarray

from expelliarmus.utils import (
    _DEFAULT_BUFF_SIZE,
    _DTYPES,
    check_buff_size,
    check_chunk_size,
    check_dtype_order,
    check_encoding,
    check_external_file,
    check_file_encoding,
    check_input_file,
    check_new_duration,
    check_output_file,
    check_time_window,
)
from expelliarmus.wizard.clib import c_cargos_t, events_cargo_t
from expelliarmus.wizard.wizard_wrapper import (
    c_cut_wrapper,
    c_read_chunk_wrapper,
    c_read_time_window_wrapper,
    c_read_wrapper,
    c_save_wrapper,
)


class Wizard:
    """
    Wizard allows to cast powerful spells, such as reading entire files to NumPy arrays or chunks of these, or cutting binary files to a certain duration.

    :param encoding: the encoding of the file, to be chosen among DAT, EVT2 and EVT3.
    :param fpath: the file to be read, either in chunks or fully.
    :param buff_size: the size of the buffer used to read the binary file.
    :param chunk_size: the chunk lenght when reading files in chunks.
    :param time_window: the time window lenght in millisecond when reading files in chunks of milliseconds.
    """

    def __init__(
        self,
        encoding: str,
        fpath: Optional[Union[str, pathlib.Path]] = None,
        chunk_size: Optional[int] = 8192,
        time_window: Optional[int] = 10,
        buff_size: Optional[int] = _DEFAULT_BUFF_SIZE,
    ) -> None:
        self._encoding = check_encoding(encoding)
        self.cargo = self._get_cargo()
        self.set_buff_size(buff_size)
        if fpath:
            self.set_file(fpath)
        else:
            self._fpath = None
        self.set_chunk_size(chunk_size)
        self.set_time_window(time_window)
        return

    @property
    def encoding(self) -> str:
        """
        The encoding of the files handles by Wizard.

        :returns: the encoding.
        """
        return self._encoding

    @property
    def fpath(self) -> pathlib.Path:
        """
        Wizard input file path.

        :returns: the file path.
        """
        return self._fpath

    @property
    def buff_size(self) -> int:
        """
        The buffer size used by Wizard to read the binary files.

        :returns: the buffer size.
        """
        return self._buff_size

    @property
    def chunk_size(self) -> int:
        """
        The length of each chunk provided by Wizard when reading files in chunks.

        :returns: the chunk size.
        """
        return self._chunk_size

    @property
    def time_window(self) -> int:
        """
        The time window lenght, expressed in milliseconds.

        :returns: the time window length.
        """
        return self._time_window

    @encoding.setter
    def encoding(self, value):
        raise AttributeError("ERROR: Denied setting of private attribute encoding.")

    @fpath.setter
    def fpath(self, value):
        raise AttributeError("ERROR: Denied setting of private attribute fpath.")

    @buff_size.setter
    def buff_size(self, value):
        raise AttributeError("ERROR: Denied setting of private attribute buff_size.")

    @chunk_size.setter
    def chunk_size(self, value):
        raise AttributeError("ERROR: Denied setting of private attribute chunk_size.")

    @time_window.setter
    def time_window(self, value):
        raise AttributeError("ERROR: Denied setting of private attribute time_window.")

    def _get_cargo(self) -> object:
        return c_cargos_t[self.encoding](events_info=events_cargo_t())

    def set_file(self, fpath: Union[str, pathlib.Path]) -> None:
        """
        Function that sets the input file.

        :param fpath: the path to the file.
        """
        self._fpath = check_input_file(fpath, encoding=self.encoding)
        self.reset()
        return

    def set_chunk_size(self, chunk_size: int) -> None:
        """
        Function to sets the size of the chunks to be read.
        WARNING: due to the vectorized events in event files, the chunks can be longer that 'chunk_size' (at most 'chunk_size+12').

        :param fpath: path to the input file.
        :param chunk_size: size of the chunks ot be read.
        """
        self._chunk_size = check_chunk_size(chunk_size, self.encoding)
        self.reset()
        return

    def set_encoding(self, encoding: Union[str, pathlib.Path]) -> None:
        """
        Sets the encoding proprierty of the class.

        :param encoding: the encoding of the file.
        """
        self._encoding = check_encoding(encoding)
        self.reset()
        return

    def set_buff_size(self, buff_size: int) -> None:
        """
        Sets te buffer size used to read the binary file.

        :param buff_size: the buffer size specified.
        """
        self._buff_size = check_buff_size(buff_size)
        self.reset()
        return

    def set_time_window(self, time_window: int) -> None:
        """
        Sets the time window lenght.

        :param buff_size: the time window specified.
        """
        self._time_window = check_time_window(time_window)
        self.reset()
        return

    def reset(self) -> None:
        """
        Function used to reset the generator, so that the file can be read from the beginning.
        """
        if self.cargo:
            del self.cargo
        self.cargo = self._get_cargo()
        return

    def cut(
        self,
        fpath_out: Union[str, pathlib.Path],
        new_duration: int,
        fpath_in: Optional[Union[str, pathlib.Path]] = None,
    ) -> int:
        """
        Cuts the duration of the recording contained in 'fpath_in' to 'new_duration' and saves the result to 'fpath_out'.

        :param fpath_in: path to input file.
        :param fpath_out: path to output file.
        :param new_duration: the desired duration, expressed in milliseconds.

        :returns: the number of events encoded in the output file.
        """
        fpath_in = check_external_file(fpath_in, self.fpath, self.encoding)
        fpath_out = check_output_file(fpath=fpath_out, encoding=self.encoding)
        new_duration = check_new_duration(new_duration)
        nevents = c_cut_wrapper(
            encoding=self.encoding,
            fpath_in=fpath_in,
            fpath_out=fpath_out,
            new_duration=new_duration,
            buff_size=self.buff_size,
        )
        return nevents

    def read(self, fpath: Optional[Union[str, pathlib.Path]] = None) -> ndarray:
        """
        Reads a binary file to a structured NumPy of events.

        :param fpath: path to the input file.

        :returns: the structured NumPy array.
        """
        fpath = check_external_file(fpath, self.fpath, self.encoding)
        arr, status = c_read_wrapper(
            encoding=self.encoding,
            fpath=fpath,
            buff_size=self.buff_size,
        )
        if status != 0:
            raise RuntimeError(
                "ERROR: Something went wrong while creating the array from the file."
            )
        return arr

    def save(
        self,
        fpath: Union[str, pathlib.Path],
        arr: ndarray,
    ) -> None:
        """
        Compresses the provided to the chosen encoding format.

        :param fpath: path to output file.
        :param arr: the NumPy array to be saved.

        :returns: the number of events encoded in the output file.
        """
        fpath = check_output_file(fpath=fpath, encoding=self.encoding)
        if not isinstance(arr, ndarray):
            raise TypeError("ERROR: A NumPy array must be provided.")
        if set(arr.dtype.names) != set(("t", "x", "y", "p")):
            raise TypeError(
                "ERROR: The structured NumPy array provided has not a ('t', 'x', 'y', 'p') structure."
            )
        if arr.size == 0:
            raise ValueError("ERROR: The NumPy array provided is empty.")
        status = c_save_wrapper(
            encoding=self.encoding,
            fpath=fpath,
            arr=arr,
            buff_size=self.buff_size,
        )
        if status != 0:
            raise RuntimeError("ERROR: Something went wrong while saving the array.")
        return

    def read_chunk(self) -> ndarray:
        """
        Generator used to read the file in chunks.

        :returns: structured NumPy array of events.
        """
        if self.fpath is None:
            raise ValueError("ERROR: An input file must be set.")
        self.cargo.events_info.is_chunk = 1
        self.cargo.events_info.is_time_window = 0
        while self.cargo.events_info.finished == 0:
            self.cargo.events_info.dim = self.chunk_size
            arr, self.cargo, status = c_read_chunk_wrapper(
                encoding=self.encoding,
                fpath=self.fpath,
                cargo=self.cargo,
                buff_size=self.buff_size,
            )
            if arr is None or status != 0:
                break
            yield arr

    def read_time_window(self) -> ndarray:
        """
        Generator used to read the file in time windows.

        :returns: structured NumPy array of events.
        """
        if self.fpath is None:
            raise ValueError("ERROR: An input file must be set.")
        self.cargo.events_info.is_chunk = 0
        self.cargo.events_info.is_time_window = 1
        self.cargo.events_info.time_window = self.time_window * 1000
        while self.cargo.events_info.finished == 0:
            arr, self.cargo, status = c_read_time_window_wrapper(
                encoding=self.encoding,
                fpath=self.fpath,
                cargo=self.cargo,
                buff_size=self.buff_size,
            )
            if arr is None or status != 0:
                break
            yield arr
