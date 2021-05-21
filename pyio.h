#pragma once

#include "Python.h"

#include "eckit/io/Buffer.h"
#include "mir/input/GribInput.h"
#include "mir/input/MIRInput.h"
#include "mir/output/GribOutput.h"
#include "mir/output/MIROutput.h"

class GribPyIOInput : public mir::input::GribInput {
public:
    GribPyIOInput(PyObject* obj);
    virtual ~GribPyIOInput();
    virtual bool next();
private:
    PyObject* obj_;
    eckit::Buffer buffer_;

    virtual void print(std::ostream&) const;
    virtual bool sameAs(const mir::input::MIRInput& other) const;
};

class GribPyIOOutput : public mir::output::GribOutput {
public:
    GribPyIOOutput(PyObject* obj);
    virtual ~GribPyIOOutput();
private:
    PyObject* obj_;

    void out(const void* message, size_t length, bool interpolated) override;

    virtual void print(std::ostream&) const;
    virtual bool sameAs(const mir::output::MIROutput& other) const;
};

