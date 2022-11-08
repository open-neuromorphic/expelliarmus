# Expelliarmus 
A Python/C library for decoding DVS proprietary data formats to NumPy structured arrays.

## Supported formats
- DAT (Prophesee).
- EVT2 (Prophesee).
- EVT3 (Prophesee). 

## Usage examples
Given an EVT3 file called `pedestrians.raw`, which can be dowloaded from [here](https://dataset.prophesee.ai/index.php/s/fB7xvMpE136yakl/download) we can decode it to an array in the following way. 


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


This can be verified by reading the new file in an array.


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

