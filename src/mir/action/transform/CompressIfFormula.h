/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#ifndef mir_action_transform_CompressIfFormula_h
#define mir_action_transform_CompressIfFormula_h

#include "mir/action/transform/CompressIf.h"

#include "eckit/memory/ScopedPtr.h"
#include "mir/util/Formula.h"


namespace mir {
namespace action {
namespace transform {


class CompressIfFormula : public CompressIf {
public:

    // -- Exceptions
    // None

    // -- Contructors

    CompressIfFormula(const param::MIRParametrisation&);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    bool operator()(const util::BoundingBox& cropping) const;

    void print(std::ostream& out) const;

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

    eckit::ScopedPtr<util::Formula> formula_;

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


}  // namespace transform
}  // namespace action
}  // namespace mir


#endif

