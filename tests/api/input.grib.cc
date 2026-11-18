// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/io/FileHandle.h"
#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribAllFileInput.h"
#include "mir/input/GribDataHandleInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/GribMemoryInput.h"
#include "mir/input/MultiDimensionalGribFileInput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir::api::tests {


using input::MIRInput;
using input_t = std::unique_ptr<MIRInput>;


template <typename T>
std::string to_string(const T& p) {
    std::ostringstream str;
    str << p;
    return str.str();
}


std::vector<char> read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}


/// Input built as the mir tool does, from a path and the --input option
input_t build(const std::string& path, const std::string& input = "") {
    param::SimpleParametrisation param;
    if (!input.empty()) {
        param.set("input", input);
    }
    return input_t(input::MIRInputFactory::build(path, param));
}


template <typename T>
T get(const MIRInput& in, const std::string& name) {
    T value{};
    EXPECT(in.parametrisation().get(name, value));
    return value;
}


long grib_long(grib_handle* h, const std::string& key) {
    long value = 0;
    GRIB_CALL(codes_get_long(h, key.c_str(), &value));
    return value;
}


long grib_long(grib_handle* h, const std::string& key, long missing) {
    return codes_is_defined(h, key.c_str()) != 0 ? grib_long(h, key) : missing;
}


double grib_double(grib_handle* h, const std::string& key) {
    double value = 0;
    GRIB_CALL(codes_get_double(h, key.c_str(), &value));
    return value;
}


CASE("GribFileInput: gridded or spectral") {
    SECTION("gridded") {
        for (const auto& path : std::vector<std::string>{
                 "ICON.shortName=lsm.grib2",
                 "MIR-375.grib1",
                 "MIR-553.grib2",
                 "O400.version=1.grib1",
                 "O400.version=2.grib1",
                 "ORCA.grib2",
                 "gridType=healpix,Nside=32,orderingConvention=ring.grib2",
                 "gridType=lambert,missingValuesPresent=0.grib2",
                 "gridType=lambert_azimuthal_equal_area.grib2",
                 "gridType=polar_stereographic.grib1",
                 "gridType=polar_stereographic.grib2",
                 "gridType=reduced_gg,gridName=N320,shortName=msl.grib1",
                 "gridType=reduced_gg,gridName=O1280,shortName=msl.grib1",
                 "gridType=reduced_gg,gridName=O32,shortName=msl.grib1",
                 "gridType=reduced_ll,shortName=2dfd,packingType=grid_simple_matrix.grib1",
                 "gridType=reduced_ll,shortName=wsp,packingType=grid_simple.grib1",
                 "gridType=regular_gg,gridName=F32,shortName=msl.grib1",
                 "gridType=regular_gg,gridName=F48,shortName=msl,global=0.001.grib1",
                 "gridType=regular_gg,gridName=F48,shortName=msl,global=0.002.grib1",
                 "gridType=regular_gg,gridName=F640,shortName=msl,global=0.grib1",
                 "gridType=regular_gg,gridName=F80,shortName=msl,global=0.001.grib1",
                 "gridType=regular_gg,gridName=F80,shortName=msl,global=0.002.grib1",
                 "gridType=unstructured_grid.grib2",
                 "levtype=ml,levelist=1,param=lnsp,grid=O32",
                 "mofc_ocean_fc_inst_horizontal.grib1",
                 "orog_1km.grib2",
                 "packingType=grid_ccsds.grib2",
                 "packingType=grid_complex.grib2",
                 "packingType=grid_ieee.grib2",
                 "packingType=grid_second_order.grib2",
                 "packingType=grid_simple.grib1",
                 "packingType=grid_simple.grib2",
                 "regular_ll.2-2.grib1",
                 "regular_ll.2-2.grib2",
                 "rotated_ll.3-3.grib1",
                 "stream=wave,param=2dfd,direction=1,frequency=1",
             }) {
            Log::info() << "'" << path << "'" << std::endl;

            input_t input(new input::GribFileInput(path));

            // testing first message only
            ASSERT(input->next());
            const auto& param = input->parametrisation();

            auto gridded_1 = param.has("gridded");
            EXPECT(gridded_1);

            auto gridded_2 = false;
            EXPECT(param.get("gridded", gridded_2));
            EXPECT_EQUAL(gridded_1, gridded_2);

            EXPECT_NOT(param.has("spectral"));
        }
    }


    SECTION("spectral") {
        for (const auto& path : std::vector<std::string>{
                 "date=19930202",
                 "gridType=sh,shortName=t,J=20.grib1",
                 "packingType=spectral_complex,param=vo_d.grib1",
                 "packingType=spectral_complex,param=vo_d.grib2",
                 "packingType=spectral_complex.grib1",
                 "packingType=spectral_complex.grib2",
                 "packingType=spectral_ieee.grib2",
                 "packingType=spectral_simple.grib1",
             }) {
            Log::info() << "'" << path << "'" << std::endl;

            input_t input(new input::GribFileInput(path));

            // testing first message only
            ASSERT(input->next());
            const auto& param = input->parametrisation();

            auto spectral_1 = param.has("spectral");
            EXPECT(spectral_1);

            auto spectral_2 = false;
            EXPECT(param.get("spectral", spectral_2));
            EXPECT_EQUAL(spectral_1, spectral_2);

            long truncation = 0;
            EXPECT(param.get("truncation", truncation) && truncation > 0);
        }
    }
}


