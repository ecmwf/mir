// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/util/Domain.h"

#include "repres.h"


namespace mir::repres::tests {


using sh::SphericalHarmonics;


MIRValuesVector coefficients(size_t T) {
    MIRValuesVector values(SphericalHarmonics::number_of_complex_coefficients(T) * 2);
    for (size_t i = 0; i < values.size(); ++i) {
        values[i] = static_cast<double>(i + 1);
    }
    return values;
}


CASE("SphericalHarmonics") {
    RepresentationHandle repres(new SphericalHarmonics(21));

    EXPECT_EQUAL(SphericalHarmonics::number_of_complex_coefficients(21), 22 * 23 / 2);
    EXPECT_EQUAL(repres->numberOfValues(), 22 * 23);
    EXPECT_EQUAL(repres->truncation(), 21);
    EXPECT_EQUAL(repres->factory(), "sh");
    EXPECT_EQUAL(repres->uniqueName(), "T21");
    EXPECT_EQUAL(to_string(*repres), "SphericalHarmonics[truncation=21]");

    EXPECT_NO_THROW(repres->validate(coefficients(21)));
    EXPECT_THROWS_AS(repres->validate(coefficients(20)), eckit::AssertionFailed);

    EXPECT_THROWS_AS(repres->sameAs(*repres), eckit::NotImplemented);


    SECTION("by parametrisation, by gridSpec") {
        param::SimpleParametrisation param;
        param.set("gridType", "sh");
        param.set("truncation", 21L);

        RepresentationHandle a(RepresentationFactory::build(param));
        RepresentationHandle b(make("{grid: T21}"));

        EXPECT_EQUAL(a->truncation(), 21);
        EXPECT_EQUAL(b->truncation(), 21);
        EXPECT_EQUAL(a->uniqueName(), b->uniqueName());
        EXPECT_EQUAL(repres->spec().str(), b->spec().str());

        param::SimpleParametrisation none;
        none.set("gridType", "sh");
        EXPECT_THROWS_AS(RepresentationFactory::build(none), eckit::AssertionFailed);
    }


    SECTION("domain (global)") {
        EXPECT(repres->isPeriodicWestEast());
        EXPECT(repres->includesNorthPole());
        EXPECT(repres->includesSouthPole());
        EXPECT(repres->domain().isGlobal());
        EXPECT(repres->isGlobal());
    }


    SECTION("comparison, (no) reordering") {
        std::string comparator;
        repres->comparison(comparator);
        EXPECT_EQUAL(comparator, "spectral");

        auto values = coefficients(21);
        auto copy   = values;
        repres->reorderToCanonical(copy);
        EXPECT(copy == values);
        repres->reorderFromCanonical(copy);
        EXPECT(copy == values);
    }


    SECTION("fillGrib") {
        grib_info info;
        repres->fillGrib(info);

        EXPECT_EQUAL(info.grid.grid_type, CODES_UTIL_GRID_SPEC_SH);
        EXPECT_EQUAL(info.grid.truncation, 21);
        EXPECT_EQUAL(info.packing.packing_type, CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX);
        EXPECT_EQUAL(info.packing.computeLaplacianOperator, 1);
        EXPECT_EQUAL(info.packing.truncateLaplacian, 1);
    }
}


CASE("SphericalHarmonics: truncation") {
    RepresentationHandle repres(new SphericalHarmonics(21));
    const auto in = coefficients(21);


    SECTION("to the same truncation (nothing to do)") {
        MIRValuesVector out;
        EXPECT(repres->truncate(21, in, out) == nullptr);
        EXPECT(out.empty());
    }


    SECTION("to a lower truncation (dropping coefficients)") {
        MIRValuesVector out;
        RepresentationHandle lower(repres->truncate(10, in, out));
        EXPECT_EQUAL(lower->truncation(), 10);
        EXPECT_EQUAL(out.size(), 11 * 12);

        // m = 0, n = 0..10 as is; m = 1 from the input n = 1 (after m = 0, n = 0..21)
        for (size_t i = 0; i < 22; ++i) {
            EXPECT_EQUAL(out[i], in[i]);
        }
        EXPECT_EQUAL(out[22], in[44]);
        EXPECT_EQUAL(out[23], in[45]);
    }


    SECTION("to a higher truncation (padding with zeros), and back") {
        MIRValuesVector lower;
        SphericalHarmonics::truncate(21, 10, in, lower);

        MIRValuesVector out;
        SphericalHarmonics::truncate(10, 21, lower, out);
        EXPECT_EQUAL(out.size(), in.size());

        for (size_t i = 0; i < 22; ++i) {
            EXPECT_EQUAL(out[i], in[i]);
        }
        for (size_t i = 22; i < 44; ++i) {
            EXPECT_EQUAL(out[i], 0.);
        }
        EXPECT_EQUAL(out[44], in[44]);

        MIRValuesVector same_truncation;
        EXPECT_THROWS_AS(SphericalHarmonics::truncate(21, 21, in, same_truncation), eckit::AssertionFailed);

        MIRValuesVector wrong_size;
        EXPECT_THROWS_AS(SphericalHarmonics::truncate(20, 10, in, wrong_size), eckit::AssertionFailed);
    }
}


CASE("SphericalHarmonics: interlaced spectra") {
    const size_t T  = 2;
    const size_t nc = SphericalHarmonics::number_of_complex_coefficients(T);
    const auto a    = coefficients(T);

    MIRValuesVector b(a.size());
    for (size_t i = 0; i < b.size(); ++i) {
        b[i] = -a[i];
    }

    MIRValuesVector interlaced(nc * 2 * 2);
    SphericalHarmonics::interlace_spectra(interlaced, a, T, nc, 0, 2);
    SphericalHarmonics::interlace_spectra(interlaced, b, T, nc, 1, 2);

    for (size_t j = 0; j < nc * 2; ++j) {
        EXPECT_EQUAL(interlaced[j * 2], a[j]);
        EXPECT_EQUAL(interlaced[j * 2 + 1], b[j]);
    }

    EXPECT_THROWS_AS(SphericalHarmonics::interlace_spectra(interlaced, coefficients(T + 1), T, nc, 0, 2),
                     exception::UserError);
    EXPECT_THROWS_AS(SphericalHarmonics::interlace_spectra(interlaced, a, T, nc, 2, 2), eckit::AssertionFailed);
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
