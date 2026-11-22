// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/geo/eckit_geo_config.h"

#include "regression.h"


namespace mir::tests::regressions {


CASE("to healpix: grid-box-average") {
    // MIR-632
    // input regular_ll.2-2.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}", {{"interpolation", "grid-box-average"}, {"grid", "H2"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"grid-box-average","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"H2"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":[2,2]},{"grid":"H2"},{"grid":"H2"}])");

    EXPECT_EQUAL(get(h, "gridType"), "healpix");
    EXPECT_EQUAL(get(h, "gridName"), "H2");
    EXPECT_EQUAL(get(h, "Nside"), "2");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "48");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "45");
}


CASE("to healpix: nn") {
    // MIR-684
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:2/2}}", {{"grid", "{grid:h2}"}, {"interpolation", "nn"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":"H2"}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":[2,2]},{"grid":"H2"},{"grid":"H2"}])");

    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "48");
}


CASE("to healpix: regrid") {
    // MIR-632
    // input regular_ll.2-2.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}", {{"grid", "H2"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"H2"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":[2,2]},{"grid":"H2"},{"grid":"H2"}])");

    EXPECT_EQUAL(get(h, "gridType"), "healpix");
    EXPECT_EQUAL(get(h, "gridName"), "H2");
    EXPECT_EQUAL(get(h, "Nside"), "2");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "48");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "45");
}


CASE("to lambert_azimuthal_equal_area: regrid") {
    // input regular_ll.2-2.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
        {{"grid",
          "{projection: {type: laea, lat_0: 52, lon_0: 10, reverse: true}, "
          "x: [-1810621.83647687, 3184378.16352313, 5000], y: [2283408.83487739, -2461591.16512261, -5000]}"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"projection":{"lat_0":52,"lon_0":10,"reverse":true,"type":"laea"},"x":[-1810621.83647687,3184378.16352313,5000],"y":[2283408.83487739,-2461591.16512261,-5000]}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":[2,2]},{"projection":{"lat_0":52,"lon_0":10,"reverse":true,"type":"laea"},"x":[-1810621.83647687,3184378.16352313,5000],"y":[2283408.83487739,-2461591.16512261,-5000]},{"projection":{"lat_0":52,"lon_0":10,"reverse":true,"type":"laea"},"x":[-1810621.83647687,3184378.16352313,5000],"y":[2283408.83487739,-2461591.16512261,-5000]}])");

    EXPECT_EQUAL(get(h, "gridType"), "lambert_azimuthal_equal_area");
}


CASE("to reduced_gg: area") {
    SECTION("001") {
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "37.6025/-114.8907/27.7626/-105.1875"}, {"grid", "O32"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[37.6025,-114.891,27.7626,-105.188],"grid":"O32"},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"area":[34.8825209937735,-114.230769230769,29.3013596217627,-107.307692307692],"grid":"O32"},{"area":[34.8825209937735,-114.230769230769,29.3013596217627,-107.307692307692],"grid":"O32"}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "34.883");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "29.301");
    }

    SECTION("002") {
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "37.6025/-114.8907/27.7626/-105.1875"},
             {"grid", "O32"},
             {"dont-compress-plan", ""},
             {"edition", "1"}},
            R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"O32"},{"type":"crop.area","area":[37.6025,-114.891,27.7626,-105.188]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"grid":"O32"},{"area":[34.8825209937735,-114.230769230769,29.3013596217627,-107.307692307692],"grid":"O32"},{"area":[34.8825209937735,-114.230769230769,29.3013596217627,-107.307692307692],"grid":"O32"}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "34.883");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "29.301");
    }

    SECTION("003") {
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "37.6025/-114.8915/27.7626/-105.188"}, {"grid", "O32"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[37.6025,-114.891,27.7626,-105.188],"grid":"O32"},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"area":[34.8825209937735,-114.230769230769,29.3013596217627,-107.307692307692],"grid":"O32"},{"area":[34.8825209937735,-114.230769230769,29.3013596217627,-107.307692307692],"grid":"O32"}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "34.883");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "29.301");
    }

    SECTION("004") {
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "37.6025/-114.8915/27.7626/-105.188"},
             {"grid", "O32"},
             {"dont-compress-plan", ""},
             {"edition", "1"}},
            R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"O32"},{"type":"crop.area","area":[37.6025,-114.891,27.7626,-105.188]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"grid":"O32"},{"area":[34.8825209937735,-114.230769230769,29.3013596217627,-107.307692307692],"grid":"O32"},{"area":[34.8825209937735,-114.230769230769,29.3013596217627,-107.307692307692],"grid":"O32"}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "34.883");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "29.301");
    }
}


