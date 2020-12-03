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


#ifndef mir_input_EmptyInput_h
#define mir_input_EmptyInput_h

#include "mir/input/MIRInput.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace input {


class EmptyInput : public MIRInput {
public:
    // -- Exceptions
    // None

    // -- Constructors
    EmptyInput();

    // -- Destructor

    virtual ~EmptyInput() override;  // Change to virtual if base class

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

    size_t calls_;
    param::SimpleParametrisation parametrisation_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual void print(std::ostream&) const override;  // Change to virtual if base class
    virtual bool sameAs(const MIRInput& other) const override;

    virtual const param::MIRParametrisation& parametrisation(size_t which) const override;
    virtual data::MIRField field() const override;

    virtual bool next() override;

    // virtual bool get(const std::string&, double&) const override;

    virtual void latitudes(std::vector<double>&) const;
    virtual void longitudes(std::vector<double>&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const EmptyInput& p)
    //  { p.print(s); return s; }
};


}  // namespace input
}  // namespace mir


#endif
