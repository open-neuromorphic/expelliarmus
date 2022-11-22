![expelliarmus](docs/_static/Logo.png)

[![PyPI](https://img.shields.io/pypi/v/expelliarmus)](https://pypi.org/project/expelliarmus/)
[![codecov](https://codecov.io/gh/fabhertz95/expelliarmus/branch/develop/graph/badge.svg?token=Q0BMYGUSZQ)](https://codecov.io/gh/fabhertz95/expelliarmus)
[![contributors](https://img.shields.io/github/contributors-anon/fabhertz95/expelliarmus)](https://github.com/fabhertz95/expelliarmus/pulse)
[![Documentation Status](https://readthedocs.org/projects/expelliarmus/badge/?version=latest)](https://expelliarmus.readthedocs.io/en/latest/?badge=latest)
[![Discord](https://img.shields.io/discord/852094154188259338)](https://discord.gg/JParSCNe5k)


A Python/C library for decoding DVS binary data formats to NumPy structured arrays.

## Supported formats
- DAT (Prophesee).
- EVT2 (Prophesee).
- EVT3 (Prophesee). 

## Installation 
You can install the library through `pip`:
```bash
pip install expelliarmus 
```

The package is tested on Windows, MacOS and Linux.

## Quickstart
Shall we start practicing some spells? For that, we need a `Wizard`!


```python
from expelliarmus import Wizard
```

Let's cast a spell called `read()` and read [this RAW file](https://dataset.prophesee.ai/index.php/s/fB7xvMpE136yakl/download) to a structured NumPy array! 

```python
wizard = Wizard(encoding="evt3", fpath="./pedestrians.raw")
arr = wizard.read()
print(arr.shape) # Number of events encoded to the NumPy array.
```

    (39297796,)


The array is a collection of `(timestamp, x_address, y_address, polarity)` tuples. 


```python
print(arr.dtype)
```

    [('t', '<i8'), ('x', '<i2'), ('y', '<i2'), ('p', 'u1')]


A typical sample looks like this:


```python
print(arr[0])
```

    (5840504, 707, 297, 0)


If we would like to reduce the EVT3 file size, we can use the `cut(fpath_in, fpath_out, new_duration)` spell to limit the recording time duration to `12ms`, for instance:


```python
nevents = wizard.cut(fpath_out="./pedestrians_cut.raw", new_duration=12)
print(f"Number of events embedded in the cut file: {nevents}.") # The number of events embedded in the output file.
```

    Number of events embedded in the cut file: 540.


This can be verified by reading the new file in an array.


```python
cut_arr = wizard.read(fpath="./pedestrians_cut.raw")
print(f"Length of array extracted from the cut recording: {len(cut_arr)}.")
```

    Length of array extracted from the cut recording: 540.


The files are consistent:


```python
print(f"First original sample: {arr[0]} | First cut sample: {cut_arr[0]}.")
print(f"{nevents}th original sample: {arr[nevents-1]} | Last cut sample: {cut_arr[-1]}.")
print((arr[:nevents]==cut_arr[:]).all())
```

    First original sample: (5840504, 707, 297, 0) | First cut sample: (5840504, 707, 297, 0).
    540th original sample: (5853218, 1208, 253, 0) | Last cut sample: (5853218, 1208, 253, 0).
    True


The time duration is, more or less, the desired one (the events are discrete, hence we have not a fine control over them).


```python
print(f"New recording duration: {((cut_arr['t'][-1] - cut_arr['t'][0])/1000):.2f} ms") 
```

    New recording duration: 12.71 ms


What if you wand is not strong enough for handling spells on very large recordings? Well, we can try to read the files one chunk at time...


```python
wizard.set_chunk_size(chunk_size=512)
print(f"Length of the chunk: {len(next(wizard.read_chunk()))}.")
```

    Length of the chunk: 512.


Let's read less events, so that we are able to visualize them


```python
wizard.set_chunk_size(chunk_size=16)
print(next(wizard.read_chunk()))
```

    [(5848837,  610, 296, 1) (5848843,  834, 302, 1) (5848846,  593, 254, 1)
     (5848846, 1003, 298, 1) (5848859,  610, 299, 1) (5848887,  709, 306, 0)
     (5848888,  756, 292, 0) (5848895,  704, 300, 0) (5848903,  744, 169, 1)
     (5848904, 1209, 252, 0) (5848905,  709, 307, 0) (5848911,  139, 315, 0)
     (5848918,  603, 301, 1) (5848918,  708, 299, 1) (5848924,  778, 295, 1)
     (5848967,  140, 315, 0)]

## A small benchmark

Here it is a small benchmark using `expelliarmus` on the file formats supported. The data shows the file size, read time for the full file and read time for reading the file in chunks.

```python
from expelliarmus import Wizard
import pathlib
import h5py
import numpy as np
import timeit
import requests

FIRST_RUN = False
REPEAT = 100
```


```python
def get_diff_perc_str(ref, val):
    if (val > ref):
        return f"+{(val/ref-1)*100:.2f}%"
    else:
        return f"-{(1-val/ref)*100:.2f}%"
    
get_fsize_MB = lambda fpath: round(fpath.stat().st_size/(1024*1024))
```


```python
if FIRST_RUN:
    # Downloading files.
    if not pathlib.Path("./spinner.dat").is_file():
        print("Downloading DAT file...")
        r = requests.get("https://dataset.prophesee.ai/index.php/s/YAri3vpPZHhEZfc/download", allow_redirects=True) # spinner.dat, DAT
        open('./spinner.dat', 'wb').write(r.content)
    if not pathlib.Path("./monitoring_40_50hz.raw").is_file():
        print("Downloading EVT2 file...")
        r = requests.get("https://dataset.prophesee.ai/index.php/s/s5DFqzVQhlaU8Y5/download", allow_redirects=True) # monitoring_40_50hz.raw, EVT2
        open('./monitoring_40_50hz.raw', 'wb').write(r.content)
    if not pathlib.Path("./driving_sample.raw").is_file():
        print("Downloading EVT3 file...")
        r = requests.get("https://dataset.prophesee.ai/index.php/s/nVcLLdWAnNzrmII/download", allow_redirects=True) # driving_sample.raw, EVT3
        open('./driving_sample.raw', 'wb').write(r.content)
    print("Everything downloaded!")
files = ("spinner.dat", "monitoring_40_50hz.raw", "driving_sample.raw")
```


```python
softwares = ("expelliarmus", "hdf5", "hdf5_lzf", "hdf5_gzip", "numpy")
encodings = ("dat", "evt2", "evt3")
metrics = ("fsize", "full_read", "chunk_read")
data = {}
for software in softwares:
    data[software] = {}
    for encoding in encodings:
        data[software][encoding] = {}
        for metric in metrics:
            data[software][encoding][metric] = 0
```


```python
wizard = Wizard(encoding="dat")
print("="*50+"\nFull file read")
for f, encoding in zip(files, encodings):
    print("="*50)

    exp_fpath = pathlib.Path(f)
    hdf5_fpath = pathlib.Path(f"./ref_HDF5_{encoding.upper()}.hdf5")
    hdf5_lzf_fpath = pathlib.Path(f"./ref_HDF5_LZF_{encoding.upper()}.hdf5")
    hdf5_gzip_fpath = pathlib.Path(f"./ref_HDF5_GZIP_{encoding.upper()}.hdf5")
    np_fpath = pathlib.Path(f"./ref_np_{encoding.upper()}.npy")

    wizard.set_encoding(encoding)
    wizard.set_file(exp_fpath)
    if FIRST_RUN:
        arr = wizard.read()
    data["expelliarmus"][encoding]["fsize"] = get_fsize_MB(exp_fpath)

    # HDF5 
    if FIRST_RUN:
        hdf5_fp = h5py.File(hdf5_fpath, "w")
        arr_hdf5 = hdf5_fp.create_dataset("arr", arr.shape, arr.dtype)
        arr_hdf5[:] = arr[:]
        hdf5_fp.close()
    data["hdf5"][encoding]["fsize"] = get_fsize_MB(hdf5_fpath)

    # HDF5 LZF
    if FIRST_RUN:
        hdf5_lzf_fp = h5py.File(hdf5_lzf_fpath, "w")
        arr_hdf5_lzf = hdf5_lzf_fp.create_dataset("arr", arr.shape, arr.dtype, compression="lzf")
        arr_hdf5_lzf[:] = arr[:]
        hdf5_lzf_fp.close()
    data["hdf5_lzf"][encoding]["fsize"] = get_fsize_MB(hdf5_lzf_fpath)
    
    # HDF5 GZIP
    if FIRST_RUN:
        hdf5_gzip_fp = h5py.File(hdf5_gzip_fpath, "w")
        arr_hdf5_gzip = hdf5_gzip_fp.create_dataset("arr", arr.shape, arr.dtype, compression="gzip")
        arr_hdf5_gzip[:] = arr[:]
        hdf5_gzip_fp.close()
    data["hdf5_gzip"][encoding]["fsize"] = get_fsize_MB(hdf5_gzip_fpath)
    
    # NumPy
    if FIRST_RUN:
        np.save(np_fpath, arr, allow_pickle=False)
    data["numpy"][encoding]["fsize"] = get_fsize_MB(np_fpath)

    data["expelliarmus"][encoding]["full_read"] = sum(timeit.repeat(lambda: wizard.read(), number=1, repeat=REPEAT))/REPEAT
    print(f'{encoding.upper()} ({data["expelliarmus"][encoding]["fsize"]}MB), execution time: {data["expelliarmus"][encoding]["full_read"]:.3f}s.')

    hdf5_fp = h5py.File(hdf5_fpath)
    data["hdf5"][encoding]["full_read"] = sum(timeit.repeat(lambda: hdf5_fp["arr"][:], number=1, repeat=REPEAT))/REPEAT
    print(f'HDF5 ({data["hdf5"][encoding]["fsize"]}MB, {get_diff_perc_str(data["expelliarmus"][encoding]["fsize"], data["hdf5"][encoding]["fsize"])}), execution time: {data["hdf5"][encoding]["full_read"]:.3f}s, {get_diff_perc_str(data["expelliarmus"][encoding]["full_read"], data["hdf5"][encoding]["full_read"])}.')
    hdf5_fp.close()


    hdf5_lzf_fp = h5py.File(hdf5_lzf_fpath)
    data["hdf5_lzf"][encoding]["full_read"] = sum(timeit.repeat(lambda: hdf5_lzf_fp["arr"][:], number=1, repeat=REPEAT))/REPEAT
    print(f'HDF5 LZF ({data["hdf5_lzf"][encoding]["fsize"]}MB, {get_diff_perc_str(data["expelliarmus"][encoding]["fsize"], data["hdf5_lzf"][encoding]["fsize"])}), execution time: {data["hdf5_lzf"][encoding]["full_read"]:.3f}s, {get_diff_perc_str(data["expelliarmus"][encoding]["full_read"], data["hdf5_lzf"][encoding]["full_read"])}.')
    hdf5_lzf_fp.close()

    hdf5_gzip_fp = h5py.File(hdf5_gzip_fpath)
    data["hdf5_gzip"][encoding]["full_read"] = sum(timeit.repeat(lambda: hdf5_gzip_fp["arr"][:], number=1, repeat=REPEAT))/REPEAT
    print(f'HDF5 GZIP ({data["hdf5_gzip"][encoding]["fsize"]}MB, {get_diff_perc_str(data["expelliarmus"][encoding]["fsize"], data["hdf5_gzip"][encoding]["fsize"])}), execution time: {data["hdf5_gzip"][encoding]["full_read"]:.3f}s, {get_diff_perc_str(data["expelliarmus"][encoding]["full_read"], data["hdf5_gzip"][encoding]["full_read"])}.')
    hdf5_gzip_fp.close()

    data["numpy"][encoding]["full_read"] = sum(timeit.repeat(lambda: np.load(np_fpath), number=1, repeat=REPEAT))/REPEAT
    print(f'NumPy ({data["numpy"][encoding]["fsize"]}MB, {get_diff_perc_str(data["expelliarmus"][encoding]["fsize"], data["numpy"][encoding]["fsize"])}), execution time: {data["numpy"][encoding]["full_read"]:.3f}s, {get_diff_perc_str(data["expelliarmus"][encoding]["full_read"], data["numpy"][encoding]["full_read"])}.')
```

    ==================================================
    Full file read
    ==================================================
    DAT (413MB), execution time: 0.563s.
    HDF5 (826MB, +100.00%), execution time: 0.445s, -20.95%.
    HDF5 LZF (316MB, -23.49%), execution time: 1.921s, +241.25%.
    HDF5 GZIP (163MB, -60.53%), execution time: 3.163s, +461.81%.
    NumPy (826MB, +100.00%), execution time: 0.246s, -56.33%.
    ==================================================
    EVT2 (157MB), execution time: 0.522s.
    HDF5 (621MB, +295.54%), execution time: 0.342s, -34.46%.
    HDF5 LZF (276MB, +75.80%), execution time: 1.612s, +209.02%.
    HDF5 GZIP (156MB, -0.64%), execution time: 2.744s, +425.97%.
    NumPy (621MB, +295.54%), execution time: 0.130s, -75.18%.
    ==================================================
    EVT3 (350MB), execution time: 2.983s.
    HDF5 (1701MB, +386.00%), execution time: 0.934s, -68.69%.
    HDF5 LZF (746MB, +113.14%), execution time: 4.079s, +36.74%.
    HDF5 GZIP (419MB, +19.71%), execution time: 7.438s, +149.34%.
    NumPy (1701MB, +386.00%), execution time: 0.835s, -72.01%.

## Contributing
Please check our documentation page for more details on contributing.
