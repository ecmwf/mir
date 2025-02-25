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

#include "eckit/geo/Projection.h"
#include "eckit/geo/spec/Custom.h"

#include "mir/repres/Gridded.h"
#include "mir/util/Atlas.h"
#include "mir/util/Shape.h"
#include "mir/util/Types.h"


namespace mir::repres::regular {


class RegularGrid : public Gridded {
public:
    // -- Types

    struct LinearSpacing : protected std::vector<double> {
        using Spec = eckit::geo::Spec;

        using vector::vector;
        LinearSpacing(value_type a, value_type b, long n, bool endpoint = true);

        value_type step() const { return size() > 1 ? operator[](1) - operator[](0) : 0; }
        const Spec& spec() const;

        using vector::back;
        using vector::front;
        using vector::size;
        using vector::operator[];

        using vector::begin;
        using vector::end;

        value_type min() const { return front() < back() ? front() : back(); }
        value_type max() const { return front() > back() ? front() : back(); }

    private:
        mutable std::shared_ptr<eckit::geo::spec::Custom> spec_;
        bool endpoint_;
    };

    using Projection = eckit::geo::Projection;

    // -- Exceptions
    // None

    // -- Constructors

    RegularGrid(const param::MIRParametrisation&, Projection::Spec*);
    RegularGrid(Projection*, const util::BoundingBox&, const LinearSpacing& x, const LinearSpacing& y,
                const util::Shape&);

    RegularGrid(const RegularGrid&) = delete;
    RegularGrid(RegularGrid&&)      = delete;

    // -- Destructor

    ~RegularGrid() override;

    // -- Convertors
    // None

    // -- Operators

    RegularGrid& operator=(const RegularGrid&) = delete;
    RegularGrid& operator=(RegularGrid&&)      = delete;

    // -- Methods
    // None

    // -- Overridden methods

    Iterator* iterator() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    const Projection& projection() const;

    const LinearSpacing& x() const { return x_; }
    const LinearSpacing& y() const { return y_; }
    bool firstPointBottomLeft() const { return firstPointBottomLeft_; }

    static eckit::geo::spec::Custom* make_proj_spec(const param::MIRParametrisation&);
    static LinearSpacing linspace(double start, double step, long num, bool plus);

    // -- Overridden methods

    // from Representation
    bool sameAs(const Representation&) const override;

    bool includesNorthPole() const override;
    bool includesSouthPole() const override;
    bool isPeriodicWestEast() const override;

    void fillGrib(grib_info&) const override;
    void fillJob(api::MIRJob&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    void reorder(long scanningMode, MIRValuesVector&) const override;
    void validate(const MIRValuesVector&) const override;
    void makeName(std::ostream&) const override;
    void print(std::ostream&) const override;

    bool extendBoundingBoxOnIntersect() const override;

    ::atlas::Grid atlasGrid() const override;
    size_t numberOfPoints() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::unique_ptr<Projection> projection_;
    LinearSpacing x_;
    LinearSpacing y_;
    util::Shape shape_;
    bool xPlus_;
    bool yPlus_;
    bool firstPointBottomLeft_;

    // -- Friends
    // None
};


}  // namespace mir::repres::regular
