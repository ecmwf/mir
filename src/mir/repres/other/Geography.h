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

#include <memory>
#include <vector>

#include "eckit/utils/Hash.h"

#include "mir/geography/GeographyInput.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"


namespace eckit {
class MD5;
}

namespace mir {
namespace repres {
namespace other {


class Geography : public Representation {
public:
    // Types

    // Geographical regions can have more than one polygon
    // Note: this should be the same type as GeoJSON::Polygons
    using Region = geography::GeographyInput::Polygons;

    // -- Exceptions
    // None

    // -- Constructors

    explicit Geography(const param::MIRParametrisation&);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    bool inRegion(const Point2&, size_t& index) const;
    void hash(eckit::MD5&) const;

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

    std::vector<Region> regions_;
    std::string geometry_;
    eckit::Hash::digest_t id_;
    const util::BoundingBox bbox_;

    // -- Methods
    // None

    // -- Overridden methods

    // From Representation
    void print(std::ostream&) const override;
    void validate(const MIRValuesVector&) const override;
    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;
    const util::BoundingBox& boundingBox() const override;
    size_t numberOfPoints() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace other
}  // namespace repres
}  // namespace mir
