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


#ifndef mir_input_GribStreamInput_h
#define mir_input_GribStreamInput_h

#include "eckit/io/Buffer.h"

#include "mir/input/GribInput.h"


namespace eckit {
class DataHandle;
}


namespace mir {
namespace input {


class GribStreamInput : public GribInput {
public:

    // -- Exceptions
    // None

    // -- Contructors

    GribStreamInput();
    GribStreamInput(size_t skip, size_t step);
    GribStreamInput(off_t offset);

    // -- Destructor

    virtual ~GribStreamInput(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    virtual bool next();

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members
    size_t skip_;
    size_t step_;
    off_t  offset_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    bool first_;
    eckit::Buffer buffer_;


    // -- Methods

    virtual eckit::DataHandle &dataHandle() = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const GribStreamInput& p)
    //  { p.print(s); return s; }

};


}  // namespace input
}  // namespace mir


#endif

