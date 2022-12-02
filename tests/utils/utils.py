import pathlib
import os
import platform
import shutil
import numpy as np
from typing import Callable, Union, Optional
from expelliarmus import Wizard
from pytest import raises

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

    wizard = Wizard(encoding=encoding)

    # Error checking on unset fpath.
    with raises(ValueError):
        wizard.cut(fpath_out=fpath_out, new_duration=new_duration)

    wizard.set_file(fpath_in)

    # Error checking on new_duration.
    with raises(TypeError):
        wizard.cut(fpath_out=fpath_out, new_duration=12.2)
    with raises(ValueError):
        wizard.cut(fpath_out=fpath_out, new_duration=-12)

    # Error checking on fpath_out.
    with raises(TypeError):
        wizard.cut(fpath_out=12, new_duration=new_duration)
    with raises(ValueError):
        wizard.cut(fpath_out="peppapig", new_duration=new_duration)

    # Checking that the desired number of events has been encoded to the output file.
    ref_arr = wizard.read()
    nevents_out = wizard.cut(fpath_out=fpath_out, new_duration=new_duration)
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

    # Error handling in the constructor.
    with raises(ValueError):
        wizard = Wizard(encoding=encoding, fpath="non_existing_path")
    with raises(ValueError):
        wizard = Wizard(encoding="non_existing_encoding", fpath=fpath)
    with raises(ValueError):
        wizard = Wizard(encoding=encoding, fpath=fpath, buff_size=-1)
    with raises(TypeError):
        wizard = Wizard(encoding=encoding, fpath=3)
    with raises(TypeError):
        wizard = Wizard(encoding=3, fpath=fpath)
    with raises(TypeError):
        wizard = Wizard(encoding=encoding, fpath=fpath, buff_size=1.21)

    wizard = Wizard(encoding=encoding, fpath=fpath)

    # Error checking in set_encoding.
    with raises(ValueError):
        wizard.set_encoding("peppapig")
    with raises(TypeError):
        wizard.set_encoding(1.2123)

    # Error checking in set_buff_size.
    with raises(ValueError):
        wizard.set_buff_size(-1)
    with raises(TypeError):
        wizard.set_buff_size(1.2123)

    # Error checking in set_fpath.
    with raises(ValueError):
        wizard.set_file("peppapig")
    with raises(TypeError):
        wizard.set_file(1.2123)

    # Error checking on private attributes.
    with raises(AttributeError):
        wizard.encoding = "dat"
    with raises(AttributeError):
        wizard.fpath = "./somewhere"
    with raises(AttributeError):
        wizard.buff_size = 321

    arr = wizard.read()
    assert (
        len(arr) == expected_nevents
    ), "ERROR: the number of events in the array does not coincide with the expected one."
    _test_fields(ref_arr, arr, sensor_size)
    return


def test_save(
    encoding: str,
    fname_out: Union[str, pathlib.Path],
    sensor_size: tuple = (640, 480),
):
    for fname_in in ("dat_sample.npy", "evt2_sample.npy", "evt3_sample.npy"):
        assert isinstance(fname_in, str) or isinstance(fname_in, pathlib.Path)
        assert str(fname_in).endswith(".npy")
        assert isinstance(fname_out, str) or isinstance(fname_out, pathlib.Path)
        fpath_in = pathlib.Path("tests", "sample-files", fname_in).resolve()
        fpath_out = TMPDIR.joinpath("test_save" + encoding)
        fpath_out.mkdir(exist_ok=True)
        assert fpath_out.is_dir()
        fpath_out = fpath_out.joinpath(fname_out)
        fpath_out.touch()
        assert fpath_out.is_file()

        wizard = Wizard(encoding=encoding)
        np_arr = np.load(fpath_in)

        try:
            # Error checking on input array.
            with raises(TypeError):
                wizard.save(fpath=fpath_out, arr=np.zeros((20,)))
            with raises(TypeError):
                wizard.save(
                    fpath=fpath_out,
                    arr=np.zeros(
                        (20,),
                        dtype=np.dtype(
                            [("a", int), ("b", int), ("c", int), ("d", int)]
                        ),
                    ),
                )
            with raises(ValueError):
                wizard.save(
                    fpath=fpath_out,
                    arr=np.array(
                        [],
                        dtype=np.dtype(
                            [("t", int), ("x", int), ("y", int), ("p", int)]
                        ),
                    ),
                )

            # Error checking on fpath_out.
            with raises(TypeError):
                wizard.save(fpath=12, arr=np_arr)
            with raises(ValueError):
                wizard.save(fpath="peppapig", arr=np_arr)

            # Checking that compressed file is consistent.
            wizard.save(fpath=fpath_out, arr=np_arr)
            uncmp_arr = wizard.read(fpath_out)
            _test_fields(np_arr, uncmp_arr, sensor_size)
        except NotImplementedError:
            print(
                f"WARNING: The save method is not implemented for {encoding} encoding."
            )
        # Cleaning up.
        shutil.rmtree(fpath_out.parent)
        return


def test_chunk_read(
    encoding: str,
    fname: Union[str, pathlib.Path],
    sensor_size: tuple = (640, 480),
):
    assert isinstance(fname, str) or isinstance(fname, pathlib.Path)
    fpath = pathlib.Path("tests", "sample-files", fname).resolve()
    assert fpath.is_file()
    ref_fpath = pathlib.Path("tests", "sample-files", fname.split(".")[0] + ".npy")
    ref_arr = np.load(ref_fpath)
    tot_nevents = len(ref_arr)

    # Error checking in constructor.
    with raises(ValueError):
        wizard = Wizard(encoding=encoding, fpath=fpath, chunk_size=-2)
    with raises(TypeError):
        wizard = Wizard(encoding=encoding, fpath=fpath, chunk_size=2.1)

    # Error checking on using read_chunk() with no fpath.
    wizard = Wizard(encoding=encoding, chunk_size=512)
    with raises(ValueError):
        next(wizard.read_chunk())

    wizard = Wizard(encoding=encoding, fpath=fpath)

    # Error checking on setting private attribute.
    with raises(AttributeError):
        wizard.chunk_size = 21

    # Error checking in set_chunk_size.
    with raises(ValueError):
        wizard.set_chunk_size(-23)
    with raises(TypeError):
        wizard.set_chunk_size(1.2123)

    wizard = Wizard(encoding=encoding, fpath=fpath)
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
            _test_fields(
                ref_arr,
                np.concatenate([chunk for chunk in wizard.read_chunk()]),
                sensor_size=sensor_size,
            )
            wizard.reset()
    return
