// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/geo/eckit_geo_config.h"

#include "regression.h"


namespace mir::tests::regressions {


CASE("to healpix: grid-box-average") {
    // MIR-632
    // input gridType=reduced_gg,gridName=O8,shortName=q.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:O8}}", {{"interpolation", "grid-box-average"}, {"grid", "H2"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"grid-box-average","lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"H2"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"O8"},{"grid":"H2"},{"grid":"H2"}])");

    EXPECT_EQUAL(get(h, "gridType"), "healpix");
    EXPECT_EQUAL(get(h, "gridName"), "H2");
    EXPECT_EQUAL(get(h, "Nside"), "2");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "48");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "45");
}


CASE("to healpix: regrid") {
    // MIR-632
    // input gridType=reduced_gg,gridName=O8,shortName=q.grib2, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:O8}}", {{"grid", "H2"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"H2"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"O8"},{"grid":"H2"},{"grid":"H2"}])");

    EXPECT_EQUAL(get(h, "gridType"), "healpix");
    EXPECT_EQUAL(get(h, "gridName"), "H2");
    EXPECT_EQUAL(get(h, "Nside"), "2");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "48");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "45");
}


CASE("to reduced_gg: area") {
    SECTION("001") {
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"area", "-60/50/-90/180"}, {"grid", "N640"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[-60,50,-90,180],"grid":"N640"},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"area":[-60.0937057991215,50,-90,180],"grid":"N640"},{"area":[-60.0937057991215,50,-90,180],"grid":"N640"}])");

        EXPECT_EQUAL(get(h, "numberOfPoints"), "52654");
        EXPECT_EQUAL(get(h, "Nj"), "213");
        EXPECT_EQUAL(get(h, "N"), "640");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "-60.093");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "50");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-89.893");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "180");
    }

    SECTION("002") {
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"area", "90/-60/25/60"}, {"grid", "O1280"}, {"edition", "1"}},
            R"([{"type":"crop.area","area":[90,-60,25,60]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"area":[90,-60,25.0615109127255,60],"grid":"O1280"},{"area":[90,-60,25.0615109127255,60],"grid":"O1280"}])");

        EXPECT_EQUAL(get(h, "Nj"), "924");
        EXPECT_EQUAL(get(h, "N"), "1280");
    }

    SECTION("003") {
        // ECC-576
        // input param=2t,levtype=sfc,grid=N256, as its grid (GRIB1)
        auto h =
            run("{artificialInput:gridspec,gridSpec:{grid:N256}}", {{"area", "90/0/-90/359.6489"}, {"edition", "1"}},
                R"([{"type":"crop.area","area":[90,0,-90,359.649]},{"type":"save","packing":"simple","edition":1}])",
                R"([{"grid":"N256"},{"grid":"N256"},{"grid":"N256"}])");

        EXPECT_EQUAL(get(h, "numberOfValues"), "348528");
    }

    SECTION("004") {
        // ECC-576
        // input param=2t,levtype=sfc,grid=N256, as its grid (GRIB1)
        auto h = run("{artificialInput:gridspec,gridSpec:{grid:N256}}", {{"area", "90/0/-90/359.9"}, {"edition", "1"}},
                     R"([{"type":"crop.area","area":[90,0,-90,359.9]},{"type":"save","packing":"simple","edition":1}])",
                     R"([{"grid":"N256"},{"grid":"N256"},{"grid":"N256"}])");

        EXPECT_EQUAL(get(h, "numberOfValues"), "348528");
    }

    SECTION("005") {
        // ECC-576
        // input param=2t,levtype=sfc,grid=N640, as its grid (GRIB1)
        auto h = run("{artificialInput:gridspec,gridSpec:{grid:N640}}", {{"area", "90/0/-90/359.9"}, {"edition", "1"}},
                     R"([{"type":"crop.area","area":[90,0,-90,359.9]},{"type":"save","packing":"simple","edition":1}])",
                     R"([{"grid":"N640"},{"grid":"N640"},{"grid":"N640"}])");

        EXPECT_EQUAL(get(h, "numberOfValues"), "2140702");
    }

    SECTION("006") {
        // ECC-576
        // input param=2t,levtype=sfc,grid=N640, as its grid (GRIB1)
        auto h =
            run("{artificialInput:gridspec,gridSpec:{grid:N640}}", {{"area", "90/0/-90/359.99"}, {"edition", "1"}},
                R"([{"type":"crop.area","area":[90,0,-90,359.99]},{"type":"save","packing":"simple","edition":1}])",
                R"([{"grid":"N640"},{"grid":"N640"},{"grid":"N640"}])");

        EXPECT_EQUAL(get(h, "numberOfValues"), "2140702");
    }

    SECTION("007") {
        // ECC-576
        // input param=2t,levtype=sfc,grid=N640, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:N640}}", {{"area", "90/-180/-90/179.99"}, {"edition", "1"}},
            R"([{"type":"crop.area","area":[90,-180,-90,179.99]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"N640"},{"area":[90,-180,-90,180],"grid":"N640"},{"area":[90,-180,-90,180],"grid":"N640"}])");

        EXPECT_EQUAL(get(h, "numberOfValues"), "2140702");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-180");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "179.918");
    }

    SECTION("008") {
        // ECC-576
        // input param=2t,levtype=sfc,grid=O640, as its grid (GRIB1)
        auto h =
            run("{artificialInput:gridspec,gridSpec:{grid:O640}}", {{"area", "90/0/-90/359.999"}, {"edition", "1"}},
                R"([{"type":"crop.area","area":[90,0,-90,359.999]},{"type":"save","packing":"simple","edition":1}])",
                R"([{"grid":"O640"},{"grid":"O640"},{"grid":"O640"}])");

        EXPECT_EQUAL(get(h, "numberOfValues"), "1661440");
    }

    SECTION("009") {
        // MIR-236
        // input param=2t,levtype=sfc,grid=N640, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:N640}}",
            {{"area", "51.5189/2.08333/50.5349/5.83333"}, {"edition", "1"}},
            R"([{"type":"crop.area","area":[51.5189,2.08333,50.5349,5.83333]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"N640"},{"area":[51.3783623577251,2.08333333333333,50.5349419809359,5.625],"grid":"N640"},{"area":[51.3783623577251,2.08333333333333,50.5349419809359,5.625],"grid":"N640"}])");

        EXPECT_EQUAL(get(h, "gridName"), "N640");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "51.379");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "2.083");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "50.534");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "5.625");
    }

    SECTION("010") {
        // MIR-236
        // ECC-474
        // input param=msl,levtype=sfc,grid=O1280, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}", {{"area", "-25/135/-47/179"}, {"edition", "1"}},
            R"([{"type":"crop.area","area":[-25,135,-47,179]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"area":[-25.0615109127255,135,-46.9947264210242,179],"grid":"O1280"},{"area":[-25.0615109127255,135,-46.9947264210242,179],"grid":"O1280"}])");

        EXPECT_EQUAL(get(h, "gridName"), "O1280");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "-25.061");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "135");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-46.995");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "179");
    }

    SECTION("011") {
        // MIR-236
        // ECC-445
        // input param=msl,levtype=sfc,grid=O1280, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}", {{"area", "27.9/253/27.8/254"}, {"edition", "1"}},
            R"([{"type":"crop.area","area":[27.9,253,27.8,254]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"area":[27.8734616366611,-106.925675675676,27.8031628686037,-106.013513513513],"grid":"O1280"},{"area":[27.8734616366611,-106.925675675676,27.8031628686037,-106.013513513513],"grid":"O1280"}])");

        EXPECT_EQUAL(get(h, "gridName"), "O1280");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "27.874");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "253.074");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "27.803");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "253.987");
    }
}


