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
