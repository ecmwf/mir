# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


from ctypes import CDLL

# init section -- ensure libmir.so is loaded, utilizing findlibs instead of relying on rpath
import findlibs

m = findlibs.find("mir")
CDLL(m)

# import mir-python itself
from _mir import *
