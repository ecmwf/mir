/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/output/PyGribOutput.h"

#include <ostream>

#include "eckit/config/Resource.h"

#include "mir/util/Exceptions.h"


namespace mir::output {


PyGribOutput::PyGribOutput(PyObject* obj) : obj_(obj) {
    ASSERT(obj_ != nullptr);
    Py_INCREF(obj_);
}


PyGribOutput::~PyGribOutput() {
    ASSERT(obj_ != nullptr);
    Py_DECREF(obj_);
}


void PyGribOutput::out(const void* message, size_t length, bool) {
    auto buf       = const_cast<char*>(reinterpret_cast<const char*>(message));
    PyObject* view = PyMemoryView_FromMemory(buf, length, PyBUF_READ);
    ASSERT(view != nullptr);
    PyObject* res = PyObject_CallMethod(obj_, "write", "O", view);
    ASSERT(res != nullptr);
    Py_DECREF(res);
    Py_DECREF(view);
}


bool PyGribOutput::sameAs(const mir::output::MIROutput& other) const {
    return this == &other;
}


void PyGribOutput::print(std::ostream& out) const {
    out << "PyGribOutput[]";
}


}  // namespace mir::output