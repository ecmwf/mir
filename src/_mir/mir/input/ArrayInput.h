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


namespace mir::input {


class ArrayInput : public MIRInput {
public:
    ArrayInput(PyObject* values, PyObject* gridspec);
    ~ArrayInput() override;

    bool next() override { return input().next(); }
    bool sameAs(const MIRInput& other) const override { return input().sameAs(other); }
    void print(std::ostream& out) const override;

    const param::MIRParametrisation& parametrisation(size_t /*which*/) const override { return *param_; }
    data::MIRField field() const override;

private:
    MIRInput& input() { return *input_; }
    const MIRInput& input() const { return *input_; }

    PyObject* values_;
    PyObject* gridspec_;
    Py_buffer buffer_;
    std::vector<double> converted_;
    std::unique_ptr<RawInput> input_;
    std::unique_ptr<param::MIRParametrisation> param_;
};


}  // namespace mir::input
