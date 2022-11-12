import expelliarmus 
import pathlib
import os

if os.sep=="/": # Unix system.
    TMPDIR = pathlib.Path("/tmp")
elif os.sep=="\\": # Findus system.
    TMPDIR = pathlib.Path(pathlib.Path().home(), "AppData", "Local", "Temp")
else:
    raise "Something went wrong with os.sep."

def test_dat_cutting(): 
    fpath_in = pathlib.Path("tests", "sample-files", "dat_test_NCARS.dat").resolve()
    fpath_out = TMPDIR.joinpath("dat_test")
    fpath_out.mkdir(exist_ok=True)
    fpath_out = fpath_out.joinpath("out_dat.dat")
    nevents = expelliarmus.cut_dat(fpath_in, fpath_out, max_nevents=25)
    assert nevents == 25
    arr = expelliarmus.read_dat(fpath_out)
    assert len(arr) == 25
    return 

def test_evt2_cutting(): 
    fpath_in = pathlib.Path("tests", "sample-files", "evt2_test_sparklers.raw").resolve()
    fpath_out = TMPDIR.joinpath("evt2_test")
    fpath_out.mkdir(exist_ok=True)
    fpath_out = fpath_out.joinpath("out_evt2.raw")
    nevents = expelliarmus.cut_evt2(fpath_in, fpath_out, max_nevents=25)
    assert nevents == 25
    arr = expelliarmus.read_evt2(fpath_out)
    assert len(arr) == 25
    return 

def test_evt3_cutting(): 
    fpath_in = pathlib.Path("tests", "sample-files", "evt3_test_pedestrians.raw").resolve()
    fpath_out = TMPDIR.joinpath("evt3_test")
    fpath_out.mkdir(exist_ok=True)
    fpath_out = fpath_out.joinpath("out_evt3.raw")
    nevents = expelliarmus.cut_evt3(fpath_in, fpath_out, max_nevents=25)
    assert nevents == 25
    arr = expelliarmus.read_evt3(fpath_out)
    assert len(arr) == 25
    return 
