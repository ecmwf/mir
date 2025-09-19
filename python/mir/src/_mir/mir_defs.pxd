# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


cimport eckit_defs as eckit
cimport mir_defs as mir
from libcpp.string cimport string
from libcpp.vector cimport vector


cdef extern from "mir/api/MIRJob.h" namespace "mir::api":
    cdef cppclass MIRJob:
        MIRJob& set(string, string)
        MIRJob& set(string, int)
        MIRJob& set(string, double)
        MIRJob& set(string, double, double)
        void execute(MIRInput, MIROutput) except +
        string json_str() const


cdef extern from "mir/config/LibMir.h" namespace "mir":
    cdef cppclass LibMir:
        @staticmethod
        string cacheDir()

        @staticmethod
        string homeDir()

        @staticmethod
        LibMir& instance()

        string version()
        string gitsha1(unsigned int n)  # n=40 for full sha1


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


cdef extern from "mir/input/GridSpecInput.h" namespace "mir::input":
    cdef cppclass GridSpecInput(MIRInput):
        GridSpecInput(string) except +


cdef extern from "mir/input/GriddefInput.h" namespace "mir::input":
    cdef cppclass GriddefInput(MIRInput):
        GriddefInput(eckit.PathName)


cdef extern from "mir/input/PyGribInput.h" namespace "mir::input":
    cdef cppclass PyGribInput(mir.MIRInput):
        PyGribInput(object)


cdef extern from "mir/input/ArrayInput.h" namespace "mir::input":
    cdef cppclass ArrayInput(mir.MIRInput):
        ArrayInput(values, gridspec) except +


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


cdef extern from "mir/output/EmptyOutput.h" namespace "mir::output":
    cdef cppclass EmptyOutput(MIROutput):
        EmptyOutput()


cdef extern from "mir/output/PyGribOutput.h" namespace "mir::output":
    cdef cppclass PyGribOutput(mir.MIROutput):
        PyGribOutput(object)


cdef extern from "mir/output/ArrayOutput.h" namespace "mir::output":
    cdef cppclass ArrayOutput(mir.MIROutput):
        ArrayOutput()
        vector[double]& values()
        vector[size_t] shape() const
        string gridspec() const


cdef extern from "mir/method/Method.h" namespace "mir::method":
    cdef cppclass Method:
        string json_str() const
        string type() const

    cdef cppclass MethodFactory:
        @staticmethod
        const Method* make_from_string(const string&) except +