CASE("to reduced_gg: grid-box-average") {
    // input regular_ll.2-2.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}", {{"grid", "O64"}, {"interpolation", "grid-box-average"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"grid-box-average","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"O64"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":[2,2]},{"grid":"O64"},{"grid":"O64"}])");

    EXPECT_EQUAL(get(h, "gridName"), "O64");
}


CASE("to reduced_gg: k-nearest-neighbours-nclosest-or-nearest") {
    // input regular_ll.2-2.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
        {{"grid", "N160"},
         {"interpolation", "k-nearest-neighbours"},
         {"nearest-method", "nclosest-or-nearest"},
         {"nclosest", "10"},
         {"distance", "100000."},
         {"distance-weighting", "gaussian"},
         {"distance-weighting-gaussian-stddev", "10000."},
         {"non-linear", "missing-if-all-missing"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"k-nearest-neighbours","distance-tolerance":1,"distance-weighting":"gaussian","distance-weighting-gaussian-stddev":10000,"lsm-weight-adjustment":0.2,"nclosest":10,"nearest-method":"nclosest-or-nearest","non-linear":["missing-if-all-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"N160"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":[2,2]},{"grid":"N160"},{"grid":"N160"}])");

    EXPECT_EQUAL(get(h, "gridName"), "N160");
}


CASE("to reduced_gg: nearest-neighbour-area") {
    // input regular_ll.2-2.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
        {{"grid", "O32"}, {"interpolation", "nearest-neighbour"}, {"area", "20/-40/-20/40"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"nearest-neighbour","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[20,-40,-20,40],"grid":"O32"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":[2,2]},{"area":[18.1389709902393,-40,-18.1389709902393,40],"grid":"O32"},{"area":[18.1389709902393,-40,-18.1389709902393,40],"grid":"O32"}])");

    EXPECT_EQUAL(get(h, "gridName"), "O32");
}


CASE("to reduced_gg: nn") {
    SECTION("001") {
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"grid", "O32"}, {"interpolation", "nn"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2namedgrid","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"O32"},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"grid":"O32"},{"grid":"O32"}])");

        EXPECT_EQUAL(get(h, "gridName"), "O32");
    }

    SECTION("002") {
        // input regular_ll.2-2.grib2, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}", {{"grid", "O32"}, {"interpolation", "nn"}},
            R"([{"type":"interpolate.grid2namedgrid","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"O32"},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":[2,2]},{"grid":"O32"},{"grid":"O32"}])");

        EXPECT_EQUAL(get(h, "gridName"), "O32");
    }

    SECTION("003") {
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{type:arakawa_c_um,grid:n96,arrangement:T}}",
            {{"grid", "{grid:o96}"}, {"interpolation", "nn"}},
            R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":"O96"}},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":[1.875,1.25],"order":"i+j+","reference":[0.9375,0.625]},{"grid":"O96"},{"grid":"O96"}])");

        EXPECT_EQUAL(get(h, "numberOfDataPoints"), "40320");
        EXPECT_EQUAL(get(h, "gridType"), "reduced_gg");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "89.2842");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "0");
    }

    SECTION("004") {
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{type:arakawa_c_um,grid:n96,arrangement:V}}",
            {{"grid", "{grid:o96}"}, {"interpolation", "nn"}},
            R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":"O96"}},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":[1.875,1.25],"order":"i+j+","reference":[0.9375,0]},{"grid":"O96"},{"grid":"O96"}])");

        EXPECT_EQUAL(get(h, "numberOfDataPoints"), "40320");
        EXPECT_EQUAL(get(h, "gridType"), "reduced_gg");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "89.2842");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "0");
    }
}


