mir-python - Cython interface to MIR
====================================

Building a Wheel
----------------

Run::

  BUILD_ROOT=<...> python -m build --installer uv

Which will create a python wheel in the dist directory, to be published to pypi or locally installed to a venv.
The `BUILD_ROOT` should be a directory with installs of eckit, eccodes and mir (see `build.sh` on how to obtain that).
Alternatively, just run `build-docker.sh`, which mounts this current directory, builds the required libraries,
and proceeds with the wheel building as above.

Example
-------

Can be found in ``example.py``.
