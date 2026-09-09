// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Python.h"

#include <memory>
#include <vector>

#include "mir/input/MIRInput.h"


namespace mir::input {


class ArrayInput : public MIRInput {
public:
    ArrayInput(PyObject* values, PyObject* gridspec);
    ~ArrayInput() override;

    bool next() override { return input_->next(); }
    bool sameAs(const MIRInput& other) const override { return input_->sameAs(other); }
    void print(std::ostream& out) const override;

    const param::MIRParametrisation& parametrisation(size_t /*which*/) const override { return *param_; }
    data::MIRField field() const override;

private:
    PyObject* values_;
    PyObject* gridspec_;
    std::vector<double> converted_;
    std::unique_ptr<MIRInput> input_;
    std::unique_ptr<param::MIRParametrisation> param_;
};


}  // namespace mir::input
