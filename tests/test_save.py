import expelliarmus
from .utils import utils


def test_dat_save():
    utils.test_save(
        encoding="dat",
        fname_out="compressed_dat.dat",
        sensor_size=(640, 480),
    )
    return


def test_evt2_save():
    utils.test_save(
        encoding="evt2",
        fname_out="compressed_evt2.raw",
        sensor_size=(640, 480),
    )
    return


def test_evt3_save():
    utils.test_save(
        encoding="evt3",
        fname_out="compressed_evt3.raw",
        sensor_size=(1280, 720),
    )
    return
