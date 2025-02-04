#!/usr/bin/bash
# (C) Copyright 2024- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.

# building & publishing of a wheel with mir-python only, with requirements for mirlib & eckitlib etc wheels
# assumed to be executed *inside* wheelmaker docker image, or at least:
# - have access to the setup_utils python package
# - uv installed
# - manylinux-compatible compilation stack

set -euo pipefail

# prepare python
rm -rf .venv
uv venv --python python3.11 .venv
source .venv/bin/activate
uv pip install --upgrade setuptools build cython twine wheel

TEST_PYPI=${TEST_PYPI:-no}
if [ "$TEST_PYPI" = "yes" ] ; then
    EXTRA_PIP="--no-cache --index-url https://test.pypi.org/simple/"
    TARGET="--repository testpypi"
else 
    EXTRA_PIP=""
    TARGET=""
fi

# mir-python prereqs
uv pip install $EXTRA_PIP eccodeslib eckit mirlib
PRF=".venv/lib/python3.11/site-packages"
if [ "$(uname)" == "Darwin" ] ; then L="lib" ; else L="lib64" ; fi
export MIR_LIB_DIR="$PRF/eckitlib/$L:$PRF/eccodeslib/$L:$PRF/mirlib/$L"
export MIR_INCLUDE_DIRS="$PRF/eckitlib/include:$PRF/eccodeslib/include:$PRF/mirlib/include"

# build
rm -rf build dist
PYTHONPATH=/buildscripts python -m build --no-isolation --wheel .

# upload
twine check dist/*whl
twine upload --verbose dist/*whl
