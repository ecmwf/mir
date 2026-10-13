cimport eckit_defs as eckit
cimport mir_defs as mir
from libcpp cimport bool
from libcpp.string cimport string
from libcpp.vector cimport vector


cdef extern from "mir/api/MIRJob.h" namespace "mir::api":
    cdef cppclass MIRJob:
        MIRJob& set(string, string) except +
        MIRJob& set(string, bool) except +
        MIRJob& set(string, int) except +
        MIRJob& set(string, double) except +
        MIRJob& set(string, double, double) except +
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


cdef extern from "mir/appendix.h" namespace "mir::appendix":
    vector[double] grid_box_areas(const string&) except +


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
        const vector[size_t] shape() const
        const string gridspec() const
        double missingValue() const


cdef extern from "mir/method/Method.h" namespace "mir::method":
    cdef cppclass Method:
        string json_str() except +
        string json_str(bool) except +
        string type() const

    cdef cppclass MethodFactory:
        @staticmethod
        const Method* make_from_string(const string&) except +


cdef extern from "mir/action/plan/Action.h" namespace "mir::action":
    cdef cppclass Action:
        string json_str() except +
        string json_str(bool) except +

    cdef cppclass ActionFactory:
        @staticmethod
        const Action* make_from_string(const string&) except +


cdef extern from * namespace "mir::python":
    """
    #include "mir/action/plan/Action.h"
    #include "mir/method/Method.h"
    namespace mir::python {
    using method::Method;
    using action::Action;
    inline bool method_equal(const Method& a, const Method& b) { return a == b; }
    inline bool action_equal(const Action& a, const Action& b) { return a == b; }
    }
    """
    bool method_equal(const Method&, const Method&) except +
    bool action_equal(const Action&, const Action&) except +
