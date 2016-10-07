from cython.operator cimport dereference
from libc.stdlib cimport malloc, free
from libcpp.string cimport string

cimport eckit_defs as eckit
cimport mir_defs as mir
cimport std_defs as std

import sys

# Initialise eckit::Main when module is loaded
cdef init():
    cdef int argc = len(sys.argv)
    cdef char** argv = <char**> malloc(argc * sizeof(char*))
    for i, arg in enumerate(sys.argv):
        argv[i] = arg
    eckit.Main.initialise(argc, argv)
    free(argv)
init()

cdef class GribFileInput:
    cdef mir.GribFileInput* g
    def __cinit__(self, string path):
        self.g = new mir.GribFileInput(eckit.PathName(path))
    def __dealloc__(self):
        del self.g

cdef class GribFileOutput:
    cdef mir.GribFileOutput* g
    def __cinit__(self, string path):
        self.g = new mir.GribFileOutput(eckit.PathName(path))
    def __dealloc__(self):
        del self.g

cdef class MIRJob:
    cdef mir.MIRJob j

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

    def execute(self, GribFileInput input, GribFileOutput output):
        while input.g.next():
            self.j.execute(dereference(input.g), dereference(output.g))

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
        return oss.to_string()

    __repr__ = __str__
