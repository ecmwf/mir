mir-python - Cython interface to MIR
====================================

Building
--------

Requires a recent version of Cython_ and Python 3::

  export MIR_INCLUDE_DIRS=<path1>:<path2>:...
  export MIR_LIB_DIR=<path1>:<path2>:...
  python3 setup.py build_ext -i

Or from a bundle (mir-bundle, mars-bundle, pgen-bundle, ...) ::

  MIR_BUNDLE_SOURCE_DIR=<path> MIR_BUNDLE_BUILD_DIR=<path> python3 setup.py build_ext -i

This will build the ``mir`` extension module in the current directory.

.. _Cython: https://cython.org/

Example
-------

Can be found in ``example.py``.
