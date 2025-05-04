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


#include "mir/input/PyGribInput.h"

#include <ostream>

#include "eckit/config/Resource.h"

#include "mir/util/Grib.h"


namespace mir::input {


static size_t buffer_size() {
    static size_t size = eckit::Resource<size_t>("$MIR_GRIB_INPUT_BUFFER_SIZE", 64 * 1024 * 1024);
    return size;
}


static long pyio_readcb(void* data, void* buf, long len) {
    auto obj      = reinterpret_cast<PyObject*>(data);
    PyObject* res = PyObject_CallMethod(obj, "read", "l", len);
    if (res == nullptr) {
        return -2;
    }

    Py_buffer read;
    if (PyObject_GetBuffer(res, &read, PyBUF_SIMPLE) < 0) {
        Py_DECREF(res);
        return -2;
    }

    Py_DECREF(res);

    long l = read.len;
    ASSERT(l <= len);
    if (PyBuffer_ToContiguous(buf, &read, l, 'C') < 0) {
        PyBuffer_Release(&read);
        return -2;
    }

    PyBuffer_Release(&read);
    return (l > 0) ? l : -1;
}


PyGribInput::PyGribInput(PyObject* obj) : obj_(obj), buffer_(buffer_size()) {
    ASSERT(obj_ != nullptr);
    Py_INCREF(obj_);
}


PyGribInput::~PyGribInput() {
    ASSERT(obj_ != nullptr);
    Py_DECREF(obj_);
}


bool PyGribInput::next() {
    ASSERT(obj_ != nullptr);

    handle(nullptr);

    size_t len = buffer_.size();
    int e      = wmo_read_any_from_stream(obj_, &pyio_readcb, buffer_, &len);

    if (e == CODES_SUCCESS) {
        ASSERT(handle(codes_handle_new_from_message(nullptr, buffer_, len)));
        return true;
    }

    if (e == CODES_END_OF_FILE) {
        return false;
    }

    if (e == CODES_BUFFER_TOO_SMALL) {
        GRIB_ERROR(e, "wmo_read_any_from_stream");
    }

    if (e == CODES_IO_PROBLEM) {
        // FIXME: propagate Python error
        GRIB_ERROR(e, "wmo_read_any_from_stream");
        return false;
    }

    GRIB_ERROR(e, "wmo_read_any_from_stream");

    return false;
}


bool PyGribInput::sameAs(const mir::input::MIRInput& other) const {
    return this == &other;
}


void PyGribInput::print(std::ostream& out) const {
    out << "PyGribInput[]";
}


}  // namespace mir::input
