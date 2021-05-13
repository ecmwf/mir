#ifndef pymir_pyio_h
#define pymir_pyio_h

#include "Python.h"

#include "eckit/io/Buffer.h"
#include "mir/input/GribInput.h"
#include "mir/input/MIRInput.h"

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

#endif /* pymir_pyio_h */
