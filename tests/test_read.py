from .utils import utils


def test_dat_decoding():
    utils.test_read(encoding="dat", fname="dat_sample.dat",  expected_nevents=4407)
    return


def test_evt2_decoding():
    utils.test_read(encoding="evt2", fname="evt2_sample.raw", expected_nevents=521252)
    return


def test_evt3_decoding():
    utils.test_read(encoding="evt3", fname="evt3_sample.raw", expected_nevents=5000)
    return
