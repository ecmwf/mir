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

#include <iosfwd>
#include <memory>
#include <string>

#include "mir/key/intgrid/Intgrid.h"
#include "mir/key/truncation/Truncation.h"


namespace mir {
namespace action {
class ActionPlan;
}
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir::key::resol {


class Resol {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Resol(const param::MIRParametrisation&, bool forceNoIntermediateGrid);

    Resol(const Resol&) = delete;
    Resol(Resol&&)      = delete;

    // -- Destructor

    virtual ~Resol() = default;

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Resol&) = delete;
    void operator=(Resol&&)      = delete;

    // -- Methods

    virtual void prepare(action::ActionPlan&) const;
    virtual bool resultIsSpectral() const;
    const std::string& gridname() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    const param::MIRParametrisation& parametrisation_;

    // -- Methods

    virtual void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    long inputTruncation_;
    std::unique_ptr<intgrid::Intgrid> intgrid_;
    std::unique_ptr<truncation::Truncation> truncation_;

    // -- Methods

    long getTargetGaussianNumber() const;
    long getSourceGaussianNumber() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Resol& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::key::resol
