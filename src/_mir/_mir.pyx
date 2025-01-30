# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

from cython.operator cimport dereference
from libc.stdlib cimport malloc, free
from libcpp.string cimport string

cimport eckit_defs as eckit
cimport mir_defs as mir
cimport std_defs as std

cimport mir_pyio

# init section -- ensure libmir.so is loaded
import findlibs
from ctypes import CDLL
m = findlibs.find("mir")
CDLL(m)

# definitions
cdef class Args:
    cdef int argc
    cdef char** argv

    def __cinit__(self):
        import sys 

        self.argc = len(sys.argv)
        self.argv = <char**> malloc(self.argc * sizeof(char*))
        for i, arg in enumerate(sys.argv):
            self.argv[i] = arg
    
    def __dealloc__(self):
        free(self.argv)

# Initialise eckit::Main when module is loaded
cdef init(Args args):
    eckit.Main.initialise(args.argc, args.argv)

_args = Args()
init(_args)

def home():
    return mir.LibMir.homeDir().decode()

def cache():
    return mir.LibMir.cacheDir().decode()

cdef class MIRInput:
    cdef mir.MIRInput* _input

cdef class MIROutput:
    cdef mir.MIROutput* _output

cdef class GribFileInput(MIRInput):
    def __cinit__(self, string path):
        self._input = new mir.GribFileInput(eckit.PathName(path))
    def __dealloc__(self):
        del self._input

cdef class GribFileOutput(MIROutput):
    def __cinit__(self, string path):
        self._output = new mir.GribFileOutput(eckit.PathName(path))
    def __dealloc__(self):
        del self._output

cdef class GribMemoryInput(MIRInput):
    def __cinit__(self, const unsigned char[::1] data):
        self._input = new mir.GribMemoryInput(&data[0], data.nbytes)
    def __dealloc__(self):
        del self._input

cdef class GribMemoryOutput(MIROutput):
    def __cinit__(self, unsigned char[::1] buf):
        self._output = new mir.GribMemoryOutput(&buf[0], buf.nbytes)
    def __dealloc__(self):
        del self._output
    def __len__(self):
        return (<mir.GribMemoryOutput*>self._output).length()

cdef class GribPyIOInput(MIRInput):
    def __cinit__(self, obj):
        self._input = new mir_pyio.GribPyIOInput(obj)
    def __dealloc__(self):
        del self._input

cdef class GribPyIOOutput(MIROutput):
    def __cinit__(self, obj):
        self._output = new mir_pyio.GribPyIOOutput(obj)
    def __dealloc__(self):
        del self._output

cdef class MultiDimensionalGribFileInput(MIRInput):
    def __cinit__(self, string path, int N):
        self._input = new mir.MultiDimensionalGribFileInput(eckit.PathName(path), N)
    def __dealloc__(self):
        del self._input

cdef class Job:
    cdef mir.MIRJob j

    def __init__(self, **kwargs):
        cdef str key, value
        for key, value in kwargs.items():
            self.set(key, value)

    def set(self, string key, string value):
        self.j.set(key, value)
        return self

    # def set(self, string key, object value):
    #     cdef string v
    #     cdef double v1, v2
    #     try:
    #         v = value
    #         self.j.set(key, v)
    #     except TypeError:
    #         try:
    #             v1, v2 = value
    #             self.j.set(key, v1, v2)
    #         except (TypeError, ValueError):
    #             raise ValueError('Invalid value: %s' % value)
    #     return self

    def execute(self, input, output):
        cdef MIRInput in_
        cdef MIROutput out

        if isinstance(input, MIRInput):
            in_ = input
        else:
            assert hasattr(input, "read")
            in_ = GribPyIOInput(input)

        if isinstance(output, MIROutput):
            out = output
        else:
            assert hasattr(output, "write")
            out = GribPyIOOutput(output)

        if not isinstance(in_, GribMemoryInput):
            while in_._input.next():
                self.j.execute(dereference(in_._input), dereference(out._output))
        else:
            self.j.execute(dereference(in_._input), dereference(out._output))

    # def execute(self, input, output):
    #     in_ = new mir.GribFileInput(eckit.PathName(input))
    #     out = new mir.GribFileOutput(eckit.PathName(output))
    #     try:
    #         self.j.execute(dereference(in_), dereference(out))
    #     finally:
    #         del in_, out
    #     return self

    @property
    def json(self):
        cdef std.ostringstream oss
        cdef eckit.JSON* j = new eckit.JSON(oss)
        self.j.json(dereference(j))
        del j
        return oss.to_string()

    def __str__(self):
        cdef mir.ostringstream oss
        oss << self.j
        cdef str jstr = oss.to_string()
        return jstr

    __repr__ = __str__