CASE("to reduced_gg: frame") {
    // MIR-691
    // input param=2t,levtype=sfc,grid=N256, as its grid (GRIB1)
    auto h = run("{artificialInput:gridspec,gridSpec:{grid:N256}}", {{"frame", "5"}, {"edition", "1"}},
                 R"([{"type":"filter.frame","frame":5},{"type":"save","packing":"simple","edition":1}])",
                 R"([{"grid":"N256"},{"grid":"N256"},{"grid":"N256"}])");

    EXPECT_EQUAL(get(h, "gridName"), "N256");
}


CASE("to reduced_gg: regrid") {
    // 1d.logarithmic
    // input levtype=ml,levelist=1,param=lnsp,grid=O32, as its grid
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:O32}}", {{"grid", "O16"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"O16"},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"O32"},{"grid":"O16"},{"grid":"O16"}])");

    EXPECT_EQUAL(get(h, "Nj"), "32");
    EXPECT_EQUAL(get(h, "N"), "16");
}


CASE("to reduced_rotated_gg: area-rotation") {
    // input param=2t,levtype=sfc, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
        {{"area", "90/0/-90/360"}, {"grid", "N400"}, {"rotation", "-30.0/-10.0"}, {"edition", "1"}},
        R"([{"type":"interpolate.grid2rotated-namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[90,0,-90,360],"rotation":[-30,-10],"grid":"N400"},{"type":"save","packing":"simple","edition":1}])",
        R"([{"grid":"O1280"},{"grid":"N400","projection":{"south_pole":[-10,-30],"type":"rotation"}},{"grid":"N400","projection":{"south_pole":[-10,-30],"type":"rotation"}}])");

    EXPECT_EQUAL(get(h, "latitudeOfSouthernPoleInDegrees"), "-30");
    EXPECT_EQUAL(get(h, "longitudeOfSouthernPoleInDegrees"), "-10");
}


