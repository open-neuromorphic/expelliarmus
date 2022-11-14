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
    fpath_in = pathlib.Path("tests", "sample-files", fname_in).resolve()
    fpath_out = TMPDIR.joinpath("test_cut"+encoding)
    fpath_out.mkdir(exist_ok=True)
    assert fpath_out.is_dir()
    fpath_out = fpath_out.joinpath(fname_out)
    fpath_out.touch()
    assert fpath_out.is_file()
    # Checking that the desired number of events has been encoded to the output file.
    wizard_0 = Wizard(fpath_in, encoding=encoding)
    nevents_out = wizard_0.cut(fpath_out, new_duration=new_duration)
    wizard_1 = Wizard(fpath_out, encoding=encoding)
    arr = wizard_1.read()
    assert len(arr) == nevents_out
    assert (arr["t"][-1] - arr["t"][0]) >= new_duration * 1000
    # Checking that the cut is consistent.
    orig_arr = wizard_0.read()
    assert (orig_arr[:nevents_out] == arr[:]).all()
    # Cleaning up.
    shutil.rmtree(fpath_out.parent)
    return


def test_read(
    encoding: str,
    fname: Union[str, pathlib.Path],
    expected_nevents: int,
):
    fpath = pathlib.Path("tests", "sample-files", fname).resolve()
    assert fpath.is_file()
    wizard = Wizard(fpath, encoding=encoding)
    arr = wizard.read()
    assert len(arr) == expected_nevents
    assert arr["p"].min()==0 and arr["p"].max()==1
    assert (np.sort(arr["t"]) == arr["t"]).all()
    fpath = pathlib.Path("tests", "sample-files", fname.split(".")[0] + ".npy")
    ref_arr = np.load(fpath)
    assert (ref_arr == arr).all()
    return


def test_chunk_read(encoding: str, fname: Union[str, pathlib.Path]):
    fpath = pathlib.Path("tests", "sample-files", fname).resolve()
    assert fpath.is_file()
    muggler = Muggle(fpath, nevents_per_chunk=128, encoding=encoding)
    fpath = pathlib.Path("tests", "sample-files", fname.split(".")[0] + ".npy")
    ref_arr = np.load(fpath)
    nevents = len(ref_arr)
    chunk_offset = 0
    for chunk_arr in muggler.read_chunk():
        chunk_width = len(chunk_arr)
        assert (
            ref_arr[chunk_offset : min(chunk_offset + chunk_width, nevents)]
            == chunk_arr[:]
        ).all()
        chunk_offset += chunk_width
    return
