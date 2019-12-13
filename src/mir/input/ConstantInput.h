/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef mir_input_ConstantInput_h
#define mir_input_ConstantInput_h

#include "mir/input/ArtificialInput.h"


namespace mir {
namespace input {


class ConstantInput : public ArtificialInput {
public:
    // -- Exceptions
    // None

    // -- Contructors

    ConstantInput(const param::MIRParametrisation&);

    // -- Destructor
    // None

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
    // None

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

    double constant_;

    // -- Methods
    // None

    // -- Overridden methods

    // From MIRInput
    virtual bool sameAs(const MIRInput&) const;

    // From ArtificialInput
    virtual void print(std::ostream&) const;
    virtual data::MIRValuesVector fill(size_t) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace input
}  // namespace mir


#endif
