# Expelliarmus 
A Python/C library for decoding DVS binary data formats to NumPy structured arrays.

## Supported formats
- DAT (Prophesee).
- EVT2 (Prophesee).
- EVT3 (Prophesee). 

## Installation 

You can install the library through `pip`:
```
pip install expelliarmus 
```

The package is tested on Windows, MacOS and Linux.

## Quickstart
Given an EVT3 file called `pedestrians.raw`, which can be dowloaded from [here](https://dataset.prophesee.ai/index.php/s/fB7xvMpE136yakl/download), we can decode it to an array in the following way. 


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

    (0, 707, 297, 0)


If we would like to reduce the EVT3 file size, we can use the `cut_evt3` function:


```python
n_events = expelliarmus.cut_evt3(fpath_in="./pedestrians.raw", fpath_out="./pedestrians_cut.raw", max_nevents=5000)
print(n_events) # The number of events embedded in the output file.
```

    5000


This can be verified by reading the new file to an array.


```python
cut_arr = expelliarmus.read_evt3(fpath="./pedestrians_cut.raw")
print(cut_arr.shape)
```

    (5000,)


The files are consistent:


```python
print(arr[0], cut_arr[0])
print(arr[4999], cut_arr[-1])
```

    (0, 707, 297, 0) (0, 707, 297, 0)
    (90266, 598, 260, 0) (90266, 598, 260, 0)

## Quick usage instructions

The function used to decode the binary files to arrays have the following prototype:

```python 
def read_FILE_FORMAT(fpath, buff_size=4096):
    ...
    return np_arr
```

The functions used to "cut" off files have the following prototype:
 ```python 
def cut_FILE_FORMAT(fpath_in, fpath_out, max_nevents=1000, buff_size=4096):
    ...
    return nevents_in_output_file
```     

More information about the arguments can be found in the source code. 

## Contributing

If you would like to contribute by proposing a bug-fix or a new feature, feel free to open a discussion on GitHub.
