mir-python - Cython interface to MIR
====================================

Building a Shared Library
-------------------------
Requires a recent version of Cython_ and Python 3::

  export MIR_INCLUDE_DIRS=<path1>:<path2>:...
  export MIR_LIB_DIR=<path1>:<path2>:...
  python3 setup.py build_ext -i

Or from a bundle (mir-bundle, mars-client-bundle, pgen-bundle, ...) ::

  MIR_BUNDLE_SOURCE_DIR=<path> MIR_BUNDLE_BUILD_DIR=<path> python3 setup.py build_ext -i

This will build the ``mir`` extension module in the ``build/lib.<platform>/`` directory, so for example
``build/lib.linux-x86_64-cpython-311/mir.cpython-311-x86_64-linux-gnu.so``.

Note this ``.so`` module is not standalone, ie, it requires externally installed mir etc to be present during 
runtime. If you can't satisfy that, see the next section instead.

.. _Cython: https://cython.org/

Building a Wheel
----------------

Run::

  BUILD_ROOT=<...> python -m build --installer uv -w .

Which will create a python wheel in the dist directory, to be published to pypi or locally installed to a venv.
The ``BUILD_ROOT`` should be a directory with installs of eckit, eccodes and mir (see ``build.sh`` on how to obtain that).
Alternatively, just run ``build-docker.sh``, which mounts this current directory, builds the required libraries,
and proceeds with the wheel building as above.

Note this wheel is completely standalone, ie, it bundles in all ``.so`` dependencies such as mir. If you instead want
to use externally available mir on the system during runtime, see the previous section instead.

Example
-------

Can be found in ``example.py``. To execute it, either ``pip install`` the wheel to your venv, or put the ``mir-python``
shared library on your python path.