CASE("to reduced_gg: structured-bicubic") {
    // input regular_ll.2-2.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}", {{"grid", "O160"}, {"interpolation", "structured-bicubic"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"structured-bicubic","interpolation-matrix-free":true,"grid":"O160"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":[2,2]},{"grid":"O160"},{"grid":"O160"}])");

    EXPECT_EQUAL(get(h, "gridName"), "O160");
}


CASE("to reduced_gg: structured-bilinear") {
    // input regular_ll.2-2.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
        {{"grid", "O320"}, {"interpolation", "structured-bilinear"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"structured-bilinear","interpolation-matrix-free":true,"grid":"O320"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":[2,2]},{"grid":"O320"},{"grid":"O320"}])");

    EXPECT_EQUAL(get(h, "gridName"), "O320");
}


CASE("to regular_gg: area") {
    // MIR-248
    // input regular_ll.2-2.grib1, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
        {{"grid", "f1280"}, {"area", "58.5940228153455/16.171875/58.5940228153455/16.2421875"}, {"edition", "1"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[58.594,16.1719,58.594,16.2422],"grid":"F1280"},{"type":"save","packing":"simple","edition":1}])",
        R"([{"grid":[2,2]},{"area":[58.5940228153455,16.171875,58.5940228153455,16.2421875],"grid":"F1280"},{"area":[58.5940228153455,16.171875,58.5940228153455,16.2421875],"grid":"F1280"}])");

    EXPECT_EQUAL(get(h, "Ni"), "2");
    EXPECT_EQUAL(get(h, "Nj"), "1");
    EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "58.595");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "16.171");
    EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "58.594");
    EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "16.243");
}


