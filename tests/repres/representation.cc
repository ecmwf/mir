// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/latlon/ReducedLL.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/GridBox.h"
#include "mir/util/MeshGeneratorParameters.h"

#include "repres.h"


namespace mir::repres::tests {


// a representation implementing nothing but print, for the defaults of the base class
class Minimal final : public Representation {
    void print(std::ostream& out) const override { out << "Minimal[]"; }
};


CASE("Representation: defaults of the base class") {
    RepresentationHandle handle(new Minimal);
    const Representation& r = *handle;

    EXPECT_EQUAL(to_string(r), "Minimal[]");

    using exception::SeriousBug;

    MIRValuesVector values(1);
    MIRValuesVector out;
    grib_info info;
    util::MeshGeneratorParameters mesh;
    util::BoundingBox bbox;
    util::IndexMapping mapping;
    double d = 0.;

    EXPECT_THROWS_AS(r.gridBoxes(), SeriousBug);
    EXPECT_THROWS_AS(r.isPeriodicWestEast(), SeriousBug);
    EXPECT_THROWS_AS(r.includesNorthPole(), SeriousBug);
    EXPECT_THROWS_AS(r.includesSouthPole(), SeriousBug);
    EXPECT_THROWS_AS(r.validate(values), SeriousBug);
    EXPECT_THROWS_AS(r.fillGrib(info), SeriousBug);
    EXPECT_THROWS_AS(r.fillMeshGen(mesh), SeriousBug);
    EXPECT_THROWS_AS(r.intersectionOnCrop(), SeriousBug);
    EXPECT_THROWS_AS(r.croppedRepresentation(bbox), SeriousBug);
    EXPECT_THROWS_AS(r.truncate(1, values, out), SeriousBug);
    EXPECT_THROWS_AS(r.uniqueName(), SeriousBug);
    EXPECT_THROWS_AS(r.sameAs(r), SeriousBug);
    EXPECT_THROWS_AS(r.atlasGrid(), SeriousBug);
    EXPECT_THROWS_AS(r.order(), SeriousBug);
    EXPECT_THROWS_AS(r.boundingBox(), SeriousBug);
    EXPECT_THROWS_AS(r.extendBoundingBox(bbox), SeriousBug);
    EXPECT_THROWS_AS(r.crop(bbox, mapping), SeriousBug);
    EXPECT_THROWS_AS(r.truncation(), SeriousBug);
    EXPECT_THROWS_AS(r.numberOfPoints(), SeriousBug);
    EXPECT_THROWS_AS(r.numberOfValues(), SeriousBug);
    EXPECT_THROWS_AS(r.getLongestElementDiagonal(d), SeriousBug);
    EXPECT_THROWS_AS(r.frame(values, 1, 0.), SeriousBug);
    EXPECT_THROWS_AS(r.factory(), SeriousBug);
    EXPECT_THROWS_AS(r.reorderToCanonical(values), SeriousBug);
    EXPECT_THROWS_AS(r.reorderFromCanonical(values), SeriousBug);
    EXPECT_THROWS_AS(r.iterator(), SeriousBug);
    EXPECT_THROWS_AS(to_json(r), SeriousBug);

    // the domain is the base of other methods
    EXPECT_THROWS_AS(r.domain(), SeriousBug);
    EXPECT_THROWS_AS(r.geographicDomain(), SeriousBug);
    EXPECT_THROWS_AS(r.isGlobal(), SeriousBug);

    // the grid (and spec) is by fillSpec
    EXPECT_THROWS_AS(r.grid(), SeriousBug);
    EXPECT_THROWS_AS(r.spec(), SeriousBug);

    api::MIRJob job;
    EXPECT_THROWS_AS(r.fillJob(job), SeriousBug);

    data::MIRField field(handle, false, 9999.);
    EXPECT_THROWS_AS(r.globalise(field), SeriousBug);

    // nothing to compare differently
    std::string comparator = "unchanged";
    r.comparison(comparator);
    EXPECT_EQUAL(comparator, "unchanged");
}


CASE("RepresentationHandle") {
    const Representation* r = new Minimal;
    EXPECT_EQUAL(r->count(), 0);

    {
        RepresentationHandle a(r);
        EXPECT_EQUAL(r->count(), 1);

        RepresentationHandle b(a);
        EXPECT_EQUAL(r->count(), 2);
        EXPECT(static_cast<const Representation*>(b) == r);
        EXPECT(b);
    }

    // a null handle
    RepresentationHandle none(nullptr);
    EXPECT_NOT(none);

    RepresentationHandle copy(none);
    EXPECT_NOT(copy);

    // the last handle has deleted it
    RepresentationHandle last(new Minimal);
    EXPECT_EQUAL(last->count(), 1);
}


CASE("RepresentationFactory") {
    SECTION("by gridType") {
        param::SimpleParametrisation param;
        param.set("gridType", "none");

        RepresentationHandle none(RepresentationFactory::build(param));
        EXPECT_EQUAL(to_string(*none), "None[]");
        EXPECT_THROWS_AS(none->numberOfPoints(), exception::SeriousBug);
    }


    SECTION("unknown, or without gridType") {
        param::SimpleParametrisation param;
        EXPECT_THROWS_AS(RepresentationFactory::build(param), exception::SeriousBug);

        param.set("gridType", "unknown");
        EXPECT_THROWS_AS(RepresentationFactory::build(param), exception::SeriousBug);
    }


    SECTION("unsupported gridType") {
        const std::vector<std::string> unsupported{"azimuth_range",        "equatorial_azimuthal_equidistant",
                                                   "rotated_sh",           "stretched_gg",
                                                   "stretched_ll",         "stretched_rotated_gg",
                                                   "stretched_rotated_ll", "stretched_rotated_sh",
                                                   "stretched_sh"};

        for (const auto& gridType : unsupported) {
            param::SimpleParametrisation param;
            param.set("gridType", gridType);
            EXPECT_THROWS_AS(RepresentationFactory::build(param), exception::FunctionalityNotSupported);
        }
    }


    SECTION("list") {
        std::ostringstream str;
        RepresentationFactory::list(str);

        const std::vector<std::string> names{"healpix",    "irregular_latlon",  "lambert",    "none",
                                             "reduced_gg", "reduced_ll",        "regular_gg", "regular_ll",
                                             "rotated_gg", "rotated_ll",        "sh",         "unstructured_grid",
                                             "space_view", "reduced_rotated_gg"};
        for (const auto& name : names) {
            EXPECT(str.str().find(name) != std::string::npos);
        }
    }
}


CASE("Representation: common behaviour, by gridSpec") {
    struct test_t {
        std::string gridspec;
        bool global;
        bool gridBoxes;
        std::string intersectionOnCrop;
    };

    const std::string INTERSECTS = "input-intersects-output";
    const std::string CONTAINS   = "input-contains-output-check";

    const std::vector<test_t> tests{
        {"{grid: [10, 10]}", true, true, INTERSECTS},
        {"{grid: [10, 10], area: [60, -10, 30, 40]}", false, true, INTERSECTS},
        {"{grid: [10, 10], rotation: [-40, 22]}", true, false, CONTAINS},
        {"{grid: [10, 10], area: [20, -20, -20, 20], rotation: [-40, 22]}", false, false, CONTAINS},
        {"{grid: F8}", true, true, INTERSECTS},
        {"{grid: F8, area: [60, -10, 30, 40]}", false, true, INTERSECTS},
        {"{grid: F8, rotation: [-40, 22]}", true, false, CONTAINS},
        {"{grid: O8}", true, true, INTERSECTS},
        {"{grid: O8, area: [60, -10, 30, 40]}", false, true, INTERSECTS},
        {"{grid: O8, rotation: [-40, 22]}", true, true, CONTAINS},
        {"{grid: N16}", true, true, INTERSECTS},
        {"{pl: [20, 24, 24, 20]}", true, true, INTERSECTS},
        {"{type: reduced_ll, pl: [4, 8, 12, 8, 4]}", true, true, "none"},  // NOTE: ReducedLL has no JSON (see report)
        {"{grid: H4}", true, true, "none"},
        {"{grid: H4, order: nested}", true, true, "none"},
    };

    RepresentationHandle other(make("{grid: [1, 1]}"));

    for (const auto& test : tests) {
        SECTION(test.gridspec) {
            RepresentationHandle repres(make(test.gridspec));
            const auto& r = *repres;
            const auto n  = r.numberOfPoints();

            EXPECT(n > 0);
            EXPECT_EQUAL(count(r), n);
            EXPECT_EQUAL(r.numberOfValues(), n);
            EXPECT_EQUAL(r.grid().size(), n);

            // the spec describes the same grid
            std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(r.spec().str()));
            EXPECT_EQUAL(grid->size(), n);

            // values of the right size
            EXPECT_NO_THROW(r.validate(MIRValuesVector(n)));
            EXPECT_THROWS_AS(r.validate(MIRValuesVector(n + 1)), eckit::AssertionFailed);

            // comparisons
            RepresentationHandle again(make(test.gridspec));
            EXPECT(r.sameAs(*again));
            EXPECT_NOT(r.sameAs(*other));
            EXPECT_EQUAL(r.uniqueName(), again->uniqueName());
            EXPECT(r.uniqueName() != other->uniqueName());

            EXPECT(!to_string(r).empty());
            if (dynamic_cast<const latlon::ReducedLL*>(&r) == nullptr) {
                EXPECT(!to_json(r).empty());
            }

            // a job, to the same grid
            api::MIRJob job;
            r.fillJob(job);

            std::string spec;
            EXPECT(job.get("grid", spec));
            EXPECT_EQUAL(spec, r.spec().str());

            // domain
            EXPECT_EQUAL(r.isGlobal(), test.global);
            EXPECT_EQUAL(r.domain().isGlobal(), test.global);
            EXPECT(r.geographicDomain().isGlobal() || !test.global);

            // meshing
            util::MeshGeneratorParameters mesh;
            r.fillMeshGen(mesh);
            EXPECT(!mesh.meshGenerator_.empty());

            EXPECT_EQUAL(static_cast<size_t>(r.atlasGrid().size()), n);

            if (double d = 0.; r.getLongestElementDiagonal(d)) {
                EXPECT(d > 0.);
            }

            // grid boxes, covering the globe if global
            if (test.gridBoxes) {
                const auto boxes = r.gridBoxes();
                EXPECT_EQUAL(boxes.size(), n);

                double area = 0.;
                for (const auto& box : boxes) {
                    EXPECT(box.area() > 0.);
                    area += box.area();
                }

                if (test.global) {
                    EXPECT(same(area / util::Earth::area(), 1., 1e-6));
                }
            }

            EXPECT_EQUAL(r.intersectionOnCrop(), test.intersectionOnCrop);

            // gridded representations don't crop by index mapping
            util::BoundingBox bbox;
            util::IndexMapping mapping;
            EXPECT_NOT(r.crop(bbox, mapping));

            // global fields are not globalised
            if (test.global) {
                data::MIRField field(repres, false, 9999.);
                MIRValuesVector values(n, 1.);
                field.update(values, 0);

                EXPECT(r.globalise(field) == nullptr);
                EXPECT_EQUAL(field.values(0).size(), n);
            }
        }
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
