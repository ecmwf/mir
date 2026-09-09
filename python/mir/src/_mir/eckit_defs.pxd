# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


from libcpp.string cimport string


cdef extern from "eckit/filesystem/PathName.h" namespace "eckit":
    cdef cppclass PathName:
        PathName(string)


cdef extern from "eckit/runtime/Main.h" namespace "eckit":
    cdef cppclass Main:
        @staticmethod
        void initialise(int argc, char** argv)
