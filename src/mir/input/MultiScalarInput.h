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


#ifndef mir_input_MultiScalarInput_h
#define mir_input_MultiScalarInput_h

#include <vector>
#include "mir/input/MIRInput.h"


namespace mir {
namespace output {
class MultiScalarOutput;
}
}  // namespace mir


namespace mir {
namespace input {


class MultiScalarInput : public MIRInput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    MultiScalarInput();

    // -- Destructor

    virtual ~MultiScalarInput() override;  // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void appendScalarInput(MIRInput*);

    // -- Overridden methods

    virtual size_t dimensions() const;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    std::vector<MIRInput*> components_;

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

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend class output::MultiScalarOutput;
};


}  // namespace input
}  // namespace mir


#endif
