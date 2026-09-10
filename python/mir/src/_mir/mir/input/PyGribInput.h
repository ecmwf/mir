// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Python.h"

#include "eckit/io/Buffer.h"

#include "mir/input/GribInput.h"


namespace mir::input {


class PyGribInput : public GribInput {
public:
    PyGribInput(PyObject*);
    virtual ~PyGribInput();

    bool next() override;

private:
    PyObject* obj_;
    eckit::Buffer buffer_;

    void print(std::ostream&) const override;
    bool sameAs(const MIRInput&) const override;
};


}  // namespace mir::input
