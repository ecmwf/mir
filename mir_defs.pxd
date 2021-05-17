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
