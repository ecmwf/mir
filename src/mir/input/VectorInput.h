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


#ifndef mir_input_VectorInput_h
#define mir_input_VectorInput_h

#include "mir/input/MIRInput.h"


namespace mir {
namespace output {
class VectorOutput;
}
}  // namespace mir


namespace mir {
namespace input {


class VectorInput : public MIRInput {
public:
    // -- Exceptions
    // None

    // -- Contructors

    VectorInput(MIRInput& component1, MIRInput& component2);

    // -- Destructor

    virtual ~VectorInput();  // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    MIRInput& component1_;
    MIRInput& component2_;

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
    // None

    // -- Methods
    // None

    // -- Overridden methods

    // From MIRInput
    virtual const param::MIRParametrisation& parametrisation(size_t which) const;
    virtual data::MIRField field() const;
    virtual bool next();
    virtual bool sameAs(const MIRInput& other) const;
    virtual void print(std::ostream& out) const;
    virtual grib_handle* gribHandle(size_t which = 0) const;
    virtual size_t dimensions() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend class output::VectorOutput;

    // friend ostream& operator<<(ostream& s,const VectorInput& p)
    //  { p.print(s); return s; }
};


}  // namespace input
}  // namespace mir


#endif
