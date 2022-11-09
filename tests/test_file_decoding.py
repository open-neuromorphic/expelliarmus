import expelliarmus

def test_dat_decoding():
    events = expelliarmus.read_dat('tests/sample-files/NCARS_obj_004397_td.dat')

    assert len(events) == 4407


def test_evt2_decoding():
    events = expelliarmus.read_evt2('tests/sample-files/sparklers.raw')

    assert len(events) == 521252