CASE("to regular_ll: area") {
    SECTION("001") {
        // Cray Broadwell arch. precision
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "70/-77/-7/7"}, {"grid", "0.7/0.7"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[70,-77,-7,7],"grid":[0.7,0.7],"area":[89.6,-77,-89.6,282.8]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"area":[70,-77,-7,7],"grid":[0.7,0.7]},{"area":[70,-77,-7,7],"grid":[0.7,0.7]}])");

        EXPECT_EQUAL(get(h, "Ni"), "121");
        EXPECT_EQUAL(get(h, "Nj"), "111");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "70");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-77");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-7");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "7");
    }

    SECTION("002") {
        // Cray Broadwell arch. precision
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "12/-12/-1.2/1.2"}, {"grid", "1.2/1.2"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[12,-12,-1.2,1.2],"grid":[1.2,1.2],"area":[90,0,-90,358.8]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"area":[12,-12,-1.2,1.2],"grid":[1.2,1.2]},{"area":[12,-12,-1.2,1.2],"grid":[1.2,1.2]}])");

        EXPECT_EQUAL(get(h, "Ni"), "12");
        EXPECT_EQUAL(get(h, "Nj"), "12");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "12");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-12");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-1.2");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "1.2");
    }

    SECTION("003") {
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "90/-999/-90/-999"}, {"grid", "9/9"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[90,-999,-90,-999],"grid":[9,9],"area":[90,0,-90,351]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"area":[90,-999,-90,-999],"grid":[9,9]},{"area":[90,-999,-90,-999],"grid":[9,9]}])");

        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-999");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "-999");
        EXPECT_EQUAL(get(h, "Ni"), "1");
    }

    SECTION("004") {
        // MIR-174
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h =
            run("{artificialInput:gridspec,gridSpec:{grid:[2,2]}}", {{"area", "88/-178/-88/180"}, {"edition", "1"}},
                R"([{"type":"crop.area","area":[88,-178,-88,180]},{"type":"save","packing":"simple","edition":1}])",
                R"([{"grid":[2,2]},{"area":[88,-178,-88,182],"grid":[2,2]},{"area":[88,-178,-88,182],"grid":[2,2]}])");

        EXPECT_EQUAL(get(h, "Ni"), "180");
        EXPECT_EQUAL(get(h, "Nj"), "89");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "88");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-178");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-88");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "180");
    }

    SECTION("005") {
        // MIR-174
        // input regular_ll.2-2.grib2, as its grid
        auto h =
            run("{artificialInput:gridspec,gridSpec:{grid:[2,2]}}", {{"area", "88/-178/-88/180"}},
                R"([{"type":"crop.area","area":[88,-178,-88,180]},{"type":"save","packing":"ccsds","edition":2}])",
                R"([{"grid":[2,2]},{"area":[88,-178,-88,182],"grid":[2,2]},{"area":[88,-178,-88,182],"grid":[2,2]}])");

        EXPECT_EQUAL(get(h, "Ni"), "180");
        EXPECT_EQUAL(get(h, "Nj"), "89");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "88");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "182");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-88");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "180");
    }

    SECTION("006") {
        // MIR-180
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "84/-165/23.9/165"}, {"grid", "0.1/0.1"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[84,-165,23.9,165],"grid":[0.1,0.1],"area":[90,0,-90,359.9]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"area":[84,-165,23.9,165],"grid":[0.1,0.1]},{"area":[84,-165,23.9,165],"grid":[0.1,0.1]}])");

        EXPECT_EQUAL(get(h, "Ni"), "3301");
        EXPECT_EQUAL(get(h, "Nj"), "602");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "84");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-165");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "23.9");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "165");
    }

    SECTION("007") {
        // MIR-180
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "84.06/-165.06/24.03/165.06"}, {"grid", "0.09/0.09"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[84.06,-165.06,24.03,165.06],"grid":[0.09,0.09],"area":[90,0,-90,359.91]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"area":[84.06,-165.06,24.03,165.06],"grid":[0.09,0.09]},{"area":[84.06,-165.06,24.03,165.06],"grid":[0.09,0.09]}])");

        EXPECT_EQUAL(get(h, "Ni"), "3669");
        EXPECT_EQUAL(get(h, "Nj"), "668");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "84.06");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-165.06");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "24.03");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "165.06");
    }

    SECTION("008") {
        // MIR-180
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "45/23.85/32.94/47.97"}, {"grid", "0.09/0.09"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[45,23.85,32.94,47.97],"grid":[0.09,0.09],"area":[90,0,-90,359.91]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"area":[45,23.85,32.94,47.97],"grid":[0.09,0.09]},{"area":[45,23.85,32.94,47.97],"grid":[0.09,0.09]}])");

        EXPECT_EQUAL(get(h, "Ni"), "269");
        EXPECT_EQUAL(get(h, "Nj"), "135");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "45");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "23.85");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "32.94");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "47.97");
    }

    SECTION("009") {
        // MIR-180
        // input regular_ll.2-2.grib2, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}", {{"area", "84/-165/23.9/165"}, {"grid", "0.1/0.1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[84,-165,23.9,165],"grid":[0.1,0.1],"area":[90,0,-90,359.9]},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":[2,2]},{"area":[84,-165,23.9,165],"grid":[0.1,0.1]},{"area":[84,-165,23.9,165],"grid":[0.1,0.1]}])");

        EXPECT_EQUAL(get(h, "Ni"), "3301");
        EXPECT_EQUAL(get(h, "Nj"), "602");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "84");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "195");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "23.9");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "165");
    }

    SECTION("010") {
        // MIR-180
        // input regular_ll.2-2.grib2, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "84.06/-165.06/24.03/165.06"}, {"grid", "0.09/0.09"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[84.06,-165.06,24.03,165.06],"grid":[0.09,0.09],"area":[90,0,-90,359.91]},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":[2,2]},{"area":[84.06,-165.06,24.03,165.06],"grid":[0.09,0.09]},{"area":[84.06,-165.06,24.03,165.06],"grid":[0.09,0.09]}])");

        EXPECT_EQUAL(get(h, "Ni"), "3669");
        EXPECT_EQUAL(get(h, "Nj"), "668");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "84.06");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "194.94");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "24.03");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "165.06");
    }

    SECTION("011") {
        // MIR-180
        // input regular_ll.2-2.grib2, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}", {{"area", "76.0/-14.4/49.6/44.8"}, {"grid", "0.8/0.8"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[76,-14.4,49.6,44.8],"grid":[0.8,0.8],"area":[89.6,0,-89.6,359.2]},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":[2,2]},{"area":[76,-14.4,49.6,44.8],"grid":[0.8,0.8]},{"area":[76,-14.4,49.6,44.8],"grid":[0.8,0.8]}])");

        EXPECT_EQUAL(get(h, "Ni"), "75");
        EXPECT_EQUAL(get(h, "Nj"), "34");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "76");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "345.6");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "49.6");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "44.8");
    }

    SECTION("012") {
        // MIR-180
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "15/279.9/-19.95/318"}, {"grid", "0.15/0.15"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[15,279.9,-19.95,318],"grid":[0.15,0.15],"area":[90,0,-90,359.85]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"area":[15,279.9,-19.95,318],"grid":[0.15,0.15]},{"area":[15,279.9,-19.95,318],"grid":[0.15,0.15]}])");

        EXPECT_EQUAL(get(h, "Ni"), "255");
        EXPECT_EQUAL(get(h, "Nj"), "234");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "15");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "279.9");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-19.95");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "318");
    }

    SECTION("013") {
        // MIR-180
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "14.94/-209.97/-70.47/5.22"}, {"grid", "0.09/0.09"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[14.94,-209.97,-70.47,5.22],"grid":[0.09,0.09],"area":[90,0,-90,359.91]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"area":[14.94,-209.97,-70.47,5.22],"grid":[0.09,0.09]},{"area":[14.94,-209.97,-70.47,5.22],"grid":[0.09,0.09]}])");

        EXPECT_EQUAL(get(h, "Ni"), "2392");
        EXPECT_EQUAL(get(h, "Nj"), "950");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "14.94");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-70.47");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-209.97");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "5.22");
    }

    SECTION("014") {
        // MIR-203
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"area", "33.075/241.85/5.95/284.025"}, {"grid", "0.175/0.175"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[33.075,241.85,5.95,284.025],"grid":[0.175,0.175],"area":[89.95,241.85,-89.95,601.825]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"area":[33.075,241.85,5.95,284.025],"grid":[0.175,0.175]},{"area":[33.075,241.85,5.95,284.025],"grid":[0.175,0.175]}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "33.075");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "241.85");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "5.95");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "284.025");
    }

    SECTION("015") {
        // MIR-218
        // test projection failure on points on domain edges
        // input regular_ll.0.5-0.5.europe.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{area:[73.5,-27,33,45],grid:[0.5,0.5]}}",
            {{"area", "50/-12/34/7"}, {"grid", "0.125/0.125"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[50,-12,34,7],"grid":[0.125,0.125],"area":[90,0,-90,359.875]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"area":[73.5,-27,33,45],"grid":[0.5,0.5]},{"area":[50,-12,34,7],"grid":[0.125,0.125]},{"area":[50,-12,34,7],"grid":[0.125,0.125]}])");

        EXPECT_EQUAL(get(h, "Ni"), "153");
        EXPECT_EQUAL(get(h, "Nj"), "129");
        EXPECT_EQUAL(get(h, "iDirectionIncrementInDegrees"), "0.125");
        EXPECT_EQUAL(get(h, "jDirectionIncrementInDegrees"), "0.125");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-12");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "7");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "50");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "34");
    }

    SECTION("016") {
        // MIR-218
        // test projection failure on points on domain edges
        // input regular_ll.0.2-0.2.europe.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{area:[73,-27,33,45],grid:[0.2,0.2]}}",
            {{"area", "50/-12/34/7"}, {"grid", "0.125/0.125"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[50,-12,34,7],"grid":[0.125,0.125],"area":[90,0,-90,359.875]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"area":[73,-27,33,45],"grid":[0.2,0.2]},{"area":[50,-12,34,7],"grid":[0.125,0.125]},{"area":[50,-12,34,7],"grid":[0.125,0.125]}])");

        EXPECT_EQUAL(get(h, "Ni"), "153");
        EXPECT_EQUAL(get(h, "Nj"), "129");
        EXPECT_EQUAL(get(h, "iDirectionIncrementInDegrees"), "0.125");
        EXPECT_EQUAL(get(h, "jDirectionIncrementInDegrees"), "0.125");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-12");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "7");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "50");
    }

    SECTION("017") {
        // MIR-218
        // test projection failure on points on domain edges
        // input regular_ll.0.1-0.1.europe.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{area:[73,-27,33,45],grid:[0.1,0.1]}}",
            {{"area", "50/-12/34/7"}, {"grid", "0.1/0.1"}, {"edition", "1"}},
            R"([{"type":"crop.area","area":[50,-12,34,7]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"area":[73,-27,33,45],"grid":[0.1,0.1]},{"area":[50,-12,34,7],"grid":[0.1,0.1]},{"area":[50,-12,34,7],"grid":[0.1,0.1]}])");

        EXPECT_EQUAL(get(h, "iDirectionIncrementInDegrees"), "0.1");
        EXPECT_EQUAL(get(h, "jDirectionIncrementInDegrees"), "0.1");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-12");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "7");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "50");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "34");
    }
}


