/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#ifndef mir_style_AutomaticTruncation_h
#define mir_style_AutomaticTruncation_h

#include <iosfwd>
#include <string>
#include "eckit/memory/NonCopyable.h"
#include "mir/param/DelayedParametrisation.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace style {


class AutomaticTruncation : public eckit::NonCopyable, public param::DelayedParametrisation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    AutomaticTruncation(const param::MIRParametrisation&);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static long getGaussianNumber(const param::MIRParametrisation&);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    const param::MIRParametrisation& parametrisation_;
    std::string mapping_;
    long truncation_;

    // -- Methods
    // None

    // -- Overridden methods

    bool get(const std::string&, long&) const;
    bool get(const std::string&, size_t&) const;
    void print(std::ostream&) const;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace style
}  // namespace mir


#endif