CASE("GribInput: field values") {
    for (const auto& path : std::vector<std::string>{
             "MIR-425.grib1",
             "MIR-442.001.wrong.grib1",
             "MIR-583.grib1",
             "MIR-587.grib2",
             "MPY-400.grib1",
             "gridType=healpix,Nside=2,orderingConvention=nested.grib2",
             "gridType=healpix,Nside=2,orderingConvention=ring.grib2",
             "gridType=healpix,Nside=4,orderingConvention=nested.grib2",
             "gridType=reduced_gg,gridName=O32,shortName=msl.grib1",
             "gridType=reduced_gg,gridName=O8,shortName=q.grib2",
             "gridType=regular_gg,gridName=F32,shortName=msl.grib1",
             "gridType=regular_gg,gridName=F48,shortName=msl,global=0.001.grib1",
             "gridType=regular_gg,gridName=F8,shortName=q.grib2",
             "gridType=regular_gg,gridName=F80,shortName=msl,global=0.001.grib1",
             "gridType=regular_ll,param=t,levtype=ml,level=1,grid=9-9.grib2",
             "gridType=regular_ll,param=t,levtype=pl,level=1000,grid=9-9.grib1",
             "gridType=sh,shortName=t,J=20.grib1",
             "packingType=grid_ccsds.grib2",
             "packingType=grid_complex.grib2",
             "packingType=grid_ieee.grib2",
             "packingType=grid_second_order.grib2",
             "packingType=grid_simple.grib1",
             "packingType=spectral_complex.grib2",
             "packingType=spectral_ieee.grib2",
             "packingType=spectral_simple.grib1",
             "param=2t,levtype=sfc,grid=O16",
             "param=q,grid=1_1,area=small.grib2",
             "regular_ll.0.5-0.5.europe.grib1",
             "regular_ll.2-2.grib2",
             "regular_ll.5-5.area.grib1",
             "rotated_ll.3-3.grib1",
             "stream=wave,param=swh,special",
             "tiny-3x3.grib",
             "ukmo-tigge.count=4.grib",
         }) {
        Log::info() << "'" << path << "'" << std::endl;

        input_t input(new input::GribFileInput(path));
        ASSERT(input->next());

        auto field = input->field();
        EXPECT_EQUAL(field.dimensions(), 1);

        repres::RepresentationHandle repres(field.representation());
        const auto& values = field.values(0);
        EXPECT_EQUAL(values.size(), repres->numberOfValues());

        // values (reordered) keep their range, missing values are flagged
        auto* h = input->gribHandle();
        ASSERT(h != nullptr);

        auto mv     = field.missingValue();
        auto min    = std::numeric_limits<double>::max();
        auto max    = std::numeric_limits<double>::lowest();
        size_t miss = 0;
        for (const auto& v : values) {
            if (field.hasMissing() && v == mv) {
                ++miss;
                continue;
            }
            min = std::min(min, v);
            max = std::max(max, v);
        }

        EXPECT_EQUAL(field.hasMissing(), (grib_long(h, "missingValuesPresent", 0) != 0));
        EXPECT_EQUAL(miss, static_cast<size_t>(grib_long(h, "numberOfMissing", 0)));

        if (miss < values.size() && codes_is_defined(h, "min") != 0 && codes_is_defined(h, "max") != 0) {
            EXPECT(std::abs(min - grib_double(h, "min")) <= 1e-9 * std::max(1., std::abs(min)));
            EXPECT(std::abs(max - grib_double(h, "max")) <= 1e-9 * std::max(1., std::abs(max)));
        }
    }


    SECTION("unsupported, wrongly encoded") {
        input_t ocean(new input::GribFileInput("mofc_ocean_fc_inst_horizontal.grib1"));
        ASSERT(ocean->next());
        EXPECT_THROWS_AS(ocean->field(), exception::UserError);

        input_t wrong(new input::GribFileInput("MIR-442.003.wrong.grib1"));
        ASSERT(wrong->next());
        EXPECT_THROWS_AS(wrong->field(), eckit::AssertionFailed);
    }
}


