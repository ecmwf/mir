# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


cimport eckit_defs as eckit
cimport eckit_geo_defs as eckit_geo
cimport mir_defs as mir
cimport numpy as cnp
from cython.operator cimport dereference
from libc.stdlib cimport free
from libc.stdlib cimport malloc
from libc.string cimport strdup
from libcpp.string cimport string
from libcpp.utility cimport pair
from libcpp.vector cimport vector

# init section -- ensure libmir.so is loaded

from ctypes import CDLL

import findlibs

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
        if not self.argv:
            raise MemoryError("Args: failed to allocate argv")

        for i, arg in enumerate(sys.argv):
            arg_str = arg.encode()
            self.argv[i] = strdup(arg_str)

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


def version():
    return mir.LibMir.instance().version().decode()


def git_sha1():
    return mir.LibMir.instance().gitsha1(40).decode()


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


cdef class GridSpecInput(MIRInput):
    def __cinit__(self, string gridspec):
        self._input = new mir.GridSpecInput(gridspec)

    def __dealloc__(self):
        del self._input


cdef class GriddefInput(MIRInput):
    def __cinit__(self, string path):
        self._input = new mir.GriddefInput(eckit.PathName(path))

    def __dealloc__(self):
        del self._input


cdef class EmptyOutput(MIROutput):
    def __cinit__(self):
        self._output = new mir.EmptyOutput()

    def __dealloc__(self):
        del self._output


cdef class GribMemoryOutput(MIROutput):
    def __cinit__(self, unsigned char[::1] buf):
        self._output = new mir.GribMemoryOutput(&buf[0], buf.nbytes)

    def __dealloc__(self):
        del self._output

    def __len__(self):
        return (<mir.GribMemoryOutput*>self._output).length()


cdef class PyGribInput(MIRInput):
    def __cinit__(self, obj):
        self._input = new mir.PyGribInput(obj)

    def __dealloc__(self):
        del self._input


cdef class PyGribOutput(MIROutput):
    def __cinit__(self, obj):
        self._output = new mir.PyGribOutput(obj)

    def __dealloc__(self):
        del self._output


cdef class ArrayInput(MIRInput):
    def __cinit__(self, values, gridspec):
        if isinstance(gridspec, dict):
            from yaml import dump
            gridspec = dump(gridspec, default_flow_style=True)

        self._input = new mir.ArrayInput(values, gridspec)

    def __dealloc__(self):
        del self._input


cdef class ArrayOutput(MIROutput):
    def __cinit__(self):
        self._output = new mir.ArrayOutput()

    def __dealloc__(self):
        del self._output

    @property
    def spec_str(self) -> str:
        return (<mir.ArrayOutput*> self._output).gridspec().decode()

    @property
    def spec(self) -> dict:
        from yaml import safe_load
        return safe_load(self.spec_str)

    @property
    def shape(self) -> tuple:
        cdef vector[size_t] shape = (<mir.ArrayOutput*> self._output).shape()
        return tuple(shape)

    @property
    def size(self) -> int:
        return (<mir.ArrayOutput*> self._output).values().size()

    def values(self, dtype = None):
        import numpy as np

        cdef double* data_ptr = (<mir.ArrayOutput*> self._output).values().data()
        cdef Py_ssize_t size = (<mir.ArrayOutput*> self._output).values().size()
        cdef vector[size_t] shape_vec = (<mir.ArrayOutput*> self._output).shape()
        cdef tuple shape = tuple(shape_vec)

        assert dtype in (None, np.float32, np.float64)
        if dtype == np.float32:
            arr = np.array(<cnp.float64_t[:size]>data_ptr, dtype=np.float32)  # copy
        else:
            arr = np.asarray(<cnp.float64_t[:size]>data_ptr)  # no-copy

        if len(shape) > 1:
            return arr.reshape(shape)
        return arr


cdef class MultiDimensionalGribFileInput(MIRInput):
    def __cinit__(self, string path, int N):
        self._input = new mir.MultiDimensionalGribFileInput(eckit.PathName(path), N)

    def __dealloc__(self):
        del self._input


