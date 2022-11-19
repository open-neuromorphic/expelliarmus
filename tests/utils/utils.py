import pathlib
import os
import platform
import shutil
import numpy as np
from typing import Callable, Union, Optional
from expelliarmus import Muggle, Wizard

if platform.system() in ("Linux", "Darwin"):  # Unix system.
    TMPDIR = pathlib.Path("/tmp")
elif platform.system() == "Windows":  # Findus system.
    TMPDIR = pathlib.Path(pathlib.Path().home(), "AppData", "Local", "Temp")
else:
    raise Exception("The OS has not been recognized.")

COORDS = ["t", "x", "y", "p"]
DATA_TYPES = [
    (np.int64, np.uint64, np.int64, np.uint64),
    (np.uint32, np.uint16, np.int32, np.int16),
    (np.uint32, np.uint16, np.int32, np.int16),
    (bool, np.uint8, np.int8, np.int16),
]

DTYPES = []
for coord_i in range(len(COORDS)):
    COORDS = COORDS[1:] + COORDS[:1]
    DATA_TYPES = DATA_TYPES[1:] + DATA_TYPES[:1]
    for data_types in zip(*DATA_TYPES):
        tmp = [(COORDS[i], data_type) for i, data_type in enumerate(data_types)]
        DTYPES.append(np.dtype(tmp))

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
    ref_wizard = Wizard(encoding=encoding)
    ref_arr = ref_wizard.read(fpath_in)
    for dtype in DTYPES:
        wizard = Wizard(encoding=encoding, dtype=dtype)
        nevents_out = wizard.cut(fpath_in, fpath_out, new_duration=new_duration)
        arr = wizard.read(fpath_out)
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
    for dtype in DTYPES:
        wizard = Wizard(encoding=encoding, dtype=dtype)
        arr = wizard.read(fpath)
        assert (
            arr.dtype == dtype
        ), "Error: the resulting dtype does not coincide with the desired one."
        assert (
            len(arr) == expected_nevents
        ), "Error: the number of events in the array does not coincide with the expected one."
        _test_fields(ref_arr, arr, sensor_size)
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
    for chunk_size in CHUNK_SIZES:
        for dtype in DTYPES:
            muggler = Muggle(
                fpath=fpath,
                chunk_size=chunk_size,
                encoding=encoding,
                dtype=dtype,
            )
            for repetition in range(2):
                chunk_offset = 0
                for chunk_arr in muggler.read_chunk():
                    nevents = len(chunk_arr)
                    _test_fields(
                        ref_arr[
                            chunk_offset : min(chunk_offset + nevents, tot_nevents)
                        ],
                        chunk_arr,
                        sensor_size,
                    )
                    chunk_offset += nevents
                muggler.reset()
    return