CASE("to regular_gg: area") {
    // input param=2t,levtype=sfc, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
        {{"grid", "f320"}, {"area", "56/-16.5/50.5/-3"}, {"edition", "1"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[56,-16.5,50.5,-3],"grid":"F320"},{"type":"save","packing":"simple","edition":1}])",
        R"([{"grid":"O1280"},{"area":[55.784517651898,-16.3125,50.7259739669878,-3.09375],"grid":"F320"},{"area":[55.784517651898,-16.3125,50.7259739669878,-3.09375],"grid":"F320"}])");

    EXPECT_EQUAL(get(h, "Ni"), "48");
    EXPECT_EQUAL(get(h, "Nj"), "19");
    EXPECT_EQUAL(get(h, "numberOfValues"), "912");
    EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "55.785");
    EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-16.313");
    EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "50.725");
    EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "-3.093");
}


CASE("to regular_gg: nn") {
    // input gridType=reduced_gg,gridName=O32,shortName=msl.grib1, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:O32}}", {{"grid", "F32"}, {"interpolation", "nn"}, {"edition", "1"}},
        R"([{"type":"interpolate.grid2namedgrid","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"F32"},{"type":"save","packing":"simple","edition":1}])",
        R"([{"grid":"O32"},{"grid":"F32"},{"grid":"F32"}])");

    EXPECT_EQUAL(get(h, "gridName"), "F32");
}