CASE("to regular_ll: area-none") {
    // MIR-586
    // input regular_ll.2-2.grib1, as its grid (GRIB1)
    auto h = run("{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
                 {{"area", "11/-1/-1/11"}, {"interpolation", "none"}, {"edition", "1"}},
                 R"([{"type":"crop.area","area":[11,-1,-1,11]},{"type":"save","packing":"simple","edition":1}])",
                 R"([{"grid":[2,2]},{"area":[10,0,0,10],"grid":[2,2]},{"area":[10,0,0,10],"grid":[2,2]}])");

    EXPECT_EQUAL(get(h, "gridType"), "regular_ll");
    EXPECT_EQUAL(get(h, "Ni"), "6");
    EXPECT_EQUAL(get(h, "Nj"), "6");
    EXPECT_EQUAL(get(h, "iDirectionIncrementInDegrees"), "2");
    EXPECT_EQUAL(get(h, "jDirectionIncrementInDegrees"), "2");
    EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "10");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "0");
    EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "0");
    EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "10");
}


CASE("to regular_ll: nn") {
    SECTION("001") {
        // MIR-684
        // input regular_ll.2-2.grib2, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}", {{"grid", "{grid:3/3}"}, {"interpolation", "nn"}},
            R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":[3,3]}},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":[2,2]},{"grid":[3,3]},{"grid":[3,3]}])");

        EXPECT_EQUAL(get(h, "numberOfDataPoints"), "7320");
    }

    SECTION("002") {
        // input regular_ll.2-2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
            {{"grid", "{grid:5/5,rotation:[-90,360]}"}, {"interpolation", "nn"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":[5,5]}},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":[2,2]},{"grid":[5,5]},{"grid":[5,5]}])");

        EXPECT_EQUAL(get(h, "gridType"), "regular_ll");
        EXPECT_EQUAL(get(h, "Ni"), "72");
        EXPECT_EQUAL(get(h, "Nj"), "37");
        EXPECT_EQUAL(get(h, "numberOfDataPoints"), "2664");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "90");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "0");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-90");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "355");
    }

    SECTION("003") {
        // MIR-684
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:2/2}}", {{"grid", "{grid:3/3}"}, {"interpolation", "nn"}},
            R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":[3,3]}},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":[2,2]},{"grid":[3,3]},{"grid":[3,3]}])");

        EXPECT_EQUAL(get(h, "numberOfDataPoints"), "7320");
    }
}


