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