CASE("to regular_ll: area") {
    SECTION("001") {
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"area", "80/-10/10/100"}, {"grid", "1/1"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[80,-10,10,100],"grid":[1,1],"area":[90,0,-90,359]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"area":[80,-10,10,100],"grid":[1,1]},{"area":[80,-10,10,100],"grid":[1,1]}])");

        EXPECT_EQUAL(get(h, "Ni"), "111");
        EXPECT_EQUAL(get(h, "Nj"), "71");
    }

    SECTION("002") {
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"dont-compress-plan", ""}, {"grid", "1/1"}, {"area", "60/180/-56/-180"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[1,1],"area":[90,0,-90,359]},{"type":"crop.area","area":[60,180,-56,540]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"grid":[1,1]},{"area":[60,-180,-56,180],"grid":[1,1]},{"area":[60,-180,-56,180],"grid":[1,1]}])");

        EXPECT_EQUAL(get(h, "Ni"), "360");
        EXPECT_EQUAL(get(h, "Nj"), "117");
        EXPECT_EQUAL(get(h, "iDirectionIncrementInDegrees"), "1");
        EXPECT_EQUAL(get(h, "jDirectionIncrementInDegrees"), "1");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "60");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "180");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-56");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "539");
    }

    SECTION("003") {
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"grid", "1.5/1.5"}, {"area", "50/109.5/20/150"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[50,109.5,20,150],"grid":[1.5,1.5],"area":[89,0,-89.5,358.5]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"area":[50,109.5,20,150],"grid":[1.5,1.5],"reference":[0,0.5]},{"area":[50,109.5,20,150],"grid":[1.5,1.5],"reference":[0,0.5]}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "50");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "109.5");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "20");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "150");
    }

    SECTION("004") {
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"area", "-10/-110/-43/-65"}, {"grid", "3/3"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[-10,-110,-43,-65],"grid":[3,3],"area":[89,1,-88,358]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"area":[-10,-110,-43,-65],"grid":[3,3],"reference":[1,2]},{"area":[-10,-110,-43,-65],"grid":[3,3],"reference":[1,2]}])");

        EXPECT_EQUAL(get(h, "numberOfValues"), "192");
    }

    SECTION("005") {
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"area", "12.4/-25.6/-13.3/23.8"}, {"grid", ".1/.1"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[12.4,-25.6,-13.3,23.8],"grid":[0.1,0.1],"area":[90,0,-90,359.9]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"area":[12.4,-25.6,-13.3,23.8],"grid":[0.1,0.1]},{"area":[12.4,-25.6,-13.3,23.8],"grid":[0.1,0.1]}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "12.4");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-13.3");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-25.6");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "23.8");
        EXPECT_EQUAL(get(h, "jDirectionIncrementInDegrees"), "0.1");
        EXPECT_EQUAL(get(h, "iDirectionIncrementInDegrees"), "0.1");
    }

    SECTION("006") {
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"area", "12.4/-25.6/-13.3/23.8"}, {"grid", "1/1"}, {"dont-compress-plan", ""}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[1,1],"area":[89.7,0.4,-89.3,359.4]},{"type":"crop.area","area":[12.4,-25.6,-13.3,23.8]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"grid":[1,1],"reference":[0.4,0.7]},{"area":[11.7,-25.6,-13.3,23.4],"grid":[1,1],"reference":[0.4,0.7]},{"area":[11.7,-25.6,-13.3,23.4],"grid":[1,1],"reference":[0.4,0.7]}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "11.7");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-13.3");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-25.6");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "23.4");
        EXPECT_EQUAL(get(h, "jDirectionIncrementInDegrees"), "1");
        EXPECT_EQUAL(get(h, "iDirectionIncrementInDegrees"), "1");
    }

    SECTION("007") {
        // MIR-243
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"grid", "1/1"}, {"area", "90/0/-90/360"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[90,0,-90,360],"grid":[1,1],"area":[90,0,-90,359]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"grid":[1,1]},{"grid":[1,1]}])");

        EXPECT_EQUAL(get(h, "Ni"), "360");
        EXPECT_EQUAL(get(h, "Nj"), "181");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "90");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "0");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-90");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "359");
    }

    SECTION("008") {
        // MIR-243
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"grid", "1/1"}, {"area", "90/0/-90/360"}, {"dont-compress-plan", ""}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[1,1],"area":[90,0,-90,359]},{"type":"crop.area","area":[90,0,-90,360]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"grid":[1,1]},{"grid":[1,1]},{"grid":[1,1]}])");

        EXPECT_EQUAL(get(h, "Ni"), "360");
        EXPECT_EQUAL(get(h, "Nj"), "181");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "90");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "0");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-90");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "359");
    }

    SECTION("009") {
        // MIR-313
        // input levtype=ml,levelist=1,param=q, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}", {{"area", "19.3/-6.3/-2.3/18.3"}, {"grid", "1/1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[19.3,-6.3,-2.3,18.3],"grid":[1,1],"area":[89.7,0.7,-89.3,359.7]},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":"O1280"},{"area":[18.7,-6.3,-2.3,17.7],"grid":[1,1],"reference":[0.7,0.7]},{"area":[18.7,-6.3,-2.3,17.7],"grid":[1,1],"reference":[0.7,0.7]}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "18.7");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "353.7");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-2.3");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "17.7");
    }

    SECTION("010") {
        // MIR-313
        // input levtype=ml,levelist=1,param=q, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"area", "19.3/-6.3/-2.3/18.3"}, {"grid", "1/1"}, {"dont-compress-plan", ""}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[1,1],"area":[89.7,0.7,-89.3,359.7]},{"type":"crop.area","area":[19.3,-6.3,-2.3,18.3]},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":"O1280"},{"grid":[1,1],"reference":[0.7,0.7]},{"area":[18.7,-6.3,-2.3,17.7],"grid":[1,1],"reference":[0.7,0.7]},{"area":[18.7,-6.3,-2.3,17.7],"grid":[1,1],"reference":[0.7,0.7]}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "18.7");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "353.7");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-2.3");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "17.7");
    }
}