CASE("GribInput: pl array with zeros (rows without points)") {
    input_t o8(new input::GribFileInput("gridType=reduced_gg,gridName=O8,shortName=q.grib2"));
    ASSERT(o8->next());

    // a message with the first and last two rows without points
    auto* h = codes_handle_clone(o8->gribHandle());
    ASSERT(h != nullptr);

    size_t n = 0;
    GRIB_CALL(codes_get_size(h, "pl", &n));
    std::vector<long> pl(n);
    GRIB_CALL(codes_get_long_array(h, "pl", pl.data(), &n));

    pl[0] = pl[n - 2] = pl[n - 1] = 0;
    GRIB_CALL(codes_set_long_array(h, "pl", pl.data(), n));

    size_t count = 0;
    for (const auto& p : pl) {
        count += static_cast<size_t>(p);
    }
    std::vector<double> values(count, 1.);
    GRIB_CALL(codes_set_double_array(h, "values", values.data(), values.size()));

    const void* message = nullptr;
    size_t size         = 0;
    GRIB_CALL(codes_get_message(h, &message, &size));
    const std::vector<char> buffer(static_cast<const char*>(message), static_cast<const char*>(message) + size);
    codes_handle_delete(h);

    input::GribMemoryInput in(buffer.data(), buffer.size());

    // the pl array is fixed (zeros replaced by their nearest non-zero neighbour)
    auto expected   = pl;
    expected[0]     = pl[1];
    expected[n - 2] = pl[n - 3];
    expected[n - 1] = pl[n - 3];

    std::vector<long> fixed;
    EXPECT(static_cast<MIRInput&>(in).parametrisation().get("pl", fixed));
    EXPECT(fixed == expected);

    // the rows without points are missing values
    auto field = static_cast<MIRInput&>(in).field();
    EXPECT(field.hasMissing());

    const auto& v = field.values(0);
    EXPECT_EQUAL(v.size(), count + static_cast<size_t>(expected[0] + expected[n - 2] + expected[n - 1]));

    size_t missing = 0;
    for (const auto& x : v) {
        missing += x == field.missingValue() ? 1 : 0;
    }
    EXPECT_EQUAL(missing, static_cast<size_t>(expected[0] + expected[n - 2] + expected[n - 1]));
}


