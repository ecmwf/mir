#!/bin/bash

# python bindings are built via cmake, as part of the overall compile.sh action
# this requires numpy present -- but currently the cmake files only check
# numpy presence, dont pip install it on its own. Hence we hack the install here

uv pip install 'numpy>=2.0,<3.0' cython