CASE("to regular_ll: k-nearest-neighbours-pseudo-laplace") {
    // MIR-544
    // input param=2t,levtype=sfc,grid=O16, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:O16}}",
        {{"grid", "2/2"},
         {"interpolation", "k-nearest-neighbours"},
         {"distance-weighting", "pseudo-laplace"},
         {"nearest-method", "nclosest-or-nearest"},
         {"edition", "1"}},
        R"([{"type":"interpolate.grid2regular-ll","interpolation":"k-nearest-neighbours","distance-tolerance":1,"distance-weighting":"pseudo-laplace","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nclosest-or-nearest","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[2,2],"area":[90,0,-90,358]},{"type":"save","packing":"simple","edition":1}])",
        R"([{"grid":"O16"},{"grid":[2,2]},{"grid":[2,2]}])");

    EXPECT_EQUAL(get(h, "iDirectionIncrementInDegrees"), "2");
    EXPECT_EQUAL(get(h, "jDirectionIncrementInDegrees"), "2");
}


CASE("to regular_ll: nn") {
    SECTION("001") {
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:o96}}",
            {{"grid", "{type:arakawa_c_um,grid:n96,arrangement:T}"}, {"interpolation", "nn"}},
            R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":[1.875,1.25],"order":"i+j+","reference":[0.9375,0.625]}},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":"O96"},{"grid":[1.875,1.25],"order":"i+j+","reference":[0.9375,0.625]},{"grid":[1.875,1.25],"order":"i+j+","reference":[0.9375,0.625]}])");

        EXPECT_EQUAL(get(h, "numberOfDataPoints"), "27648");
        EXPECT_EQUAL(get(h, "gridType"), "regular_ll");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "-89.375");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "0.9375");
    }

    SECTION("002") {
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:o96}}",
            {{"grid", "{type:arakawa_c_um,grid:n96,arrangement:V}"}, {"interpolation", "nn"}},
            R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":[1.875,1.25],"order":"i+j+","reference":[0.9375,0]}},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":"O96"},{"grid":[1.875,1.25],"order":"i+j+","reference":[0.9375,0]},{"grid":[1.875,1.25],"order":"i+j+","reference":[0.9375,0]}])");

        EXPECT_EQUAL(get(h, "numberOfDataPoints"), "27840");
        EXPECT_EQUAL(get(h, "gridType"), "regular_ll");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "-90");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "0.9375");
    }
}


CASE("to regular_ll: plan") {
    SECTION("001") {
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"plan", "grid2regular-ll(grid=[2,2],area=[90,-3,0,3]);crop(area=[90,-3,0,4]);frame(1)"},
             {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[90,-3,0,4],"grid":[2,2],"area":[90,1,-90,359]},{"type":"filter.frame","frame":1},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"area":[90,-3,0,3],"grid":[2,2],"reference":[1,0]},{"area":[90,-3,0,3],"grid":[2,2],"reference":[1,0]},{"area":[90,-3,0,3],"grid":[2,2],"reference":[1,0]}])");

        EXPECT_EQUAL(get(h, "Ni"), "4");
        EXPECT_EQUAL(get(h, "Nj"), "46");
    }

    SECTION("002") {
        // MIR-534
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"plan", "grid2regular-ll(grid=[0.5,0.5]);crop(area=france)"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[51.5,-5,42,8.5],"grid":[0.5,0.5],"area":[90,0,-90,359.5]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"area":[51.5,-5,42,8.5],"grid":[0.5,0.5]},{"area":[51.5,-5,42,8.5],"grid":[0.5,0.5]}])");

        EXPECT_EQUAL(get(h, "Ni"), "28");
        EXPECT_EQUAL(get(h, "Nj"), "20");
        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "51.5");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "-5");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "42");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "8.5");
    }
}


