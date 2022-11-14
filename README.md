![expelliarmus](Logo.png)

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
wizard = Wizard(encoding="evt3")
```

Let's cast a spell called `read(fpath)` and read [this RAW file](https://dataset.prophesee.ai/index.php/s/fB7xvMpE136yakl/download) to a structured NumPy array! 


```python
arr = wizard.read(fpath="./pedestrians.raw")
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
nevents = wizard.cut(fpath_in="./pedestrians.raw", fpath_out="./pedestrians_cut.raw", new_duration=12)
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
print(f"First original sample: {arr[0]} ! First cut sample: {cut_arr[0]}.")
print(f"{nevents}th original sample: {arr[nevents-1]} ! Last cut sample: {cut_arr[-1]}.")
print((arr[:nevents]==cut_arr[:]).all())
```

    First original sample: (5840504, 707, 297, 0) ! First cut sample: (5840504, 707, 297, 0).
    540th original sample: (5853218, 1208, 253, 0) ! Last cut sample: (5853218, 1208, 253, 0).
    True


The time duration is, more or less, the desired one (the events are discrete, hence we have not a fine control over them).


```python
print(f"New recording duration: {((cut_arr['t'][-1] - cut_arr['t'][0])/1000):.2f} ms") 
```

    New recording duration: 12.71 ms


What if you're a poor `Muggle` and you cannot cast any spells? Well, we can try to read the files one chunk at time...


```python
from expelliarmus import Muggle
muggle = Muggle(encoding="evt3")
print(f"Length of the chunk: {len(next(muggle.read_chunk(fpath='./pedestrians.raw', nevents_per_chunk=512)))}.")
```

    Length of the chunk: 512.


Let's read less events, so that we are able to visualize them


```python
print(next(muggle.read_chunk(fpath="./pedestrians.raw", nevents_per_chunk=16)))
```

    [(5848837,  610, 296, 1) (5848843,  834, 302, 1) (5848846,  593, 254, 1)
     (5848846, 1003, 298, 1) (5848859,  610, 299, 1) (5848887,  709, 306, 0)
     (5848888,  756, 292, 0) (5848895,  704, 300, 0) (5848903,  744, 169, 1)
     (5848904, 1209, 252, 0) (5848905,  709, 307, 0) (5848911,  139, 315, 0)
     (5848918,  603, 301, 1) (5848918,  708, 299, 1) (5848924,  778, 295, 1)
     (5848967,  140, 315, 0)]


## Quick usage instructions

Right now, three encodings are supported: `dat`, `evt2` and `evt3`. `Wizard` can read and cut files, while `Muggle` allows to read files in chunks when these are too large. The constructors are the following:

```python 
class Wizard(encoding, buff_size, dtype)
```
```python
class Muggle(encoding, buff_size, dtype)
```

To read a file we use the `read(fpath)` method, while to cut it the `cut(fpath_in, fpath_out, new_duration)` one from the `Wizard`.

To read in chunks from a file, you can use the `Muggle`'s `read_chunk(fpath, nevents_per_chunk)` method.

More information about the usage can be found in the source code. Sooner or later we'll publish a proper documentation! 

## Contributing

If you would like to contribute by proposing a bug-fix or a new feature, feel free to open a discussion on GitHub or to open an issue. You can even write an email!