CASE("GribInput: parametrisation") {
    SECTION("regular_ll") {
        input_t in(new input::GribFileInput("regular_ll.2-2.grib1"));
        ASSERT(in->next());
        const auto& param = in->parametrisation();

        EXPECT_EQUAL(get<std::string>(*in, "gridType"), "regular_ll");
        EXPECT_EQUAL(get<double>(*in, "north"), 90.);
        EXPECT_EQUAL(get<double>(*in, "south"), -90.);
        EXPECT_EQUAL(get<double>(*in, "west"), 0.);
        EXPECT_EQUAL(get<double>(*in, "east"), 358.);
        EXPECT_EQUAL(get<double>(*in, "west_east_increment"), 2.);
        EXPECT_EQUAL(get<double>(*in, "south_north_increment"), 2.);
        EXPECT(get<std::vector<double>>(*in, "grid") == std::vector<double>({2., 2.}));
        EXPECT(get<std::vector<double>>(*in, "area") == std::vector<double>({90., 0., -90., 358.}));
        EXPECT_EQUAL(get<long>(*in, "paramId"), 151);
        EXPECT_EQUAL(get<long>(*in, "param"), 151);
        EXPECT_EQUAL(get<int>(*in, "Ni"), 180);
        EXPECT_EQUAL(get<std::string>(*in, "order"), "i+j-");
        EXPECT_EQUAL(get<std::string>(*in, "packing"), "simple");
        EXPECT(get<bool>(*in, "gridded_regular_ll"));
        EXPECT(get<double>(*in, "angular_precision") >= 0.);

        // not a wind component
        long wind = -1;
        EXPECT_NOT(param.get("is_wind_component_uv", wind));
        EXPECT_NOT(param.get("is_wind_component_vod", wind));

        std::string gridSpec;
        EXPECT(param.get("gridSpec", gridSpec));
        EXPECT(gridSpec.find("[2,2]") != std::string::npos);

        // keys not available
        long l = 0;
        bool b = false;
        std::string s;
        std::vector<long> vl;
        std::vector<double> vd;
        EXPECT_NOT(param.has("statistics"));
        EXPECT_NOT(param.get("statistics", l));
        EXPECT_NOT(param.get("statistics", b));
        EXPECT_NOT(param.get("statistics", s));
        EXPECT_NOT(param.get("statistics", vd));
        EXPECT_NOT(param.get("statistics", vl));
        EXPECT_NOT(param.has("doesNotExist"));
        EXPECT_NOT(param.get("doesNotExist", vl));
        EXPECT_NOT(param.has("latitudes/longitudes"));
        EXPECT_NOT(param.has("rotation"));
        EXPECT_NOT(param.has("pl"));

        // grid name is a string, not a vector
        EXPECT_NOT(param.get("gridName", vd));

        std::vector<int> vi;
        EXPECT_THROWS_AS(param.get("Nj", vi), eckit::NotImplemented);

        std::vector<std::string> vs;
        EXPECT_THROWS_AS(param.get("gridType", vs), eckit::NotImplemented);

        // other types (on a fresh input, as the parametrisation caches values by type)
        input_t fresh(new input::GribFileInput("regular_ll.2-2.grib1"));
        ASSERT(fresh->next());

        // grid is not a scalar
        double d = 0;
        EXPECT_THROWS_AS(fresh->parametrisation().get("grid", d), eckit::AssertionFailed);

        EXPECT(get<std::vector<float>>(*fresh, "grid") == std::vector<float>({2.F, 2.F}));
        EXPECT_EQUAL(get<float>(*fresh, "north"), 90.F);
    }


    SECTION("rotated_ll") {
        input_t in(new input::GribFileInput("rotated_ll.3-3.grib1"));
        ASSERT(in->next());

        EXPECT_EQUAL(get<std::string>(*in, "gridType"), "rotated_ll");

        auto rotation = get<std::vector<double>>(*in, "rotation");
        EXPECT_EQUAL(rotation.size(), 2);
        EXPECT_EQUAL(rotation[0], get<double>(*in, "south_pole_latitude"));
        EXPECT_EQUAL(rotation[1], get<double>(*in, "south_pole_longitude"));
        EXPECT_EQUAL(get<double>(*in, "south_pole_rotation_angle"), 0.);
        EXPECT(get<std::vector<double>>(*in, "grid") == std::vector<double>({3., 3.}));
    }


    SECTION("reduced_gg, regular_gg") {
        input_t o32(new input::GribFileInput("gridType=reduced_gg,gridName=O32,shortName=msl.grib1"));
        ASSERT(o32->next());

        EXPECT_EQUAL(get<std::string>(*o32, "grid"), "O32");
        EXPECT_EQUAL(get<long>(*o32, "octahedral"), 32);
        EXPECT_EQUAL(get<double>(*o32, "east"), 360. - 360. / 144.);
        EXPECT_EQUAL(get<std::vector<long>>(*o32, "pl").size(), 64);
        EXPECT_NOT(o32->parametrisation().has("reduced"));

        input_t n320(new input::GribFileInput("gridType=reduced_gg,gridName=N320,shortName=msl.grib1"));
        ASSERT(n320->next());
        EXPECT_EQUAL(get<long>(*n320, "reduced"), 320);
        EXPECT_EQUAL(get<std::string>(*n320, "grid"), "N320");

        input_t f32(new input::GribFileInput("gridType=regular_gg,gridName=F32,shortName=msl.grib1"));
        ASSERT(f32->next());
        EXPECT_EQUAL(get<long>(*f32, "regular"), 32);
        EXPECT_EQUAL(get<std::string>(*f32, "grid"), "F32");
        EXPECT_EQUAL(get<std::string>(*f32, "gridType"), "regular_gg");
    }


    SECTION("HEALPix") {
        for (const auto& order : std::vector<std::string>{"nested", "ring"}) {
            input_t in(new input::GribFileInput("gridType=healpix,Nside=2,orderingConvention=" + order + ".grib2"));
            ASSERT(in->next());

            EXPECT_EQUAL(get<std::string>(*in, "grid"), std::string(order == "nested" ? "HN2" : "H2"));
            EXPECT_EQUAL(get<std::string>(*in, "order"), order);
            EXPECT_EQUAL(get<std::string>(*in, "packing"), "ccsds");
            EXPECT_EQUAL(get<long>(*in, "Nside"), 2);
        }
    }


    SECTION("spectral") {
        input_t in(new input::GribFileInput("packingType=spectral_complex,param=vo_d.grib2"));

        ASSERT(in->next());
        EXPECT_EQUAL(get<long>(*in, "truncation"), 20);
        EXPECT_EQUAL(get<std::string>(*in, "packing"), "complex");
        EXPECT_EQUAL(get<long>(*in, "is_wind_component_vod"), 1);

        ASSERT(in->next());
        EXPECT_EQUAL(get<long>(*in, "is_wind_component_vod"), 2);

        EXPECT_NOT(in->next());
    }


    SECTION("wind components") {
        input_t in(new input::GribFileInput("date=20200308,level=1000,grid=O80,param=u_v"));

        ASSERT(in->next());
        EXPECT_EQUAL(get<long>(*in, "is_wind_component_uv"), 1);

        ASSERT(in->next());
        EXPECT_EQUAL(get<long>(*in, "is_wind_component_uv"), 2);
    }


    SECTION("projections") {
        struct test_t {
            std::string path;
            std::string gridType;
        };

        for (const auto& test : std::vector<test_t>{
                 {"gridType=lambert,missingValuesPresent=0.grib2", "lambert"},
                 {"gridType=lambert_azimuthal_equal_area.grib2", "lambert_azimuthal_equal_area"},
                 {"gridType=polar_stereographic.grib2", "polar_stereographic"},
             }) {
            input_t in(new input::GribFileInput(test.path));
            ASSERT(in->next());

            EXPECT_EQUAL(get<std::string>(*in, "gridType"), test.gridType);

            auto grid = get<std::vector<double>>(*in, "grid");
            EXPECT_EQUAL(grid.size(), 2);
            EXPECT(grid[0] > 0. && grid[1] > 0.);
        }
    }


    SECTION("unstructured_grid") {
        input_t in(new input::GribFileInput("ICON.shortName=lsm.grib2"));
        ASSERT(in->next());

        EXPECT_EQUAL(get<std::string>(*in, "gridType"), "unstructured_grid");
        EXPECT_EQUAL(get<std::string>(*in, "uid").size(), 32);
    }


    SECTION("marsRequest") {
        input::GribFileInput in("regular_ll.2-2.grib1");
        ASSERT(static_cast<MIRInput&>(in).next());

        std::ostringstream request;
        in.marsRequest(request);
        EXPECT(request.str().find("param=") != std::string::npos);
    }
}


