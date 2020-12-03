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


#ifndef GribFileOutput_H
#define GribFileOutput_H

#include "eckit/filesystem/PathName.h"

#include "mir/output/GribStreamOutput.h"


namespace mir {
namespace output {


class GribFileOutput : public GribStreamOutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GribFileOutput(const eckit::PathName&, bool append = false);

    // -- Destructor

    ~GribFileOutput();  // Change to virtual if base class

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


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    eckit::PathName path_;
    eckit::DataHandle* handle_;
    bool append_;

    // -- Methods
    // None

    // -- Overridden methods
    // From MIROutput
    virtual bool sameAs(const MIROutput& other) const override;
    virtual void print(std::ostream&) const override;  // Change to virtual if base class

    // From GribInput

    virtual eckit::DataHandle& dataHandle() override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const GribFileOutput& p)
    //	{ p.print(s); return s; }
};


}  // namespace output
}  // namespace mir
#endif
