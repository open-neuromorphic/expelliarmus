from .utils import utils

NEVENTS_PER_CHUNK = 256


def test_dat_chunk_read():
    utils.test_chunk_read(
        encoding="dat", fname="dat_sample.dat", nevents_per_chunk=NEVENTS_PER_CHUNK
    )
    return


def test_evt2_chunk_read():
    utils.test_chunk_read(
        encoding="evt2", fname="evt2_sample.raw", nevents_per_chunk=NEVENTS_PER_CHUNK
    )
    return


def test_evt3_chunk_read():
    utils.test_chunk_read(
        encoding="evt3", fname="evt3_sample.raw", nevents_per_chunk=NEVENTS_PER_CHUNK
    )
    return
