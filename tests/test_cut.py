import expelliarmus
from .utils import utils

NEW_DURATION = 15


def test_dat_cutting():
    utils.test_cut(
        encoding="dat",
        fname_in="dat_sample.dat",
        fname_out="out_dat.dat",
        new_duration=NEW_DURATION,
        sensor_size=(640, 480),
    )
    return


def test_evt2_cutting():
    utils.test_cut(
        encoding="evt2",
        fname_in="evt2_sample.raw",
        fname_out="out_evt2.raw",
        new_duration=NEW_DURATION,
        sensor_size=(640, 480),
    )
    return


def test_evt3_cutting():
    utils.test_cut(
        encoding="evt3",
        fname_in="evt3_sample.raw",
        fname_out="out_evt3.raw",
        new_duration=NEW_DURATION,
        sensor_size=(1280, 720),
    )
    return
