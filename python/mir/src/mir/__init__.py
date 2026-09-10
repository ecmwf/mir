# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import findlibs

findlibs.load("mir")

from eckit.geo import Grid
from mir._mir import *

__lib_version__ = version()
__git_sha1__ = git_sha1()