CASE("to regular_ll: random") {
    SECTION("001") {
        // MIR-488
        // generate a field with uniformly distributed integers between 1 and 3
        auto h = run(
            "{artificialInput:constant,constant:0.,gridded:True,gridType:regular_ll,north:10.,west:0.,south:0.,east:10."
            ",south_north_increment:1.,west_east_increment:1.,Ni:11,Nj:11}",
            {{"add-random", "uniform-int-distribution{a:1,b:3}"}},
            R"([{"type":"filter.add-random","add-random":"uniform-int-distribution{a:1,b:3}"},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"area":[10,0,0,10],"grid":[1,1]},{"area":[10,0,0,10],"grid":[1,1]},{"area":[10,0,0,10],"grid":[1,1]}])");

        EXPECT_EQUAL(get(h, "Ni"), "11");
        EXPECT_EQUAL(get(h, "Nj"), "11");
    }

    SECTION("002") {
        // MIR-488
        // generate a field with normal, or Gaussian, distributed values given mean and standard deviation
        auto h = run(
            "{artificialInput:constant,constant:0.,gridded:True,gridType:regular_ll,north:10.,west:0.,south:0.,east:10."
            ",south_north_increment:1.,west_east_increment:1.,Ni:11,Nj:11}",
            {{"add-random", "normal-distribution{mean:42.,stddev:3.}"}},
            R"([{"type":"filter.add-random","add-random":"normal-distribution{mean:42.,stddev:3.}"},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"area":[10,0,0,10],"grid":[1,1]},{"area":[10,0,0,10],"grid":[1,1]},{"area":[10,0,0,10],"grid":[1,1]}])");

        EXPECT_EQUAL(get(h, "Ni"), "11");
        EXPECT_EQUAL(get(h, "Nj"), "11");
    }
}


