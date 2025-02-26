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


#include "pyio.h"

#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/geo/Grid.h"
#include "eckit/log/JSON.h"

#include "mir/action/context/Context.h"
#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


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


GribPyIOInput::GribPyIOInput(PyObject* obj) : obj_(obj), buffer_(buffer_size()) {
    ASSERT(obj_ != nullptr);
    Py_INCREF(obj_);
}


GribPyIOInput::~GribPyIOInput() {
    ASSERT(obj_ != nullptr);
    Py_DECREF(obj_);
}


bool GribPyIOInput::next() {
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


bool GribPyIOInput::sameAs(const mir::input::MIRInput& other) const {
    return this == &other;
}


void GribPyIOInput::print(std::ostream& out) const {
    out << "GribPyIOInput[]";
}


GribPyIOOutput::GribPyIOOutput(PyObject* obj) : obj_(obj) {
    ASSERT(obj_ != nullptr);
    Py_INCREF(obj_);
}


GribPyIOOutput::~GribPyIOOutput() {
    ASSERT(obj_ != nullptr);
    Py_DECREF(obj_);
}


void GribPyIOOutput::out(const void* message, size_t length, bool) {
    auto buf       = const_cast<char*>(reinterpret_cast<const char*>(message));
    PyObject* view = PyMemoryView_FromMemory(buf, length, PyBUF_READ);
    ASSERT(view != nullptr);
    PyObject* res = PyObject_CallMethod(obj_, "write", "O", view);
    ASSERT(res != nullptr);
    Py_DECREF(res);
    Py_DECREF(view);
}


bool GribPyIOOutput::sameAs(const mir::output::MIROutput& other) const {
    return this == &other;
}


void GribPyIOOutput::print(std::ostream& out) const {
    out << "GribPyIOOutput[]";
}


ArrayInput::ArrayInput(PyObject* values, PyObject* gridspec) : values_(values), gridspec_(gridspec) {
    Py_INCREF(values_);
    Py_INCREF(gridspec_);

    if (PyObject_GetBuffer(values_, &buffer_, PyBUF_CONTIG_RO | PyBUF_FORMAT) == -1 ||
        strcmp(buffer_.format, "d") != 0) {
        PyBuffer_Release(&buffer_);
        throw std::runtime_error("ArrayInput: values expects a contiguous buffer of doubles.");
    }

    if (!PyUnicode_Check(gridspec_)) {
        throw std::runtime_error("ArrayInput: gridspec expects a string.");
    }

    param_ = std::make_unique<mir::param::GridSpecParametrisation>(std::string{PyUnicode_AsUTF8(gridspec_)});
    ASSERT(param_);

    input_ = std::make_unique<mir::input::RawInput>(static_cast<double*>(buffer_.buf),
                                                    static_cast<size_t>(buffer_.len / sizeof(double)), *param_);
    ASSERT(input_);
}


ArrayInput::~ArrayInput() {
    PyBuffer_Release(&buffer_);
    Py_DECREF(values_);
    Py_DECREF(gridspec_);
}


mir::data::MIRField ArrayInput::field() const {
    return input().field();
}


void ArrayInput::print(std::ostream& out) const {
    out << "ArrayInput[" << *input_ << "]";
}


const eckit::geo::Spec& ArrayOutput::gridspec() const {
    if (!gridspec_) {
        throw std::runtime_error("ArrayOutput: no metadata.");
    }

    return gridspec_->spec();
}


size_t ArrayOutput::save(const mir::param::MIRParametrisation&, mir::context::Context& ctx) {
    const auto& field = ctx.field();

    // save values
    ASSERT(field.dimensions() == 1);
    field.validate();
    values_ = field.values(0);

    // save gridspec (a hack)
    mir::api::MIRJob job;
    mir::repres::RepresentationHandle(field.representation())->fillJob(job);

    std::ostringstream spec;
    eckit::JSON j(spec);
    j << job;

    gridspec_ =
        std::make_unique<mir::param::GridSpecParametrisation>(eckit::geo::GridFactory::make_from_string(spec.str()));
    ASSERT(gridspec_);

    return 1;
}


void ArrayOutput::print(std::ostream& out) const {
    out << "ArrayOutput[#values=" << values_.size() << ",gridspec=[";
    if (gridspec_) {
        out << gridspec_->spec();
    }
    out << "]]";
}
