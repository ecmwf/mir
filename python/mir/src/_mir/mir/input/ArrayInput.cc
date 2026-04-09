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


#include "mir/input/ArrayInput.h"

#include <algorithm>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "eckit/geo/Exceptions.h"

#include "mir/data/MIRField.h"
#include "mir/input/RawInput.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Representation.h"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>


namespace mir::input {


ArrayInput::ArrayInput(PyObject* values, PyObject* gridspec) : values_(values), gridspec_(gridspec) {
    Py_INCREF(values_);
    Py_INCREF(gridspec_);

    if (!PyUnicode_Check(gridspec_)) {
        throw std::runtime_error("ArrayInput: gridspec expects a string.");
    }

    param_ = std::make_unique<mir::param::GridSpecParametrisation>(std::string{PyUnicode_AsUTF8(gridspec_)});
    ASSERT(param_);

    if (_import_array() < 0) {
        PyErr_Print();
        throw std::runtime_error("ArrayInput: NumPy failed to import/initialise");
    }

    // Check if input is a NumPy array
    if (!PyArray_Check(values_)) {
        throw std::runtime_error("ArrayInput: must be a NumPy array.");
    }

    PyArrayObject* arr = reinterpret_cast<PyArrayObject*>(values_);

    if (!PyArray_ISCARRAY_RO(arr)) {
        throw std::runtime_error("ArrayInput: NumPy array must be contiguous and readable.");
    }

    if (PyArray_TYPE(arr) == NPY_DOUBLE) {
        auto* src = static_cast<double*>(PyArray_DATA(arr));
        auto size = static_cast<size_t>(PyArray_SIZE(arr));

        input_ = std::make_unique<mir::input::RawInput>(src, size, *param_);
        ASSERT(input_);

        return;
    }

    if (PyArray_TYPE(arr) == NPY_FLOAT) {
        converted_.resize(PyArray_SIZE(arr));
        float* src = static_cast<float*>(PyArray_DATA(arr));
        std::copy(src, src + converted_.size(), converted_.begin());

        input_ = std::make_unique<mir::input::RawInput>(converted_.data(), converted_.size(), *param_);
        ASSERT(input_);

        return;
    }

    throw std::runtime_error("ArrayInput: NumPy array must be float32 or float64.");
}


ArrayInput::~ArrayInput() {
    Py_DECREF(values_);
    Py_DECREF(gridspec_);
}


mir::data::MIRField ArrayInput::field() const {
    return input_->field();
}


void ArrayInput::print(std::ostream& out) const {
    out << "ArrayInput[" << *input_ << "]";
}


}  // namespace mir::input