cdef class Job:
    cdef mir.MIRJob j

    def __init__(self, **kwargs):
        for key, value in kwargs.items():
            self.set(key, value)

    def set(self, key: str, value):
        cdef string key_str, value_str

        assert isinstance(key, str)
        key_str = key.replace("_", "-").encode()

        if isinstance(value, dict):
            for k, v in value.items():
                assert isinstance(k, str)
                self.set(key if k == "type" else k, v)
        elif isinstance(value, list):
            self.set(key, "/".join(str(v) for v in value))
        elif isinstance(value, str):
            value_str = value.encode()
            self.j.set(key_str, value_str)
        elif isinstance(value, int):
            self.j.set(key_str, <int>value)
        elif isinstance(value, float):
            self.j.set(key_str, <double>value)
        else:
            raise TypeError(f"Job: unsupported value type for set(): {type(value)}")

        return self

    def execute(self, input, output):
        cdef MIRInput in_
        cdef MIROutput out

        if isinstance(input, MIRInput):
            in_ = input
        else:
            assert hasattr(input, "read")
            in_ = PyGribInput(input)

        if isinstance(output, MIROutput):
            out = output
        else:
            assert hasattr(output, "write")
            out = PyGribOutput(output)

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
    def json(self) -> str:
        cdef string s = self.j.json_str()
        return s.decode()

    def __str__(self):
        return self.json

    __repr__ = __str__


cdef class Grid:
    cdef const eckit_geo.Grid* _grid

    def __cinit__(self, spec = None, **kwargs):
        assert bool(spec) != bool(kwargs)

        if kwargs or isinstance(spec, dict):
            from yaml import dump
            spec = dump(kwargs if kwargs else spec, default_flow_style=True).strip()

        try:
            assert isinstance(spec, str)
            self._grid = eckit_geo.GridFactory.make_from_string(spec.encode())

        except RuntimeError as e:
            # opportunity to do something interesting
            raise

    def __eq__(self, other) -> bool:
        if not isinstance(other, Grid):
            return NotImplemented
        return self.spec_str == other.spec_str

    def to_latlons(self):
        cdef pair[vector[double], vector[double]] latlons = self._grid.to_latlons()
        return list(latlons.first), list(latlons.second)

    def bounding_box(self) -> tuple:
        cdef const eckit_geo.BoundingBox* bbox = &self._grid.boundingBox()
        cdef double north = bbox.north
        cdef double west = bbox.west
        cdef double south = bbox.south
        cdef double east = bbox.east
        return north, west, south, east

    @property
    def spec_str(self) -> str:
        return self._grid.spec_str().decode()

    @property
    def spec(self) -> dict:
        from yaml import safe_load
        return safe_load(self.spec_str)

    @property
    def type(self) -> str:
        return self._grid.type().decode()

    @property
    def shape(self) -> tuple:
        cdef vector[size_t] shape_vec = self._grid.shape()
        return tuple(shape_vec)

    def size(self) -> int:
        return self._grid.size()

    def __len__(self) -> int:
        return self.size()

    def __dealloc__(self):
        del self._grid


cdef class Interpolation:
    cdef const mir.Method* _method

    def __cinit__(self, spec = None, **kwargs):
        # convert arguments to spec: dict without None/blank string values
        if kwargs:
            assert not spec
            spec = kwargs

        if spec is None:
            spec = dict()
        elif isinstance(spec, str):
            spec = dict(interpolation=spec) if spec.strip() else dict()
        elif not isinstance(spec, dict):
            raise TypeError(f"Interpolation: unsupported spec type: {type(spec)}")

        spec = {k.replace("_", "-"): v
                for k, v in spec.items()
                if v is not None and not (isinstance(v, str) and not v.strip())}

        try:
            from yaml import dump

            s = dump(spec, default_flow_style=True).strip() if spec else ""
            self._method = mir.MethodFactory.make_from_string(s.encode())

        except RuntimeError as e:
            # opportunity to do something interesting
            raise

    def __eq__(self, other) -> bool:
        if not isinstance(other, Interpolation):
            return NotImplemented
        return self.json == other.json

    @property
    def spec(self) -> dict:
        from yaml import safe_load
        return safe_load(self.json)

    @property
    def type(self) -> str:
        return self._method.type().decode()

    @property
    def json(self) -> str:
        cdef string s = self._method.json_str()
        return s.decode()

    def __dealloc__(self):
        del self._method
