# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

from libcpp.string cimport string
cimport eckit_defs as eckit

cdef extern from "mir/api/MIRJob.h" namespace "mir::api":
    cdef cppclass MIRJob:
        MIRJob& set(string, string)
        MIRJob& set(string, int)
        MIRJob& set(string, double)
        MIRJob& set(string, double, double)
        void execute(MIRInput, MIROutput)
        void json(eckit.JSON&)

cdef extern from "<sstream>" namespace "std" nogil:
    cdef cppclass ostringstream:
        ostringstream& operator<<(MIRJob)
        string to_string "str" () const

cdef extern from "mir/input/MIRInput.h" namespace "mir::input":
    cdef cppclass MIRInput:
        bint next() except +

cdef extern from "mir/input/GribMemoryInput.h" namespace "mir::input":
    cdef cppclass GribMemoryInput(MIRInput):
        GribMemoryInput(const void*, size_t)

cdef extern from "mir/input/GribFileInput.h" namespace "mir::input":
    cdef cppclass GribFileInput(MIRInput):
        GribFileInput(eckit.PathName)

cdef extern from "mir/input/MultiDimensionalGribFileInput.h" namespace "mir::input":
    cdef cppclass MultiDimensionalGribFileInput(MIRInput):
        MultiDimensionalGribFileInput(eckit.PathName, size_t)

cdef extern from "mir/output/MIROutput.h" namespace "mir::output":
    cdef cppclass MIROutput:
        pass

cdef extern from "mir/output/GribFileOutput.h" namespace "mir::output":
    cdef cppclass GribFileOutput(MIROutput):
        GribFileOutput(eckit.PathName)

cdef extern from "mir/output/GribMemoryOutput.h" namespace "mir::output":
    cdef cppclass GribMemoryOutput(MIROutput):
        GribMemoryOutput(void*, size_t)
        size_t length()

cdef extern from "mir/config/LibMir.h" namespace "mir":
    cdef cppclass LibMir:
        @staticmethod
        string cacheDir()
        @staticmethod
        string homeDir()
