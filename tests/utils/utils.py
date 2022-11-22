import pathlib
import os
import platform
import shutil
import numpy as np
from typing import Callable, Union, Optional
from expelliarmus import Wizard

if platform.system() in ("Linux", "Darwin"):  # Unix system.
    TMPDIR = pathlib.Path("/tmp")
elif platform.system() == "Windows":  # Findus system.
    TMPDIR = pathlib.Path(pathlib.Path().home(), "AppData", "Local", "Temp")
else:
    raise Exception("The OS has not been recognized.")

CHUNK_SIZES = tuple([2**i for i in range(7, 16 + 1)])


def _test_fields(ref_arr: np.ndarray, arr: np.ndarray, sensor_size: tuple):
    assert (
        arr["p"].min() >= 0 and arr["p"].max() <= 1
    ), f"Error: p_min={arr['p'].min()}, max_p={arr['p'].max()}."
    assert (
        arr["x"].min() >= 0 and arr["x"].max() < sensor_size[0]
    ), f"Error: {arr['x'].min()} <= x < {arr['x'].max()} (max {sensor_size[0]})."
    assert (
        arr["y"].min() >= 0 and arr["y"].max() < sensor_size[1]
    ), f"Error: {arr['y'].min()} <= y < {arr['y'].max()} (max {sensor_size[1]})."
    assert arr["t"].min() >= 0, f"Error: min_t={arr['t'].min()}."
    assert (
        np.sort(arr["t"]) == arr["t"]
    ).all(), "Error: the timestamps are not monotonic."
    assert (
        ref_arr["t"] == arr["t"]
    ).all(), "Error: the timestamps do not coincide with the reference."
    assert (
        ref_arr["x"] == arr["x"]
    ).all(), "Error: the x addresses do not coincide with the reference."
    assert (
        ref_arr["y"] == arr["y"]
    ).all(), "Error: the y addresses do not coincide with the reference."
    assert (
        ref_arr["p"] == arr["p"]
    ).all(), "Error: the polarities do not coincide with the reference."
    return


def test_cut(
    encoding: str,
    fname_in: Union[str, pathlib.Path],
    fname_out: Union[str, pathlib.Path],
    new_duration: int = 20,
    sensor_size=(640, 480),
):
    assert isinstance(fname_in, str) or isinstance(fname_in, pathlib.Path)
    assert isinstance(fname_out, str) or isinstance(fname_out, pathlib.Path)
    fpath_in = pathlib.Path("tests", "sample-files", fname_in).resolve()
    fpath_out = TMPDIR.joinpath("test_cut" + encoding)
    fpath_out.mkdir(exist_ok=True)
    assert fpath_out.is_dir()
    fpath_out = fpath_out.joinpath(fname_out)
    fpath_out.touch()
    assert fpath_out.is_file()
    # Checking that the desired number of events has been encoded to the output file.
    wizard = Wizard(encoding=encoding)
    wizard.set_file(fpath=fpath_in)
    ref_arr = wizard.read()
    nevents_out = wizard.cut(
        fpath_out=fpath_out, new_duration=new_duration
    )
    arr = wizard.read(fpath=fpath_out)
    assert (
        len(arr) == nevents_out
    ), "The lenght of the array does not coincide with the number of events returned by the C function."
    assert (
        arr["t"][-1] - arr["t"][0]
    ) >= new_duration * 1000, f"Error: the actual duration of the recording is {(arr['t'][-1] - arr['t'][0])/1000:.2f} ms instead of {new_duration} ms."
    # Checking that the cut is consistent.
    _test_fields(ref_arr[:nevents_out], arr, sensor_size)
    nevents_out = wizard.cut(
        fpath_in=fpath_in, fpath_out=fpath_out, new_duration=new_duration
    )
    arr = wizard.read(fpath=fpath_out)
    assert (
        len(arr) == nevents_out
    ), "The lenght of the array does not coincide with the number of events returned by the C function."
    assert (
        arr["t"][-1] - arr["t"][0]
    ) >= new_duration * 1000, f"Error: the actual duration of the recording is {(arr['t'][-1] - arr['t'][0])/1000:.2f} ms instead of {new_duration} ms."
    # Checking that the cut is consistent.
    _test_fields(ref_arr[:nevents_out], arr, sensor_size)
    # Cleaning up.
    shutil.rmtree(fpath_out.parent)
    return


def test_read(
    encoding: str,
    fname: Union[str, pathlib.Path],
    expected_nevents: int,
    sensor_size=(640, 480),
):
    assert isinstance(fname, str) or isinstance(fname, pathlib.Path)
    fpath = pathlib.Path("tests", "sample-files", fname).resolve()
    assert fpath.is_file()
    fpath_ref = pathlib.Path("tests", "sample-files", fname.split(".")[0] + ".npy")
    ref_arr = np.load(fpath_ref)
    wizard = Wizard(encoding=encoding, fpath=fpath)
    arr = wizard.read()
    assert (
        len(arr) == expected_nevents
    ), "Error: the number of events in the array does not coincide with the expected one."
    _test_fields(ref_arr, arr, sensor_size)
    try:
        wizard.set_encoding("stocazzo")
    except:
        print("Stocazzo!")
    wizard._encoding = "peppa_pig"
    try:
        wizard.read()
    except:
        print("Stocazzo!")
    return


def test_chunk_read(
    encoding: str,
    fname: Union[str, pathlib.Path],
    sensor_size=(640, 480),
):
    assert isinstance(fname, str) or isinstance(fname, pathlib.Path)
    fpath = pathlib.Path("tests", "sample-files", fname).resolve()
    assert fpath.is_file()
    ref_fpath = pathlib.Path("tests", "sample-files", fname.split(".")[0] + ".npy")
    ref_arr = np.load(ref_fpath)
    tot_nevents = len(ref_arr)
    wizard = Wizard(encoding=encoding, fpath=fpath)
    wizard.set_encoding(encoding)
    for chunk_size in CHUNK_SIZES:
        wizard.set_chunk_size(chunk_size)
        for repetition in range(2):
            chunk_offset = 0
            for chunk_arr in wizard.read_chunk():
                nevents = len(chunk_arr)
                _test_fields(
                    ref_arr[chunk_offset : min(chunk_offset + nevents, tot_nevents)],
                    chunk_arr,
                    sensor_size,
                )
                chunk_offset += nevents
            wizard.reset()
            _test_fields(ref_arr, np.concatenate([chunk for chunk in wizard.read_chunk()]), sensor_size=sensor_size)
            wizard.reset()
    return
