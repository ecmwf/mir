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
cimport numpy as cnp
from cython.operator cimport dereference
from libc.stdlib cimport free
from libc.stdlib cimport malloc
from libc.string cimport strdup
from libcpp.string cimport string
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
            self.argv[i] = strdup(arg)

    def __dealloc__(self):
        free(self.argv)

# Initialise eckit::Main when module is loaded
cdef init(Args args):
    eckit.Main.initialise(args.argc, args.argv)

_args = Args()
init(_args)


def home() -> str:
    return mir.LibMir.homeDir()


def cache() -> str:
    return mir.LibMir.cacheDir()


def version() -> str:
    return mir.LibMir.instance().version()


def git_sha1() -> str:
    return mir.LibMir.instance().gitsha1(40)


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
        return (<mir.ArrayOutput*> self._output).gridspec()

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
        arr = np.array(<cnp.float64_t[:size]>data_ptr, dtype=dtype)  # copy

        cdef double miss = (<mir.ArrayOutput*> self._output).missingValue()
        if not np.isnan(miss):
            arr[arr == miss] = np.nan

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
        key_str = key.replace("_", "-")

        if (
            isinstance(value, dict)
            and key_str == "grid"
            and set(value.keys()) == {"grid"}
        ):
            # flatten a nested grid key
            self.set(key, value["grid"])
        elif isinstance(value, dict) and key_str in ("grid", "interpolation"):
            # spec-like values encoded as YAML strings
            from yaml import dump
            value_str = dump(value, default_flow_style=True).strip().encode()
            self.j.set(key_str, value_str)
        elif isinstance(value, dict):
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
        return self.j.json_str()

    def __str__(self):
        return self.json

    __repr__ = __str__


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
            self._method = mir.MethodFactory.make_from_string(s)

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
        return self._method.type()

    @property
    def json(self) -> str:
        return self._method.json_str()

    def __dealloc__(self):
        del self._method
