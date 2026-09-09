// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Python.h"

#include "mir/output/GribOutput.h"


namespace mir::output {


class PyGribOutput : public GribOutput {
public:
    PyGribOutput(PyObject*);
    virtual ~PyGribOutput();

private:
    PyObject* obj_;

    void out(const void* message, size_t length, bool interpolated) override;
    void print(std::ostream&) const override;
    bool sameAs(const MIROutput&) const override;
};


}  // namespace mir::output