CASE("GribInput: auxiliary information") {
    SECTION("griddef") {
        auto in = build("gridType=unstructured_grid.grib2", "{griddef: gridType=unstructured_grid.pts}");
        ASSERT(in->next());

        EXPECT(in->parametrisation().has("latitudes/longitudes"));

        auto latitudes = get<std::vector<double>>(*in, "latitudes");
        EXPECT_EQUAL(latitudes.size(), 42);
        EXPECT_EQUAL(latitudes.front(), 90.);

        auto field = in->field();
        EXPECT_EQUAL(field.values(0).size(), 42);
    }


    SECTION("latitudes/longitudes") {
        auto in = build("ICON.shortName=lsm.grib2",
                        "{latitudes: ICON.shortName=CLAT.grib2, longitudes: ICON.shortName=CLON.grib2}");
        ASSERT(in->next());

        EXPECT(in->parametrisation().has("latitudes/longitudes"));
        EXPECT_EQUAL(get<std::vector<double>>(*in, "longitudes").size(), 20480);
        EXPECT_EQUAL(in->field().values(0).size(), 20480);
    }


    SECTION("errors") {
        EXPECT_THROWS_AS(build("ICON.shortName=lsm.grib2", "{latitudes: ICON.shortName=CLAT.grib2}"),
                         exception::UserError);
        EXPECT_THROWS_AS(build("ICON.shortName=lsm.grib2",
                               "{latitudes: ICON.shortName=CLAT.grib2, longitudes: ICON.shortName=CLON.grib2, "
                               "griddef: tour.pts}"),
                         exception::UserError);
    }
}


