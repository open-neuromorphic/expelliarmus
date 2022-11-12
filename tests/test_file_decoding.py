import expelliarmus
import pathlib

def test_dat_decoding():
    fpath = pathlib.Path("tests", "sample-files", "dat_test_NCARS.dat").resolve()
    events = expelliarmus.read_dat(fpath)

    assert len(events) == 4407


def test_evt2_decoding():
    fpath = pathlib.Path("tests", "sample-files", "evt2_test_sparklers.raw").resolve()
    events = expelliarmus.read_evt2(fpath)

    assert len(events) == 521252

def test_evt3_decoding():
    fpath = pathlib.Path("tests", "sample-files", "evt3_test_pedestrians.raw").resolve()
    events = expelliarmus.read_evt3(fpath)

    assert len(events) == 5000
