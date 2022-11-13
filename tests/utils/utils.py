import pathlib
import os
import shutil
import numpy as np
from typing import Callable, Union, Optional

if os.sep == "/":  # Unix system.
    TMPDIR = pathlib.Path("/tmp")
elif os.sep == "\\":  # Findus system.
    TMPDIR = pathlib.Path(pathlib.Path().home(), "AppData", "Local", "Temp")
else:
    raise "Something went wrong with os.sep."


def test_cut_wrapper(
    cut_fn: Callable[
        [
            Union[str, pathlib.Path],
            Union[str, pathlib.Path],
            Optional[int],
            Optional[int],
        ],
        int,
    ],
    read_fn: Callable[
        [Union[str, pathlib.Path], Optional[int]], np.ndarray
    ],
    fname_in: Union[str, pathlib.Path],
    fname_out: Union[str, pathlib.Path],
    new_duration: int = 20
):
    fpath_in = pathlib.Path("tests", "sample-files", fname_in).resolve()
    fpath_out = TMPDIR.joinpath(cut_fn.__name__)
    fpath_out.mkdir(exist_ok=True)
    assert fpath_out.is_dir()
    fpath_out = fpath_out.joinpath(fname_out)
    fpath_out.touch()
    assert fpath_out.is_file()
    # Checking that the desired number of events has been encoded to the output file.
    nevents_out = cut_fn(fpath_in, fpath_out, new_duration=new_duration)
    arr = read_fn(fpath_out)
    assert len(arr) == nevents_out
    assert (arr["t"][-1] - arr["t"][0]) >= new_duration*1000
    # Checking that the cut is consistent.
    orig_arr = read_fn(fpath_in)
    assert (orig_arr[:nevents_out] == arr[:]).all()
    # Cleaning up.
    shutil.rmtree(fpath_out.parent)
    return

def test_read_wrapper(
    read_fn: Callable[
        [Union[str, pathlib.Path], Optional[int]], np.ndarray
    ],
    fname: Union[str, pathlib.Path],
    expected_nevents: int
):
    fpath = pathlib.Path("tests", "sample-files", fname).resolve()
    assert fpath.is_file()
    arr = read_fn(fpath)
    assert len(arr) == expected_nevents
    fpath = pathlib.Path("tests", "sample-files", fname.split('.')[0]+".npy")
    ref_arr = np.load(fpath)
    assert (ref_arr==arr).all() 
    return
