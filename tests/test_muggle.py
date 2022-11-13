from .utils import utils


def test_dat_muggle():
    utils.test_muggle(encoding="dat", fname="dat_sample.dat")
    return


def test_evt2_muggler():
    utils.test_muggle(encoding="evt2", fname="evt2_sample.raw")
    return


def test_evt3_muggle():
    utils.test_muggle(encoding="evt3", fname="evt3_sample.raw")
    return
