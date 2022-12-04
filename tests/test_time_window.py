from .utils import utils

TIME_WINDOW = 1


def test_dat_time_window():
    utils.test_time_window(
        encoding="dat",
        fname="dat_sample.dat",
        sensor_size=(640, 480),
        time_window=TIME_WINDOW,
    )
    return


def test_evt2_time_window():
    utils.test_time_window(
        encoding="evt2",
        fname="evt2_sample.raw",
        sensor_size=(640, 480),
        time_window=TIME_WINDOW,
    )
    return


def test_evt3_time_window():
    utils.test_time_window(
        encoding="evt3",
        fname="evt3_sample.raw",
        sensor_size=(1280, 720),
        time_window=TIME_WINDOW,
    )
    return
