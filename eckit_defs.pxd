from libcpp.string cimport string
cimport std_defs as std

cdef extern from "eckit/filesystem/PathName.h" namespace "eckit":
    cdef cppclass PathName:
        PathName(string)

cdef extern from "eckit/log/JSON.h" namespace "eckit":
    cdef cppclass JSON:
        JSON(std.ostream&)

cdef extern from "eckit/runtime/Main.h" namespace "eckit":
    cdef cppclass Main:
        @staticmethod
        void initialise(int argc, char** argv)
