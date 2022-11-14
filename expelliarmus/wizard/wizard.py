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
        dtype: Optional[np.dtype] = _DEFAULT_DTYPE,
    ):
        self._check_errors(buff_size, dtype)
        self._encoding = check_encoding(encoding)
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
        self,
        buff_size: int,
        dtype: np.dtype,
    ):
        assert (
            isinstance(buff_size, int) and buff_size > 0
        ), "Error: the buffer size for the binary read has to be larger than 0."
        assert (
            isinstance(dtype, np.dtype) and len(dtype) == 4
        ), "The dtype provided is not valid. It should be [('t', type), ('x', type), ('y', type), ('p', type)] in any order."
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
        nevents = self._cut_fn(
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
        return self._read_fn(fpath=fpath, buff_size=self._buff_size, dtype=self._dtype)