CASE("to regular_ll: regrid") {
    SECTION("001") {
        // input param=2t,levtype=sfc, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}", {{"grid", "2/2"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[2,2],"area":[90,0,-90,358]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O1280"},{"grid":[2,2]},{"grid":[2,2]}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "90");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "0");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-90");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "358");
    }

    SECTION("002") {
        // input O400.version=1.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O400}}", {{"grid", "3/3"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[3,3],"area":[90,0,-90,357]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O400"},{"grid":[3,3]},{"grid":[3,3]}])");

        EXPECT_EQUAL(get(h, "gridType"), "regular_ll");
    }

    SECTION("003") {
        // input O400.version=2.grib1, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O400}}", {{"grid", "3/3"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[3,3],"area":[90,0,-90,357]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"grid":"O400"},{"grid":[3,3]},{"grid":[3,3]}])");

        EXPECT_EQUAL(get(h, "gridType"), "regular_ll");
    }

    SECTION("004") {
        // input O400.version=3.grib1.NON-GLOBAL, as its grid (GRIB1)
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{area:[90,0,-90,359.775],grid:O400}}",
            {{"grid", "3/3"}, {"edition", "1"}},
            R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[3,3],"area":[90,0,-90,357]},{"type":"save","packing":"simple","edition":1}])",
            R"([{"area":[90,0,-90,359.775],"grid":"O400"},{"grid":[3,3]},{"grid":[3,3]}])");

        EXPECT_EQUAL(get(h, "gridType"), "regular_ll");
    }
}


CASE("to rotated_ll: area-rotation") {
    SECTION("001") {
        // MIR-313
        // input levtype=ml,levelist=1,param=q, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"area", "19.3/-6.3/-2.3/18.3"}, {"grid", "1/1"}, {"rotation", "-40/10"}},
            R"([{"type":"interpolate.grid2rotated-regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"cropping":[19.3,-6.3,-2.3,18.3],"rotation":[-40,10],"grid":[1,1],"area":[89.7,0.7,-89.3,359.7]},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":"O1280"},{"area":[18.7,-6.3,-2.3,17.7],"grid":[1,1],"projection":{"south_pole":[10,-40],"type":"rotation"},"reference":[0.7,0.7]},{"area":[18.7,-6.3,-2.3,17.7],"grid":[1,1],"projection":{"south_pole":[10,-40],"type":"rotation"},"reference":[0.7,0.7]}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "18.7");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "353.7");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-2.3");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "17.7");
    }

    SECTION("002") {
        // MIR-313
        // input levtype=ml,levelist=1,param=q, as its grid
        auto h = run(
            "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
            {{"area", "19.3/-6.3/-2.3/18.3"}, {"grid", "1/1"}, {"rotation", "-40/10"}, {"dont-compress-plan", ""}},
            R"([{"type":"interpolate.grid2rotated-regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"rotation":[-40,10],"grid":[1,1],"area":[89.7,0.7,-89.3,359.7]},{"type":"crop.area","area":[19.3,-6.3,-2.3,18.3]},{"type":"save","packing":"ccsds","edition":2}])",
            R"([{"grid":"O1280"},{"area":[89.770187600933,0,-90,360],"grid":[1,1],"projection":{"south_pole":[10,-40],"type":"rotation"},"reference":[0.7,0.7]},{"area":[18.7,-6.3,-2.3,17.7],"grid":[1,1],"projection":{"south_pole":[10,-40],"type":"rotation"},"reference":[0.7,0.7]},{"area":[18.7,-6.3,-2.3,17.7],"grid":[1,1],"projection":{"south_pole":[10,-40],"type":"rotation"},"reference":[0.7,0.7]}])");

        EXPECT_EQUAL(get(h, "latitudeOfFirstGridPointInDegrees"), "18.7");
        EXPECT_EQUAL(get(h, "longitudeOfFirstGridPointInDegrees"), "353.7");
        EXPECT_EQUAL(get(h, "latitudeOfLastGridPointInDegrees"), "-2.3");
        EXPECT_EQUAL(get(h, "longitudeOfLastGridPointInDegrees"), "17.7");
    }
}


