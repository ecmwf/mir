// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/TmpDir.h"
#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/ArtificialInput.h"
#include "mir/input/EmptyInput.h"
#include "mir/input/GeoPointsFileInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/GridSpecInput.h"
#include "mir/input/GriddefInput.h"
#include "mir/input/MultiDimensionalInput.h"
#include "mir/input/MultiStreamInput.h"
#include "mir/input/VectorInput.h"
#include "mir/output/GriddefOutput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/ValueMap.h"


namespace mir::api::tests {


using input::MIRInput;
using input_t = std::unique_ptr<MIRInput>;


/// Input built as the mir tool does, from a path and the --input option
input_t build(const std::string& path, const std::string& input = "") {
    param::SimpleParametrisation param;
    if (!input.empty()) {
        param.set("input", input);
    }
    return input_t(input::MIRInputFactory::build(path, param));
}


template <typename T>
std::string to_string(const T& p) {
    std::ostringstream str;
    str << p;
    return str.str();
}


void write_file(const std::string& path, const std::string& contents) {
    std::ofstream out(path);
    out << contents;
}


size_t number_of_values(const data::MIRField& field) {
    repres::RepresentationHandle repres(field.representation());
    return repres->numberOfValues();
}


CASE("MIRInput (base class)") {
    struct TestingInput final : MIRInput {
        const param::MIRParametrisation& parametrisation(size_t /*which*/) const override { NOTIMP; }
        bool sameAs(const MIRInput& /*unused*/) const override { return false; }
        data::MIRField field() const override { NOTIMP; }
        void print(std::ostream& out) const override { out << "TestingInput[]"; }
    } input;

    MIRInput& base = input;

    EXPECT_THROWS_AS(base.next(), exception::SeriousBug);
    EXPECT_THROWS_AS(base.only(167), exception::SeriousBug);
    EXPECT_THROWS_AS(base.dimensions(), exception::SeriousBug);
    EXPECT_THROWS_AS(base.copy(nullptr, 0), exception::SeriousBug);
    EXPECT_THROWS_AS(base.setAuxiliaryInformation(util::ValueMap()), exception::SeriousBug);

    // a GRIB sample handle, the same every time
    auto* h = base.gribHandle();
    EXPECT(h != nullptr);
    EXPECT(base.gribHandle() == h);

    EXPECT_EQUAL(to_string(base), "TestingInput[]");
}


CASE("MIRInputFactory") {
    eckit::TmpDir dir;


    SECTION("by file magic number") {
        auto grib = build("regular_ll.2-2.grib1");
        EXPECT(to_string(*grib).find("GribFileInput[") == 0);

        auto geo = build("geopoints-format-xyv.gpt");
        EXPECT(to_string(*geo).find("GeoPointsFileInput[") == 0);

        // unknown, assuming GRIB (but not finding any)
        auto path = (dir / "unknown").asString();
        write_file(path, "unknown contents");

        auto unknown = build(path);
        EXPECT(to_string(*unknown).find("GribFileInput[") == 0);
        EXPECT_NOT(unknown->next());

        std::ostringstream list;
        input::MIRInputFactory::list(list);
        EXPECT(list.str().find("0x47524942 (GRIB)") != std::string::npos);
        EXPECT(list.str().find("0x2347454f (#GEO)") != std::string::npos);
    }


    SECTION("by the input option (YAML, JSON or a file)") {
        auto yml  = (dir / "input.yml").asString();
        auto yaml = (dir / "input.yaml").asString();
        auto json = (dir / "input.json").asString();

        write_file(yml, "artificialInput: dummy\n");
        write_file(yaml, "artificialInput: dummy\n");
        write_file(json, R"({"artificialInput": "dummy"})");

        for (const auto& input : std::vector<std::string>{"{artificialInput: dummy}", yml, yaml, json}) {
            auto in = build("/dev/null", input);
            EXPECT(in->next());
            EXPECT(to_string(*in).find("ArtificialInput[") == 0);
            EXPECT_EQUAL(in->field().values(0).size(), 360 * 181);
        }

        EXPECT_THROWS_AS(build("/dev/null", "{artificialInput: unknown}"), exception::UserError);
    }


    SECTION("multi-dimensional input") {
        for (const auto& input : std::vector<std::string>{"{multiDimensional: 2}", ""}) {
            param::SimpleParametrisation param;
            if (input.empty()) {
                param.set("uv2uv", true);
            }
            else {
                param.set("input", input);
            }

            input_t in(input::MIRInputFactory::build("tiny-2x2.grib", param));
            EXPECT(to_string(*in).find("MultiDimensionalInput[GribFileInput[") == 0);

            EXPECT(in->next());
            EXPECT_EQUAL(in->dimensions(), 2);

            auto field = in->field();
            EXPECT_EQUAL(field.dimensions(), 2);
            EXPECT_EQUAL(field.values(0).size(), 4);
            EXPECT_EQUAL(field.values(1).size(), 4);

            EXPECT_NOT(in->next());
        }

        param::SimpleParametrisation both;
        both.set("uv2uv", true).set("vod2uv", true);
        EXPECT_THROWS_AS(input::MIRInputFactory::build("tiny-2x2.grib", both), eckit::AssertionFailed);
    }
}


CASE("ArtificialInput") {
    const std::string regular_ll =
        "gridded: true, gridType: regular_ll, north: 10, west: 0, south: 0, east: 10, west_east_increment: 1, "
        "south_north_increment: 1, Ni: 11, Nj: 11";


    SECTION("constant") {
        auto in = build("/dev/null", "{artificialInput: constant, constant: 42, " + regular_ll + "}");
        EXPECT(in->next());
        EXPECT_NOT(in->next());
        EXPECT_EQUAL(in->dimensions(), 1);

        auto field = in->field();
        EXPECT_EQUAL(field.values(0).size(), 121);
        for (const auto& v : field.values(0)) {
            EXPECT_EQUAL(v, 42.);
        }

        // additional keys (grid, paramId and numberOfDataPoints)
        const auto& param = in->parametrisation();

        std::vector<double> grid;
        EXPECT(param.get("grid", grid) && grid == std::vector<double>({1., 1.}));

        long paramId = 0;
        EXPECT(param.get("paramId", paramId) && paramId == 255);

        // the same input
        auto same    = build("/dev/null", "{artificialInput: constant, constant: 42, " + regular_ll + "}");
        auto another = build("/dev/null", "{artificialInput: constant, constant: 1, " + regular_ll + "}");
        EXPECT(in->sameAs(*same));
        EXPECT_NOT(in->sameAs(*another));
        EXPECT_NOT(in->sameAs(*build("regular_ll.2-2.grib1")));

        EXPECT(to_string(*in).find("ArtificialInput[parametrisation={") == 0);
    }


    SECTION("constant, rotated") {
        auto in = build("/dev/null",
                        "{artificialInput: constant, constant: 1, gridded: true, gridType: rotated_ll, north: 10, "
                        "west: 0, south: 0, east: 10, west_east_increment: 1, south_north_increment: 1, Ni: 11, "
                        "Nj: 11, south_pole_latitude: -40, south_pole_longitude: 22, south_pole_rotation_angle: 0}");
        EXPECT(in->next());

        std::vector<double> rotation;
        EXPECT(in->parametrisation().get("rotation", rotation));
        EXPECT(rotation == std::vector<double>({-40., 22.}));

        EXPECT_EQUAL(in->field().values(0).size(), 121);
    }


    SECTION("distribution") {
        auto in = build("/dev/null",
                        R"({artificialInput: distribution, distribution: "uniform-int-distribution{a: 1, b: 3}", )" +
                            regular_ll + "}");
        EXPECT(in->next());

        auto field         = in->field();
        const auto& values = field.values(0);
        EXPECT_EQUAL(values.size(), 121);
        for (const auto& v : values) {
            EXPECT(v == 1. || v == 2. || v == 3.);
        }
    }


    SECTION("dummy") {
        auto in = build("/dev/null", "{artificialInput: dummy}");
        EXPECT(in->next());

        auto field         = in->field();
        const auto& values = field.values(0);
        EXPECT_EQUAL(values.size(), 360 * 181);
        EXPECT(std::abs(values[0] - 1.) < 1e-12);  // sin(0) + cos(0)
    }


    SECTION("gridspec") {
        auto in = build("/dev/null", "{artificialInput: gridspec, gridSpec: {grid: O16}}");
        EXPECT(in->next());

        auto field = in->field();
        EXPECT_EQUAL(field.values(0).size(), 1600);
        EXPECT(std::all_of(field.values(0).begin(), field.values(0).end(), [](double v) { return v == 0.; }));

        std::string gridType;
        EXPECT(in->parametrisation().get("gridType", gridType) && gridType == "reduced_gg");

        // the key as before (gridspec), the same input; none, an error
        auto before = build("/dev/null", "{artificialInput: gridspec, gridspec: {grid: O16}}");
        EXPECT(before->next());
        EXPECT_EQUAL(before->field().values(0).size(), 1600);

        EXPECT_THROWS_AS(build("/dev/null", "{artificialInput: gridspec}"), exception::UserError);

        // spectral
        input_t sh(new input::GridSpecInput("{grid: T21}", false));
        EXPECT(sh->next());

        bool spectral = false;
        EXPECT(sh->parametrisation().get("spectral", spectral) && spectral);
        auto coefficients = sh->field();
        EXPECT_EQUAL(coefficients.values(0).size(), number_of_values(coefficients));
    }


    SECTION("unstructured grid, by griddef") {
        auto in = build("/dev/null",
                        "{artificialInput: constant, constant: 1, gridded: true, gridType: unstructured_grid, griddef: "
                        "tour.pts}");
        EXPECT(in->next());

        long n = 0;
        EXPECT(in->parametrisation().get("numberOfDataPoints", n) && n == 34);

        std::vector<double> latitudes;
        EXPECT(in->parametrisation().get("latitudes", latitudes) && latitudes.size() == 34);

        EXPECT_EQUAL(in->field().values(0).size(), 34);
    }
}


CASE("EmptyInput") {
    input_t in(new input::EmptyInput);
    EXPECT(in->next());
    EXPECT_NOT(in->next());

    std::string gridType;
    EXPECT(in->parametrisation().get("gridType", gridType) && gridType == "none");

    EXPECT(in->sameAs(input::EmptyInput()));
    EXPECT_NOT(in->sameAs(*build("/dev/null", "{artificialInput: dummy}")));
    EXPECT_EQUAL(to_string(*in), "EmptyInput[...]");
    EXPECT_NO_THROW(in->field());
}


CASE("GriddefInput") {
    SECTION("text") {
        input_t in(new input::GriddefInput("tour.pts"));
        EXPECT(in->next());
        EXPECT_NOT(in->next());
        EXPECT_EQUAL(in->dimensions(), 1);

        bool gridded = false;
        EXPECT(in->parametrisation().get("gridded", gridded) && gridded);

        repres::RepresentationHandle repres(in->field().representation());
        EXPECT_EQUAL(repres->numberOfPoints(), 34);

        EXPECT_NOT(in->sameAs(*in));
        EXPECT_EQUAL(to_string(*in), "GriddefInput[size=34]");
    }


    SECTION("binary") {
        eckit::TmpDir dir;
        auto path = dir / "griddef";

        const std::vector<double> lats{10., 20., 30.};
        const std::vector<double> lons{1., 2., 3.};
        output::GriddefOutput::save(path, lats, lons, true);

        std::vector<double> latitudes;
        std::vector<double> longitudes;
        input::GriddefInput::load(path, latitudes, longitudes);
        EXPECT(latitudes == lats);
        EXPECT(longitudes == lons);

        input_t in(new input::GriddefInput(path));
        EXPECT_EQUAL(to_string(*in), "GriddefInput[size=3]");
    }


    SECTION("errors") {
        std::vector<double> latitudes;
        std::vector<double> longitudes;
        EXPECT_THROWS_AS(input::GriddefInput::load("does-not-exist.pts", latitudes, longitudes),
                         exception::CantOpenFile);
    }
}


CASE("GeoPointsFileInput") {
    struct test_t {
        std::string path;
        size_t size;
        double lat;
        double lon;
        double value;
    };


    SECTION("text formats") {
        for (const auto& test : std::vector<test_t>{
                 {"geopoints-standard.gpt", 0, 36.15, -5.35, 300.9},
                 {"geopoints-format-xyv.gpt", 0, 36.15, -5.35, 300.9},
                 {"geopoints-format-ncols.gpt", 0, 32.55, 35.85, 75.},
                 {"geopoints-format-xy-vector.gpt", 0, 80., 0., -8.3126},
                 {"geopoints-format-polar-vector.gpt", 3, 50.97, 6.05, 90.},
             }) {
            Log::info() << "'" << test.path << "'" << std::endl;

            input::GeoPointsFileInput geo(test.path);
            MIRInput& in = geo;

            EXPECT(in.next());
            EXPECT_EQUAL(in.dimensions(), 1);
            EXPECT_NOT(in.next());

            EXPECT(!geo.latitudes().empty());
            EXPECT_EQUAL(geo.latitudes().size(), geo.longitudes().size());
            EXPECT_EQUAL(geo.latitudes().size(), geo.values().size());
            if (test.size > 0) {
                EXPECT_EQUAL(geo.values().size(), test.size);
            }

            EXPECT_EQUAL(geo.latitudes().front(), test.lat);
            EXPECT_EQUAL(geo.longitudes().front(), test.lon);
            EXPECT_EQUAL(geo.values().front(), test.value);
            EXPECT(geo.footprint() > 0);

            std::string gridType;
            EXPECT(in.parametrisation().get("gridType", gridType) && gridType == "unstructured_grid");

            auto field = in.field();
            EXPECT(field.values(0) == geo.values());

            EXPECT(in.sameAs(*build(test.path)));
            EXPECT(to_string(in).find("GeoPointsFileInput[path=" + test.path) == 0);
        }

        // metadata
        input::GeoPointsFileInput geo("geopoints-standard.gpt");
        auto metadata = to_string(static_cast<MIRInput&>(geo).parametrisation());
        Log::info() << "metadata: " << metadata << std::endl;
        EXPECT(metadata.find("12004") != std::string::npos);
    }


    SECTION("multiple fields") {
        eckit::TmpDir dir;
        auto path = (dir / "multi.gpt").asString();
        write_file(path,
                   "#GEO\n#FORMAT XYV\n#DATA\n1 2 3\n4 5 6\n"
                   "#GEO\n#FORMAT XYV\n#DATA\n7 8 9\n");

        EXPECT_THROWS_AS(input::GeoPointsFileInput{path}, exception::SeriousBug);
        EXPECT_THROWS_AS(input::GeoPointsFileInput(path, 2), exception::SeriousBug);

        input::GeoPointsFileInput first(path, 0);
        EXPECT_EQUAL(first.values().size(), 2);
        EXPECT_EQUAL(first.values().front(), 3.);

        input::GeoPointsFileInput second(path, 1);
        EXPECT_EQUAL(second.values().size(), 1);
        EXPECT_EQUAL(second.values().front(), 9.);
    }


    SECTION("errors") {
        eckit::TmpDir dir;
        auto path = (dir / "empty.gpt").asString();
        write_file(path, "#NOTHING\n");

        EXPECT_THROWS_AS(input::GeoPointsFileInput{path}, exception::SeriousBug);
        EXPECT_THROWS(input::GeoPointsFileInput{"does-not-exist.gpt"});
    }
}


CASE("NetcdfFileInput") {
    auto in = build("nemo.nc", "checkDuplicatePoints: False");
    EXPECT(to_string(*in).find("NetcdfFileInput[") == 0);

    size_t count = 0;
    while (in->next()) {
        ++count;
        EXPECT(in->dimensions() > 0);

        const auto& param = in->parametrisation();
        Log::info() << "NetcdfFileInput: " << param << std::endl;

        // values from the field, or from the field rules
        bool b = false;
        EXPECT_NO_THROW(param.get("gridded", b));

        std::vector<double> grid;
        EXPECT_NO_THROW(param.get("grid", grid));

        bool checkDuplicatePoints = true;
        EXPECT(param.get("checkDuplicatePoints", checkDuplicatePoints) && !checkDuplicatePoints);

        auto field = in->field();
        EXPECT_EQUAL(field.dimensions(), in->dimensions());
        EXPECT_EQUAL(field.values(0).size(), number_of_values(field));
    }

    EXPECT(count > 0);
    EXPECT(in->sameAs(*build("nemo.nc")));
}


CASE("VectorInput") {
    input::GribFileInput u("tiny-2x2.grib", 0, 2);
    input::GribFileInput v("tiny-2x2.grib", 1, 2);
    input::VectorInput vector(u, v);

    MIRInput& in = vector;
    EXPECT(in.next());
    EXPECT_EQUAL(in.dimensions(), 2);

    auto field = in.field();
    EXPECT_EQUAL(field.dimensions(), 2);
    EXPECT_EQUAL(field.values(0).size(), 4);
    EXPECT_EQUAL(field.values(1).size(), 4);

    // each component has its own parametrisation and handle
    long edition0 = 0;
    long edition1 = 0;
    EXPECT(in.parametrisation(0).get("edition", edition0) && edition0 == 1);
    EXPECT(in.parametrisation(1).get("edition", edition1) && edition1 == 2);
    EXPECT(in.gribHandle(0) != in.gribHandle(1));

    input::GribFileInput u2("tiny-2x2.grib", 0, 2);
    input::GribFileInput v2("tiny-2x2.grib", 1, 2);
    EXPECT(in.sameAs(input::VectorInput(u2, v2)));
    EXPECT_NOT(in.sameAs(u));

    EXPECT(to_string(in).find("VectorInput[GribFileInput[") == 0);
    EXPECT_NOT(in.next());
}


CASE("MultiDimensionalInput") {
    struct TestingInput : MIRInput {
        const param::MIRParametrisation& parametrisation(size_t /*which*/) const override { NOTIMP; }
        bool sameAs(const MIRInput& /*unused*/) const override { return false; }
        data::MIRField field() const override { NOTIMP; }
        void print(std::ostream& /*unused*/) const override { NOTIMP; }
    };

    struct TestingMultiDimensionalInput final : TestingInput {
        TestingMultiDimensionalInput(size_t Nfields, size_t Ndim) {
            for (size_t n = Nfields; n > 0; n -= Ndim) {
                if (n < Ndim) {
                    dims_.insert(dims_.begin(), n);
                    break;
                }
                dims_.push_back(Ndim);
            }
            dims_.push_back(0);
        }

    private:
        bool next() override {
            dims_.pop_back();
            return !dims_.empty();
        }

        size_t dimensions() const override {
            ASSERT(!dims_.empty());
            return dims_.back();
        }

        std::vector<size_t> dims_;
    };

    struct TestingSingleDimensionalInput final : TestingInput {
        explicit TestingSingleDimensionalInput(size_t Nfields) : Nfields_(Nfields + 1) {}

    private:
        bool next() override { return --Nfields_ != 0; }
        size_t dimensions() const override { return 1; }
        size_t Nfields_;
    };


    SECTION("next(), dimensions()") {
        const std::vector<size_t> _numberOfFields{1, 2, 3, 4, 5};
        const std::vector<size_t> _numberOfDimensions{1, 2, 3};

        for (const auto& numberOfFields : _numberOfFields) {
            for (const auto& numberOfDimensions : _numberOfDimensions) {
                auto numberOfNext = (numberOfFields - 1) / numberOfDimensions + 1;
                ASSERT(numberOfNext >= 1);

                input_t input(new TestingMultiDimensionalInput(numberOfFields, numberOfDimensions));

                size_t Nfields = 0;
                size_t Nnext   = 0;
                for (; input->next(); Nnext++) {
                    auto Ndims = input->dimensions();
                    Nfields += Ndims;

                    auto numberOfDims =
                        Nfields == numberOfFields ? (numberOfFields - 1) % numberOfDimensions + 1 : numberOfDimensions;
                    EXPECT(numberOfFields >= Nfields);
                    EXPECT(numberOfDims == Ndims);
                }

                EXPECT(numberOfFields == Nfields);
                EXPECT(numberOfNext == Nnext);
            }
        }
    }


    SECTION("empty") {
        input_t input(new input::MultiDimensionalInput);
        EXPECT(!input->next());
    }


    SECTION("append() different-sized fields") {
        auto* multi = new input::MultiDimensionalInput;
        input_t input(multi);
        multi->append(new TestingSingleDimensionalInput(4));
        multi->append(new TestingSingleDimensionalInput(2));
        multi->append(new TestingSingleDimensionalInput(4));

        EXPECT(input->next());
        EXPECT(input->dimensions() == 3);
        EXPECT(input->next());
        EXPECT(input->dimensions() == 3);
        EXPECT(input->next());
        EXPECT(input->dimensions() == 2);
        EXPECT(input->next());
        EXPECT(input->dimensions() == 2);
        EXPECT(!input->next());
    }


    SECTION("append() include empty-sized fields") {
        auto* multi = new input::MultiDimensionalInput;
        input_t input(multi);
        multi->append(new TestingSingleDimensionalInput(2));
        multi->append(new TestingSingleDimensionalInput(0));
        multi->append(new TestingSingleDimensionalInput(3));

        EXPECT(input->next());
        EXPECT(input->dimensions() == 2);
        EXPECT(input->next());
        EXPECT(input->dimensions() == 2);
        EXPECT(input->next());
        EXPECT(input->dimensions() == 1);
        EXPECT(!input->next());
    }


    SECTION("GRIB components") {
        auto make = []() {
            auto* multi = new input::MultiDimensionalInput;
            multi->append(new input::GribFileInput("tiny-3x3.grib", 0, 2));
            multi->append(new input::GribFileInput("tiny-3x3.grib", 1, 2));
            return input_t(multi);
        };

        auto in = make();
        EXPECT(in->next());
        EXPECT_EQUAL(in->dimensions(), 2);

        auto field = in->field();
        EXPECT_EQUAL(field.dimensions(), 2);
        EXPECT_EQUAL(field.values(1).size(), 9);

        long edition = 0;
        EXPECT(in->parametrisation(1).get("edition", edition) && edition == 2);
        EXPECT(in->gribHandle(1) != nullptr);
        EXPECT_THROWS(in->parametrisation(2));

        EXPECT_NO_THROW(in->setAuxiliaryInformation(util::ValueMap()));

        EXPECT(in->sameAs(*make()));
        EXPECT_NOT(in->sameAs(*build("tiny-3x3.grib")));

        auto other = std::make_unique<input::MultiDimensionalInput>();
        other->append(new input::GribFileInput("tiny-3x3.grib", 1, 2));
        other->append(new input::GribFileInput("tiny-3x3.grib", 0, 2));
        EXPECT_NOT(in->sameAs(*other));

        EXPECT(to_string(*in).find("MultiDimensionalInput[GribFileInput[") == 0);
    }
}


CASE("MultiStreamInput") {
    auto* multi = new input::MultiStreamInput;
    input_t in(multi);

    // components are not owned
    input::GribFileInput a("tiny-2x2.grib");
    input::GribFileInput b("tiny-3x3.grib");
    multi->append(&a);
    multi->append(&b);

    // the first stream's dimensions and parametrisation
    MIRInput& first = a;
    EXPECT(first.next());
    EXPECT_EQUAL(in->dimensions(), 1);
    EXPECT(&in->parametrisation() == &first.parametrisation());
    EXPECT(in->gribHandle() == first.gribHandle());
    EXPECT_EQUAL(in->field().values(0).size(), 4);

    auto* same = new input::MultiStreamInput;
    input_t other(same);
    same->append(&a);
    same->append(&b);
    EXPECT(in->sameAs(*other));

    auto* shorter = new input::MultiStreamInput;
    input_t another(shorter);
    shorter->append(&a);
    EXPECT_NOT(in->sameAs(*another));

    EXPECT(to_string(*in).find("MultiStreamInput[GribFileInput[") == 0);
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
