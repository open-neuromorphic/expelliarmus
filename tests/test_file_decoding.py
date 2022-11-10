import expelliarmus

def test_dat_decoding():
    events = expelliarmus.read_dat('tests/sample-files/dat_test_NCARS.dat')

    assert len(events) == 4407


def test_evt2_decoding():
    events = expelliarmus.read_evt2('tests/sample-files/evt2_test_sparklers.raw')

    assert len(events) == 521252

def test_evt3_decoding():
    events = expelliarmus.read_evt3('tests/sample-files/evt3_test_pedestrians.raw')

    assert len(events) == 5000
