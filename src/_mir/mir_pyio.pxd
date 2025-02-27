# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


from libcpp.string cimport string
from libcpp.vector cimport vector

cimport mir_defs as mir


cdef extern from "pyio.h":
    cdef cppclass GribPyIOInput(mir.MIRInput):
        GribPyIOInput(object)

    cdef cppclass GribPyIOOutput(mir.MIROutput):
        GribPyIOOutput(object)

    cdef cppclass ArrayInput(mir.MIRInput):
        ArrayInput(values, gridspec)

    cdef cppclass ArrayOutput(mir.MIROutput):
        ArrayOutput()
        vector[double]& values()
        vector[size_t] shape() const
        string gridspec() const
