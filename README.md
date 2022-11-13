# Expelliarmus 
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


```python
import expelliarmus
arr = expelliarmus.read_evt3(fpath="./pedestrians.raw")
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


If we would like to reduce the EVT3 file size, we can use the `cut_evt3` function to limit the recording time duration to `10ms`, for instance:


```python
nevents = expelliarmus.cut_evt3(fpath_in="./pedestrians.raw", fpath_out="./pedestrians_cut.raw", new_duration=10)
print(f"Number of events embedded in the cut file: {nevents}.") # The number of events embedded in the output file.
```

    Number of events embedded in the cut file: 540.


This can be verified by reading the new file in an array.


```python
cut_arr = expelliarmus.read_evt3(fpath="./pedestrians_cut.raw")
print(f"Length of array extracted from the cut recording: {len(cut_arr)}.")
```

    Length of array extracted from the cut recording: 540.


The files are consistent:


```python
print(f"First original sample: {arr[0]} ! First cut sample: {cut_arr[0]}.")
print(f"{nevents}th original sample: {arr[nevents-1]} ! Last cut sample: {cut_arr[-1]}.")
```

    First original sample: (5840504, 707, 297, 0) ! First cut sample: (5840504, 707, 297, 0).
    540th original sample: (5853218, 1208, 253, 0) ! Last cut sample: (5853218, 1208, 253, 0).


The time duration is, more or less, the desired one (the events are discrete, hence we have not a fine control over them).


```python
print(f"New recording duration: {((cut_arr['t'][-1] - cut_arr['t'][0])/1000):.2f} ms") 
```

    New recording duration: 12.71 ms


## Quick usage instructions

The function used to decode the binary files to arrays have the following prototype:

```python 
def read_FILE_FORMAT(fpath, buff_size=4096):
    ...
    return np_arr
```

The functions used to "cut" off files have the following prototype:
 ```python 
def cut_FILE_FORMAT(fpath_in, fpath_out, new_duration=10, buff_size=4096):
    ...
    return nevents_in_output_file
```     

More information about the arguments can be found in the source code. 

## Contributing

If you would like to contribute by proposing a bug-fix or a new feature, feel free to open a discussion on GitHub.