CASE("to rotated_ll: nn") {
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:o32}}",
        {{"grid", "{grid:5/5,rotation:[-40,20]}"}, {"interpolation", "nn"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"grid":[5,5],"projection":{"south_pole":[20,-40],"type":"rotation"}}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"O32"},{"grid":[5,5],"projection":{"south_pole":[20,-40],"type":"rotation"}},{"grid":[5,5],"projection":{"south_pole":[20,-40],"type":"rotation"}}])");

    EXPECT_EQUAL(get(h, "gridType"), "rotated_ll");
    EXPECT_EQUAL(get(h, "Ni"), "72");
    EXPECT_EQUAL(get(h, "Nj"), "37");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "2664");
    EXPECT_EQUAL(get(h, "latitudeOfSouthernPoleInDegrees"), "-40");
    EXPECT_EQUAL(get(h, "longitudeOfSouthernPoleInDegrees"), "20");
    EXPECT_EQUAL(get(h, "numberOfMissing"), "0");
}


CASE("to rotated_ll: plan") {
    // input param=2t,levtype=sfc, as its grid (GRIB1)
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:O1280}}",
        {{"plan",
          "grid2regular-ll(grid=[2,2]);grid2namedgrid(grid=O80);grid2rotated-regular-ll(grid=[2,2],rotation=[-90,0])"},
         {"edition", "1"}},
        R"([{"type":"interpolate.grid2regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":[2,2],"area":[90,0,-90,358]},{"type":"interpolate.grid2namedgrid","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":"O80"},{"type":"interpolate.grid2rotated-regular-ll","interpolation":"linear","finite-element-projection-fail":"missing-value","finite-element-validate-mesh":false,"lsm-weight-adjustment":0.2,"non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"rotation":[-90,0],"grid":[2,2],"area":[90,0,-90,358]},{"type":"save","packing":"simple","edition":1}])",
        R"([{"grid":"O1280"},{"grid":[2,2]},{"grid":"O80"},{"grid":[2,2]},{"grid":[2,2]}])");

    EXPECT_EQUAL(get(h, "numberOfValues"), "16380");
}


CASE("to unstructured_grid: nn") {
    auto h = run(
        "{artificialInput:gridspec,gridSpec:{grid:o32}}", {{"grid", "{grid:ICON-CH2}"}, {"interpolation", "nn"}},
        R"([{"type":"interpolate.grid2gridspec","interpolation":"nn","distance-weighting":"inverse-distance-weighting-squared","lsm-weight-adjustment":0.2,"nclosest":4,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":["missing-if-heaviest-missing"],"pole-displacement-in-degree":0,"prune-epsilon":1e-10,"solver":{"type":"multiply"},"grid":{"area":[50.6,-0.9,41.9,17.8],"grid":"ICON-CH2_C","uid":"bbbd5a09855499243c7a4aa4c8762920"}},{"type":"save","packing":"ccsds","edition":2}])",
        R"([{"grid":"O32"},{"area":[50.6,-0.9,41.9,17.8],"grid":"ICON-CH2_C","uid":"bbbd5a09855499243c7a4aa4c8762920"},{"area":[50.6,-0.9,41.9,17.8],"grid":"ICON-CH2_C","uid":"bbbd5a09855499243c7a4aa4c8762920"}])");

    EXPECT_EQUAL(get(h, "gridType"), "unstructured_grid");
    EXPECT_EQUAL(get(h, "numberOfDataPoints"), "283876");
    EXPECT_EQUAL(get(h, "uuidOfHGrid"), "bbbd5a09855499243c7a4aa4c8762920");
    EXPECT_EQUAL(get(h, "numberOfMissing"), "0");
}


}  // namespace mir::tests::regressions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
