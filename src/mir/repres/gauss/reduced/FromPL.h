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

#include "mir/repres/gauss/reduced/Reduced.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace repres {
namespace gauss {
namespace reduced {


class FromPL : public Reduced {
public:
    // -- Exceptions
    // None

    // -- Constructors

    FromPL(const param::MIRParametrisation&);
    FromPL(size_t, const std::vector<long>&, const util::BoundingBox& = util::BoundingBox(),
           double angularPrecision = 0);

    // -- Destructor

    virtual ~FromPL() override = default;

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
    virtual void fill(grib_info&) const override;
    virtual void fill(api::MIRJob&) const override;
    atlas::Grid atlasGrid() const override;
    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;

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


}  // namespace reduced
}  // namespace gauss
}  // namespace repres
}  // namespace mir