CASE("GribFileInput: messages") {
    const std::string path = "different-interpolation-methods.grib1";

    auto paramIds = [](MIRInput& in) {
        std::vector<long> ids;
        while (in.next()) {
            ids.push_back(get<long>(in, "paramId"));
        }
        return ids;
    };


    SECTION("all") {
        input::GribFileInput in(path);
        EXPECT(paramIds(in) == std::vector<long>({29, 151, 30, 151}));
        EXPECT_EQUAL(static_cast<MIRInput&>(in).dimensions(), 1);
    }


    SECTION("skip, step") {
        input::GribFileInput even(path, 0, 2);
        EXPECT(paramIds(even) == std::vector<long>({29, 30}));

        input::GribFileInput odd(path, 1, 2);
        EXPECT(paramIds(odd) == std::vector<long>({151, 151}));

        input::GribFileInput last(path, 3, 1);
        EXPECT(paramIds(last) == std::vector<long>({151}));

        input::GribFileInput beyond(path, 5, 1);
        EXPECT(paramIds(beyond).empty());

        EXPECT(static_cast<MIRInput&>(even).sameAs(input::GribFileInput(path, 0, 2)));
        EXPECT_NOT(static_cast<MIRInput&>(even).sameAs(odd));
        EXPECT(to_string(static_cast<MIRInput&>(odd)) == "GribFileInput[path=" + path + ",skip=1, step=2]");
    }


    SECTION("offset") {
        input::GribFileInput first(path);
        MIRInput& in = first;
        ASSERT(in.next());

        auto offset = static_cast<off_t>(grib_long(in.gribHandle(), "totalLength"));

        input::GribFileInput second(path, offset);
        EXPECT(paramIds(second) == std::vector<long>({151, 30, 151}));
    }


    SECTION("only") {
        input::GribFileInput file(path);
        MIRInput& in = file;

        EXPECT(in.only(151));
        EXPECT_EQUAL(get<long>(in, "paramId"), 151);
        EXPECT(in.only(151));
        EXPECT_NOT(in.only(151));

        input::GribFileInput none(path);
        EXPECT_NOT(static_cast<MIRInput&>(none).only(167));
    }


    SECTION("multi-dimensional") {
        input::MultiDimensionalGribFileInput file(path, 2);
        MIRInput& in = file;

        ASSERT(in.next());
        EXPECT_EQUAL(in.dimensions(), 2);
        EXPECT_EQUAL(get<long>(in, "paramId"), 29);
        long paramId = 0;
        EXPECT(in.parametrisation(1).get("paramId", paramId) && paramId == 151);

        ASSERT(in.next());
        EXPECT_EQUAL(get<long>(in, "paramId"), 30);

        EXPECT_NOT(in.next());
    }
}


