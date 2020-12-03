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


#ifndef mir_output_EmptyOutput_h
#define mir_output_EmptyOutput_h

#include "mir/output/MIROutput.h"


namespace mir {
namespace output {


class EmptyOutput : public MIROutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    EmptyOutput();
    EmptyOutput(const ::std::string& path);

    // -- Destructor

    ~EmptyOutput();  // Change to virtual if base class

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
    // None

    // -- Methods
    // None

    // -- Overridden methods

    // From MIROutput
    virtual size_t copy(const param::MIRParametrisation&, context::Context&);  // No interpolation performed
    virtual size_t save(const param::MIRParametrisation&, context::Context&);
    virtual bool sameAs(const MIROutput&) const override;
    virtual bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const;
    virtual bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const;
    virtual void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const EmptyOutput& p)
    // { p.print(s); return s; }
};


}  // namespace output
}  // namespace mir


#endif