CASE("to rotated_ll: area-rotation") {
    // input regular_ll.2-2.grib1, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
        {{"area", "22.7/-13.7/-5.9/21.7"}, {"grid", "0.2/0.2"}, {"rotation", "-40/10"}, {"edition", "1"}},
        R"([{"type":"interpolate.grid2rotated-regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[22.7,-13.7,-5.9,21.7],"rotation":[-40,10],"grid":[0.2,0.2],"area":[89.9,0.1,-89.9,359.9]},{"type":"save","packing":"simple","edition":1}])",
        R"([{"grid":[2,2]},{"area":[22.7,-13.7,-5.9,21.7],"grid":[0.2,0.2],"projection":{"south_pole":[10,-40],"type":"rotation"},"reference":[0.1,0.1]},{"area":[22.7,-13.7,-5.9,21.7],"grid":[0.2,0.2],"projection":{"south_pole":[10,-40],"type":"rotation"},"reference":[0.1,0.1]}])");

    EXPECT_EQUAL(get(h, "Ni"), "178");
    EXPECT_EQUAL(get(h, "Nj"), "144");
    EXPECT_EQUAL(get(h, "iDirectionIncrementInDegrees"), "0.2");
    EXPECT_EQUAL(get(h, "jDirectionIncrementInDegrees"), "0.2");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-13.7");
    EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "21.7");
    EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "22.7");
    EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-5.9");
    EXPECT_EQUAL(get(h, "uvRelativeToGrid"), "0");
    EXPECT_EQUAL(get(h, "latitudeOfSouthernPoleInDegrees"), "-40");
    EXPECT_EQUAL(get(h, "longitudeOfSouthernPoleInDegrees"), "10");
}


CASE("to rotated_ll: nn") {
    // input regular_ll.2-2.grib1, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
        {{"grid", "{grid:5/5,rotation:[-40,20]}"}, {"interpolation", "nn"}, {"edition", "1"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":[5,5],"projection":{"south_pole":[20,-40],"type":"rotation"}}},{"type":"save","packing":"simple","edition":1}])",
        R"([{"grid":[2,2]},{"grid":[5,5],"projection":{"south_pole":[20,-40],"type":"rotation"}},{"grid":[5,5],"projection":{"south_pole":[20,-40],"type":"rotation"}}])");

    EXPECT_EQUAL(get(h, "gridType"), "rotated_ll");
    EXPECT_EQUAL(get(h, "Ni"), "72");
    EXPECT_EQUAL(get(h, "Nj"), "37");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "2664");
    EXPECT_EQUAL(get(h, "latitudeOfSouthernPoleInDegrees"), "-40");
    EXPECT_EQUAL(get(h, "longitudeOfSouthernPoleInDegrees"), "20");
}


CASE("to rotated_ll: rotation") {
    // MIR-233
    // input regular_ll.2-2.grib1, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:[2,2]}}",
        {{"grid", "3/3"}, {"rotation", "-30.0/-10.0"}, {"edition", "1"}},
        R"([{"type":"interpolate.grid2rotated-regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"rotation":[-30,-10],"grid":[3,3],"area":[90,0,-90,357]},{"type":"save","packing":"simple","edition":1}])",
        R"([{"grid":[2,2]},{"grid":[3,3],"projection":{"south_pole":[-10,-30],"type":"rotation"}},{"grid":[3,3],"projection":{"south_pole":[-10,-30],"type":"rotation"}}])");

    EXPECT_EQUAL(get(h, "uvRelativeToGrid"), "0");
}


}  // namespace mir::tests::regressions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