CASE("GribMemoryInput") {
    auto buffer = read_file("regular_ll.2-2.grib2");
    ASSERT(!buffer.empty());

    input::GribMemoryInput memory(buffer.data(), buffer.size());
    MIRInput& in = memory;

    EXPECT_EQUAL(get<std::string>(in, "gridType"), "regular_ll");
    EXPECT_EQUAL(in.field().values(0).size(), 16380);

    EXPECT(in.sameAs(in));
    EXPECT_NOT(in.sameAs(input::GribMemoryInput(buffer.data(), buffer.size())));
    EXPECT_EQUAL(to_string(in), "GribMemoryInput[]");

    std::vector<char> garbage(100, 'x');
    EXPECT_THROWS(input::GribMemoryInput(garbage.data(), garbage.size()));
}


CASE("GribDataHandleInput") {
    const std::string path = "different-interpolation-methods.grib1";

    auto count = [](MIRInput& in) {
        size_t n = 0;
        while (in.next()) {
            ++n;
            EXPECT_EQUAL(in.field().values(0).size(), 16380);
        }
        return n;
    };

    {
        eckit::FileHandle handle(path);
        input::GribDataHandleInput in(handle);
        EXPECT_EQUAL(count(in), 4);
        EXPECT(static_cast<MIRInput&>(in).sameAs(in));
        EXPECT(to_string(static_cast<MIRInput&>(in)).find("GribDataHandleInput[handle=") == 0);
    }

    {
        eckit::FileHandle handle(path);
        input::GribDataHandleInput in(handle, 1, 2);
        EXPECT_EQUAL(count(in), 2);
    }

    {
        input::GribFileInput first(path);
        ASSERT(static_cast<MIRInput&>(first).next());
        auto offset = static_cast<off_t>(grib_long(static_cast<MIRInput&>(first).gribHandle(), "totalLength"));

        eckit::FileHandle handle(path);
        input::GribDataHandleInput in(handle, offset);
        EXPECT_EQUAL(count(in), 3);
    }
}


CASE("GribAllFileInput") {
    input::GribAllFileInput all("tiny-2x2.grib");
    MIRInput& in = all;

    EXPECT_EQUAL(in.dimensions(), 2);
    EXPECT(in.next());

    auto field = in.field();
    EXPECT_EQUAL(field.dimensions(), 2);
    EXPECT_EQUAL(field.values(0).size(), 4);
    EXPECT_EQUAL(field.values(1).size(), 4);

    long edition = 0;
    EXPECT(in.parametrisation(0).get("edition", edition) && edition == 1);
    EXPECT(in.parametrisation(1).get("edition", edition) && edition == 2);
    EXPECT(in.gribHandle(1) != nullptr);

    EXPECT(in.sameAs(input::GribAllFileInput("tiny-2x2.grib")));
    EXPECT_NOT(in.sameAs(input::GribFileInput("tiny-2x2.grib")));
    EXPECT_EQUAL(to_string(in), "GribAllFileInput[tiny-2x2.grib,size=2]");
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
