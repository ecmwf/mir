# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

# building & publishing of a wheel with mir-python only, with requirements for mirlib & eckitlib etc wheels
# assumed to be executed *inside* wheelmaker docker image, or at least:
# - have access to the setup_utils python package
# - uv installed
# - manylinux-compatible compilation stack
# - env var PYVERSION, eg 3.11

set -euo pipefail

# prepare python
rm -rf .venv
uv venv --python python$PYVERSION .venv
source .venv/bin/activate
uv pip install --upgrade -r ./requirements-devel.txt

TEST_PYPI=${TEST_PYPI:-no}
if [ "$TEST_PYPI" = "yes" ] ; then
    EXTRA_PIP="--no-cache --index-url https://test.pypi.org/simple/"
    TARGET="--repository testpypi"
else
    EXTRA_PIP=""
    TARGET=""
fi

# mir-python prereqs
# TODO get these from pyproject...
uv pip install --prerelease=allow $EXTRA_PIP eccodeslib eckitlib mirlib
PRF=".venv/lib/python$PYVERSION/site-packages"
if [ "$(uname)" == "Darwin" ] ; then L="lib" ; else L="lib64" ; fi
export MIR_LIB_DIR="$PRF/eckitlib/$L:$PRF/eccodeslib/$L:$PRF/mirlib/$L"
export MIR_INCLUDE_DIRS="$PRF/eckitlib/include:$PRF/eccodeslib/include:$PRF/mirlib/include"

# build
rm -rf build dist
PYTHONPATH=/buildscripts python -m build --no-isolation --wheel .

# test
uv pip install ./dist/*
# pytest tests/ # TODO re-enable after fixed
twine check dist/*whl

# upload
# NOTE we don't upload because of execution via ci-utils/wheelmaker/buildscripts/multirelease.sh, which uploads on its own
# twine upload --verbose --skip-existing dist/*whl
