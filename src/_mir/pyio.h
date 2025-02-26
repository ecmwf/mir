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


#pragma once

#include "Python.h"

#include <memory>

#include "eckit/io/Buffer.h"

#include "mir/input/GribInput.h"
#include "mir/input/RawInput.h"
#include "mir/output/GribOutput.h"


class GribPyIOInput : public mir::input::GribInput {
public:
    GribPyIOInput(PyObject*);
    virtual ~GribPyIOInput();

    bool next() override;

private:
    PyObject* obj_;
    eckit::Buffer buffer_;

    void print(std::ostream&) const override;
    bool sameAs(const mir::input::MIRInput&) const override;
};


class GribPyIOOutput : public mir::output::GribOutput {
public:
    GribPyIOOutput(PyObject*);
    virtual ~GribPyIOOutput();

private:
    PyObject* obj_;

    void out(const void* message, size_t length, bool interpolated) override;
    void print(std::ostream&) const override;
    bool sameAs(const mir::output::MIROutput&) const override;
};


class ArrayInput final : public mir::input::MIRInput {
public:
    ArrayInput(PyObject* data, PyObject* gridspec);

    ~ArrayInput() override {
        PyBuffer_Release(&buffer_);
        Py_DECREF(data_);
        Py_DECREF(gridspec_);
    }

    bool next() override { return input().next(); }
    bool sameAs(const mir::input::MIRInput& other) const override { return input().sameAs(other); }
    void print(std::ostream& out) const override;

    const mir::param::MIRParametrisation& parametrisation(size_t /*which*/) const override { return *param_; }
    mir::data::MIRField field() const override;

private:
    mir::input::MIRInput& input() { return *input_; }
    const mir::input::MIRInput& input() const { return *input_; }

    PyObject* data_;
    PyObject* gridspec_;
    Py_buffer buffer_;
    std::unique_ptr<mir::input::RawInput> input_;
    std::unique_ptr<mir::param::MIRParametrisation> param_;
};
