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
#include <vector>

#include "mir/input/RawInput.h"
#include "mir/output/MIROutput.h"


class ArrayInput final : public mir::input::MIRInput {
public:
    ArrayInput(PyObject* values, PyObject* gridspec);

    ~ArrayInput() override;

    bool next() override { return input().next(); }
    bool sameAs(const mir::input::MIRInput& other) const override { return input().sameAs(other); }
    void print(std::ostream& out) const override;

    const mir::param::MIRParametrisation& parametrisation(size_t /*which*/) const override { return *param_; }
    mir::data::MIRField field() const override;

private:
    mir::input::MIRInput& input() { return *input_; }
    const mir::input::MIRInput& input() const { return *input_; }

    PyObject* values_;
    PyObject* gridspec_;
    Py_buffer buffer_;
    std::vector<double> converted_;
    std::unique_ptr<mir::input::RawInput> input_;
    std::unique_ptr<mir::param::MIRParametrisation> param_;
};


class ArrayOutput final : public mir::output::MIROutput {
public:
    ArrayOutput() = default;

    std::vector<double>& values() { return values_; }
    std::vector<size_t> shape() const { return shape_; }
    std::string gridspec() const { return gridspec_; }

private:
    std::vector<double> values_;
    std::vector<size_t> shape_;
    std::string gridspec_;

    size_t save(const mir::param::MIRParametrisation&, mir::context::Context&) override;
    void print(std::ostream&) const override;

    bool sameAs(const MIROutput&) const override { return false; /*dummy*/ }
    bool sameParametrisation(const mir::param::MIRParametrisation&,
                             const mir::param::MIRParametrisation&) const override {
        return false; /*dummy*/
    }
    bool printParametrisation(std::ostream&, const mir::param::MIRParametrisation&) const override {
        return false; /*dummy*/
    }
};
