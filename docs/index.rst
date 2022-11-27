.. image:: _static/Logo.png
  
**Decoding DVS binary data**
----------------------------

Expelliarmus makes it easy to decode binary formats from event cameras directly into NumPy structured arrays. The following formats are supported at the moment:

- `DAT <https://docs.prophesee.ai/stable/data/file_formats/dat.html>`_
- `EVT2 <https://docs.prophesee.ai/stable/data/encoding_formats/evt2.html>`_
- `EVT3 <https://docs.prophesee.ai/stable/data/encoding_formats/evt3.html>`_ 

You can install the library through `pip`::
    
    pip install expelliarmus

The package is tested on Windows, MacOS and Linux. 

Check out the :doc:`getting started <quickstart>` section for some instructions on how to use this library! 

If you're interested in benchmarks, we also provide those in :doc:`benchmarks <benchmarking/index>`.

.. toctree::
    :hidden:

    quickstart
    benchmarking/index
    about/index
