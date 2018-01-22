/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#ifndef mir_style_Resol_h
#define mir_style_Resol_h

#include <iosfwd>
#include <string>
#include "eckit/memory/NonCopyable.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/style/Intgrid.h"
#include "mir/style/Truncation.h"


namespace mir {
namespace action {
class ActionPlan;
}
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace style {


class Resol : public eckit::NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Resol(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Resol() {}

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual void prepare(action::ActionPlan&) const;
    virtual bool resultIsSpectral() const;

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
    eckit::ScopedPtr<Intgrid> intgrid_;
    eckit::ScopedPtr<Truncation> truncation_;

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


}  // namespace style
}  // namespace mir


#endif

