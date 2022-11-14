import pathlib
import os
import shutil
import numpy as np
from typing import Callable, Union, Optional
from expelliarmus import Muggle, Wizard

if os.sep == "/":  # Unix system.
    TMPDIR = pathlib.Path("/tmp")
elif os.sep == "\\":  # Findus system.
    TMPDIR = pathlib.Path(pathlib.Path().home(), "AppData", "Local", "Temp")
else:
    raise "Something went wrong with os.sep."


def test_cut(
    encoding: str, 
    fname_in: Union[str, pathlib.Path],
    fname_out: Union[str, pathlib.Path],
    new_duration: int = 20,
):
    assert isinstance(fname_in, str) or isinstance(fname_in, pathlib.Path)
    assert isinstance(fname_out, str) or isinstance(fname_out, pathlib.Path)
    fpath_in = pathlib.Path("tests", "sample-files", fname_in).resolve()
    fpath_out = TMPDIR.joinpath("test_cut"+encoding)
    fpath_out.mkdir(exist_ok=True)
    assert fpath_out.is_dir()
    fpath_out = fpath_out.joinpath(fname_out)
    fpath_out.touch()
    assert fpath_out.is_file()
    # Checking that the desired number of events has been encoded to the output file.
    wizard = Wizard(encoding=encoding)
    nevents_out = wizard.cut(fpath_in, fpath_out, new_duration=new_duration)
    arr = wizard.read(fpath_out)
    assert len(arr) == nevents_out
    assert (arr["t"][-1] - arr["t"][0]) >= new_duration * 1000
    # Checking that the cut is consistent.
    orig_arr = wizard.read(fpath_in)
    assert (orig_arr[:nevents_out] == arr[:]).all()
    # Cleaning up.
    shutil.rmtree(fpath_out.parent)
    return


def test_read(
    encoding: str,
    fname: Union[str, pathlib.Path],
    expected_nevents: int,
):
    assert isinstance(fname, str) or isinstance(fname, pathlib.Path)
    fpath = pathlib.Path("tests", "sample-files", fname).resolve()
    assert fpath.is_file()
    wizard = Wizard(encoding=encoding)
    arr = wizard.read(fpath)
    assert len(arr) == expected_nevents
    assert arr["p"].min()==0 and arr["p"].max()==1
    assert (np.sort(arr["t"]) == arr["t"]).all()
    fpath = pathlib.Path("tests", "sample-files", fname.split(".")[0] + ".npy")
    ref_arr = np.load(fpath)
    assert (ref_arr == arr).all()
    return


def test_chunk_read(encoding: str, fname: Union[str, pathlib.Path], nevents_per_chunk: int):
    assert isinstance(fname, str) or isinstance(fname, pathlib.Path)
    fpath = pathlib.Path("tests", "sample-files", fname).resolve()
    assert fpath.is_file()
    muggler = Muggle(encoding=encoding)
    fpath_ref = pathlib.Path("tests", "sample-files", fname.split(".")[0] + ".npy")
    ref_arr = np.load(fpath_ref)
    nevents = len(ref_arr)
    chunk_offset = 0
    for chunk_arr in muggler.read_chunk(fpath, nevents_per_chunk):
        chunk_width = len(chunk_arr)
        assert (
            ref_arr[chunk_offset : min(chunk_offset + chunk_width, nevents)]
            == chunk_arr[:]
        ).all()
        chunk_offset += chunk_width
    return
