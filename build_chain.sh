#!/usr/bin/bash
# building & publishing of a wheel with mir-python only, with requirements for mirlib & eckitlib etc wheels
# assumed to be executed *inside* wheelmaker docker image, or at least:
# - have access to the setup_utils python package
# - uv installed
# - manylinux-compatible compilation stack

# prepare python
uv venv --python python3.11 .venv
source .venv/bin/activate
uv pip install --upgrade setuptools build cython twine

# mir-python prereqs
# TODO remove test pypi after all done. We use the --no-cache to get latest version
uv pip install --no-cache --index-url https://test.pypi.org/simple/ eccodeslib eckit mirlib
PRF=".venv/lib/python3.11/site-packages"
export MIR_LIB_DIR="$PRF/eckitlib/lib64:$PRF/eccodeslib/lib64:$PRF/mirlib/lib64"
export MIR_INCLUDE_DIRS="$PRF/eckitlib/include:$PRF/eccodeslib/include:$PRF/mirlib/include"

# build
PYTHONPATH=/buildscripts python -m build --no-isolation --wheel .

twine check dist/*whl
# NOTE remove test pypi after all done
twine upload --repository testpypi --verbose dist/*whl
