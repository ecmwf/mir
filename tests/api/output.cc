// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/TmpDir.h"
#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"

#include "mir/action/context/Context.h"
#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/input/GeoPointsFileInput.h"
#include "mir/input/GriddefInput.h"
#include "mir/input/MultiDimensionalInput.h"
#include "mir/input/RawInput.h"
#include "mir/input/VectorInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/output/GeoPointsFileOutputXYV.h"
#include "mir/output/GeoPointsFileOutputXYVector.h"
#include "mir/output/GmshOutput.h"
#include "mir/output/GriddefOutput.h"
#include "mir/output/MultiDimensionalOutput.h"
#include "mir/output/PNGOutput.h"
#include "mir/output/ValuesOutput.h"
#include "mir/output/VectorOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir::api::tests {


using output::MIROutput;
using output_t = std::unique_ptr<MIROutput>;


/// Input of a field on a grid (by its gridSpec), values by point index, with metadata
struct GridInput {
    explicit GridInput(const std::string& gridspec, size_t dimensions = 1) : grid(gridspec), metadata(grid) {
        auto n = grid.grid().size();
        for (size_t d = 0; d < dimensions; ++d) {
            for (size_t i = 0; i < n; ++i) {
                values.push_back(static_cast<double>(i + d * 100));
            }
        }

        if (dimensions > 1) {
            metadata.set("dimensions", static_cast<long>(dimensions));
        }

        input = std::make_unique<input::RawInput>(values.data(), n, metadata);
        ASSERT(input->next());
    }

    input::MIRInput& operator*() { return *input; }

    param::GridSpecParametrisation grid;
    param::RuntimeParametrisation metadata;
    std::vector<double> values;
    std::unique_ptr<input::MIRInput> input;
};


template <typename T>
std::string to_string(const T& p) {
    std::ostringstream str;
    str << p;
    return str.str();
}


std::vector<std::string> read_lines(const std::string& path) {
    std::ifstream in(path);
    std::vector<std::string> lines;
    for (std::string line; std::getline(in, line);) {
        lines.push_back(line);
    }
    return lines;
}


std::string read_bytes(const std::string& path, size_t n) {
    std::ifstream in(path, std::ios::binary);
    std::string bytes(n, '\0');
    in.read(&bytes[0], static_cast<std::streamsize>(n));
    return bytes;
}


CASE("MIROutputFactory") {
    eckit::TmpDir dir;

    auto build = [&dir](const std::string& name, const param::SimpleParametrisation& user) {
        const param::CombinedParametrisation param(user);
        return output_t(output::MIROutputFactory::build((dir / name).asString(), param));
    };

    struct test_t {
        std::string name;
        std::string format;
        std::string print;
    };


    SECTION("by format") {
        for (const auto& test : std::vector<test_t>{
                 {"a.grib", "grib", "GribFileOutput[path="},
                 {"a", "empty", "EmptyOutput[]"},
                 {"a", "geopoints", "GeoPointsFileOutput[path="},
                 {"a", "geopoints-xyv", "GeoPointsFileOutput[path="},
                 {"a", "geopoints-xy-vector", "GeoPointsFileOutput[path="},
                 {"a", "griddef", "GriddefOutput[path="},
                 {"a", "griddef-text", "GriddefOutput[path="},
                 {"a", "gmsh", "GmshOutput[path="},
                 {"a", "png", "PNGOutput[path="},
                 {"a.gpt", "extension", "GeoPointsFileOutput[path="},
             }) {
            param::SimpleParametrisation user;
            user.set("format", test.format);

            auto out = build(test.name, user);
            EXPECT(to_string(*out).find(test.print) == 0);
        }

        param::SimpleParametrisation user;
        user.set("format", "unknown");
        EXPECT_THROWS_AS(build("a", user), exception::SeriousBug);

        std::ostringstream list;
        output::MIROutputFactory::list(list);
        EXPECT(list.str().find("geopoints-xy-vector") != std::string::npos);
    }


    SECTION("by extension") {
        for (const auto& test : std::vector<test_t>{
                 {"a.grib", "", "GribFileOutput[path="},
                 {"a.unknown", "", "GribFileOutput[path="},
                 {"a", "", "GribFileOutput[path="},
                 {"a.gp", "", "GeoPointsFileOutput[path="},
                 {"a.gpt", "", "GeoPointsFileOutput[path="},
                 {"a.geo", "", "GeoPointsFileOutput[path="},
                 {"a.msh", "", "GmshOutput[path="},
                 {"a.png", "", "PNGOutput[path="},
             }) {
            auto out = build(test.name, param::SimpleParametrisation());
            EXPECT(to_string(*out).find(test.print) == 0);
        }
    }


    SECTION("geopoints (if griddef, latitudes or longitudes)") {
        for (const auto& key : std::vector<std::string>{"griddef", "latitudes", "longitudes"}) {
            param::SimpleParametrisation user;
            user.set(key, "x");

            auto out = build("a.grib", user);
            EXPECT(to_string(*out).find("GeoPointsFileOutput[path=") == 0);
        }
    }
}


CASE("EmptyOutput") {
    GridInput in("{grid: [10, 10]}");

    output::EmptyOutput out;
    MIROutput& base = out;

    MIRJob job;
    job.set("caching", false);
    EXPECT_NO_THROW(job.execute(*in, out));

    EXPECT(base.sameAs(output::EmptyOutput("path")));
    EXPECT_NOT(base.sameAs(output::ValuesOutput()));
    EXPECT_EQUAL(to_string(base), "EmptyOutput[]");

    param::SimpleParametrisation field;
    param::SimpleParametrisation user;
    const param::CombinedParametrisation param(user, field);
    EXPECT_NOT(base.sameParametrisation(param, param));

    std::ostringstream none;
    EXPECT_NOT(base.printParametrisation(none, param));
    EXPECT(none.str().empty());

    user.set("accuracy", 12L).set("packing", "ccsds").set("edition", 2L);

    std::ostringstream all;
    EXPECT(base.printParametrisation(all, param));
    EXPECT_EQUAL(all.str(), "accuracy=12,packing=ccsds,edition=2");

    std::ostringstream json;
    eckit::JSON j(json);
    j.startObject();
    base.jsonParametrisation(j, param);
    j.endObject();
    EXPECT_EQUAL(json.str(), R"({"accuracy":12,"packing":"ccsds","edition":2})");

    for (const auto& key : std::vector<std::string>{"packing", "edition"}) {
        param::SimpleParametrisation one;
        one.set(key, key == "packing" ? std::string("simple") : std::string("1"));
        const param::CombinedParametrisation p(one, field);

        std::ostringstream str;
        EXPECT(base.printParametrisation(str, p));
        EXPECT(str.str().find(key + "=") == 0);
    }
}


CASE("ValuesOutput") {
    GridInput in("{grid: [10, 10], area: [10, 0, -10, 20]}");

    output::ValuesOutput out;
    MIROutput& base = out;

    MIRJob job;
    job.set("caching", false);
    job.execute(*in, out);

    EXPECT_EQUAL(out.dimensions(), 1);
    EXPECT(out.values() == in.values);
    EXPECT_NOT(out.hasMissing());
    EXPECT_EQUAL(out.missingValue(), 9999.);
    EXPECT_THROWS_AS(out.values(1), eckit::AssertionFailed);

    EXPECT(base.sameAs(out));
    EXPECT_NOT(base.sameAs(output::ValuesOutput()));
    EXPECT_EQUAL(to_string(base), "ValuesOutput[]");

    param::SimpleParametrisation p;
    EXPECT(base.sameParametrisation(p, p));

    std::ostringstream str;
    EXPECT_NOT(base.printParametrisation(str, p));


    SECTION("missing values") {
        GridInput mv("{grid: [10, 10], area: [10, 0, -10, 20]}");
        mv.metadata.set("missing_value", 2.);

        output::ValuesOutput out2;
        job.execute(*mv, out2);
        EXPECT(out2.hasMissing());
        EXPECT_EQUAL(out2.missingValue(), 2.);
    }
}


CASE("GeoPointsFileOutputXYV") {
    eckit::TmpDir dir;
    auto path = (dir / "out.gpt").asString();

    GridInput in("{grid: [10, 10], area: [10, 0, 0, 20]}");
    in.metadata.set("stream", "oper");
    in.metadata.set("missing_value", 1.);

    MIRJob job;
    job.set("caching", false);


    SECTION("text") {
        {
            output::GeoPointsFileOutputXYV out(path);
            job.execute(*in, out);

            // only once
            EXPECT_THROWS_AS(job.execute(*in, out), exception::SeriousBug);
        }

        auto lines = read_lines(path);
        EXPECT(lines.size() >= 4);
        EXPECT_EQUAL(lines[0], "#GEO");
        EXPECT_EQUAL(lines[1], "#FORMAT XYV");
        EXPECT_EQUAL(lines[2], "# stream=oper");
        EXPECT_EQUAL(lines[3], "#DATA");

        // 2x3 points (lon lat value), missing value (1) skipped
        EXPECT_EQUAL(lines.size(), 4 + 6 - 1);
        EXPECT_EQUAL(lines[4], "0 10 0");
        EXPECT_EQUAL(lines[5], "20 10 2");

        // the file reads back
        input::GeoPointsFileInput geo(path);
        EXPECT_EQUAL(geo.values().size(), 5);
        EXPECT_EQUAL(geo.values().back(), 5.);
    }


    SECTION("binary") {
        {
            output::GeoPointsFileOutputXYV out(path, true);
            job.execute(*in, out);
        }

        // the file reads back (missing values included)
        input::GeoPointsFileInput geo(path);
        EXPECT_EQUAL(geo.values().size(), 6);
        EXPECT_EQUAL(geo.latitudes().front(), 10.);
        EXPECT_EQUAL(geo.longitudes().back(), 20.);
        EXPECT_EQUAL(geo.values().back(), 5.);

        input::MIRInput& base = geo;
        std::string stream;
        EXPECT(base.parametrisation().get("stream", stream) && stream == "oper");
        EXPECT(base.next());
        EXPECT_EQUAL(base.field().values(0).size(), 6);
    }


    SECTION("comparisons") {
        output::GeoPointsFileOutputXYV out(path);
        MIROutput& base = out;

        EXPECT(base.sameAs(output::GeoPointsFileOutputXYV(path)));
        EXPECT_NOT(base.sameAs(output::GeoPointsFileOutputXYV(path, true)));
        EXPECT_NOT(base.sameAs(output::EmptyOutput()));
        EXPECT_EQUAL(to_string(base), "GeoPointsFileOutput[path=" + path + "]");

        param::SimpleParametrisation p;
        EXPECT(base.sameParametrisation(p, p));

        std::ostringstream str;
        EXPECT_NOT(base.printParametrisation(str, p));
    }
}


CASE("GeoPointsFileOutputXYVector") {
    eckit::TmpDir dir;
    auto path = (dir / "out.gpt").asString();

    GridInput in("{grid: [10, 10], area: [10, 0, 0, 10]}", 2);

    MIRJob job;
    job.set("caching", false).set("date", "20250101");

    {
        output::GeoPointsFileOutputXYVector out(path);
        job.execute(*in, out);
    }

    auto lines = read_lines(path);
    EXPECT_EQUAL(lines.size(), 4 + 4);
    EXPECT_EQUAL(lines[1], "#FORMAT XY_VECTOR");
    EXPECT_EQUAL(lines[4], "10 0 0 20250101 0 0 100");
    EXPECT_EQUAL(lines[7], "0 10 0 20250101 0 3 103");

    // the file reads back (the last column as value)
    input::GeoPointsFileInput geo(path);
    EXPECT_EQUAL(geo.values().size(), 4);
    EXPECT_EQUAL(geo.values().front(), 100.);
}


CASE("GriddefOutput") {
    eckit::TmpDir dir;

    GridInput in("{grid: [10, 10], area: [10, 0, 0, 20]}");

    MIRJob job;
    job.set("caching", false);

    for (const auto& binary : {false, true}) {
        auto path = (dir / (binary ? "binary" : "text")).asString();

        {
            output_t out(binary ? static_cast<MIROutput*>(new output::GriddefBinaryOutput(path))
                                : new output::GriddefTextOutput(path));
            job.execute(*in, *out);

            MIROutput& base = *out;
            EXPECT(base.sameAs(output::GriddefTextOutput(path)));
            EXPECT(base.sameAs(output::GriddefBinaryOutput(path)));
            EXPECT_NOT(base.sameAs(output::GriddefTextOutput(path + "x")));

            param::SimpleParametrisation p;
            EXPECT_NOT(base.sameParametrisation(p, p));

            std::ostringstream str;
            EXPECT_NOT(base.printParametrisation(str, p));
            EXPECT_EQUAL(to_string(base), "GriddefOutput[path=" + path + ",binary=" + (binary ? "1" : "0") + "]");
        }

        std::vector<double> latitudes;
        std::vector<double> longitudes;
        input::GriddefInput::load(path, latitudes, longitudes);

        EXPECT(latitudes == std::vector<double>({10., 10., 10., 0., 0., 0.}));
        EXPECT(longitudes == std::vector<double>({0., 10., 20., 0., 10., 20.}));
    }

    auto lines = read_lines((dir / "text").asString());
    EXPECT_EQUAL(lines.size(), 6);
    EXPECT_EQUAL(lines[0], "10.000000 0.000000");
}


CASE("PNGOutput") {
    eckit::TmpDir dir;
    auto path = (dir / "out.png").asString();

    GridInput in("{grid: [10, 10], area: [10, 0, -10, 20]}");

    MIRJob job;
    job.set("caching", false);

    const std::string magic("\x89PNG", 4);


    SECTION("encoders") {
        size_t i = 0;
        for (const auto& encoder :
             std::vector<std::string>{"8-bit/g", "8-bit/ga", "16-bit/g", "16-bit/ga", "8-bit/rgb", "8-bit/rgba"}) {
            auto p = (dir / ("out." + std::to_string(i++) + ".png")).asString();

            MIRJob j;
            j.set("caching", false).set("png-output-encoder", encoder).set("png-output-minmax", 0., 8.);

            output::PNGOutput out(p);
            j.execute(*in, out);

            EXPECT_EQUAL(read_bytes(p, 4), magic);
        }
    }


    SECTION("missing values, existing files") {
        in.metadata.set("missing_value", 0.);

        for (size_t i = 0; i < 2; ++i) {
            output::PNGOutput out(path);
            job.execute(*in, out);
        }

        EXPECT_EQUAL(read_bytes(path, 4), magic);
        EXPECT_EQUAL(read_bytes(path + ".0001", 4), magic);
    }


    SECTION("comparisons, errors") {
        output::PNGOutput out(path);
        MIROutput& base = out;

        EXPECT(base.sameAs(out));
        EXPECT_NOT(base.sameAs(output::PNGOutput(path)));
        EXPECT_EQUAL(to_string(base), "PNGOutput[path=" + path + "]");

        param::SimpleParametrisation p;
        EXPECT(base.sameParametrisation(p, p));

        std::ostringstream str;
        EXPECT_NOT(base.printParametrisation(str, p));

        MIRJob unknown;
        unknown.set("caching", false).set("png-output-encoder", "unknown");
        EXPECT_THROWS_AS(unknown.execute(*in, out), exception::SeriousBug);

        GridInput o16("{grid: O16}");
        EXPECT_THROWS_AS(job.execute(*o16, out), exception::UserError);
    }
}


CASE("GmshOutput") {
    eckit::TmpDir dir;
    auto path = (dir / "out.msh").asString();

    GridInput in("{grid: [30, 30]}");

    MIRJob job;
    job.set("caching", false);

    {
        output::GmshOutput out(path);
        job.execute(*in, out);
    }

    auto lines = read_lines(path);
    EXPECT(!lines.empty());
    EXPECT_EQUAL(lines[0], "$MeshFormat");

    // without overwriting, a new file
    {
        MIRJob j;
        j.set("caching", false).set("output", "{overwrite: false, write_mesh: false}");

        output::GmshOutput out(path);
        j.execute(*in, out);
    }

    EXPECT(!read_lines((dir / "out.0001.msh").asString()).empty());

    output::GmshOutput out(path);
    MIROutput& base = out;
    EXPECT(base.sameAs(output::GmshOutput(path)));
    EXPECT_NOT(base.sameAs(output::GmshOutput(path + "x")));
    EXPECT_EQUAL(to_string(base), "GmshOutput[path=" + path + "]");

    param::SimpleParametrisation p;
    EXPECT_NOT(base.sameParametrisation(p, p));

    std::ostringstream str;
    EXPECT_NOT(base.printParametrisation(str, p));
}


CASE("VectorOutput") {
    GridInput u("{grid: [10, 10], area: [10, 0, 0, 10]}");
    GridInput v("{grid: [10, 10], area: [10, 0, 0, 10]}");
    for (auto& x : v.values) {
        x += 100.;
    }

    input::VectorInput vector(*u.input, *v.input);
    util::MIRStatistics statistics;

    output::ValuesOutput out1;
    output::ValuesOutput out2;
    output::VectorOutput out(out1, out2);
    MIROutput& base = out;

    param::SimpleParametrisation user;
    const param::CombinedParametrisation param(user);


    SECTION("save") {
        context::Context ctx(vector, statistics);
        EXPECT_EQUAL(ctx.field().dimensions(), 2);

        base.save(param, ctx);
        EXPECT(out1.values() == u.values);
        EXPECT(out2.values() == v.values);
    }


    SECTION("copy, set") {
        context::Context ctx1(vector, statistics);
        base.copy(param, ctx1);
        EXPECT(out1.values() == u.values);
        EXPECT(out2.values() == v.values);

        output::ValuesOutput out3;
        output::ValuesOutput out4;
        output::VectorOutput another(out3, out4);

        context::Context ctx2(vector, statistics);
        static_cast<MIROutput&>(another).set(param, ctx2);
        EXPECT(out4.values() == v.values);
    }


    SECTION("not a vector input") {
        context::Context ctx(*u.input, statistics);
        EXPECT_THROWS_AS(base.copy(param, ctx), exception::SeriousBug);
        EXPECT_THROWS_AS(base.set(param, ctx), exception::SeriousBug);

        GridInput two("{grid: [10, 10], area: [10, 0, 0, 10]}", 2);
        context::Context ctx2(*two.input, statistics);
        EXPECT_THROWS_AS(base.save(param, ctx2), exception::SeriousBug);
    }


    SECTION("comparisons") {
        output::VectorOutput same(out1, out2);
        output::VectorOutput swapped(out2, out1);
        EXPECT(base.sameAs(same));
        EXPECT_NOT(base.sameAs(swapped));
        EXPECT_NOT(base.sameAs(out1));

        EXPECT(base.sameParametrisation(param, param));

        std::ostringstream str;
        EXPECT_NOT(base.printParametrisation(str, param));

        std::ostringstream json;
        eckit::JSON j(json);
        j.startObject();
        base.jsonParametrisation(j, param);
        j.endObject();
        EXPECT_EQUAL(json.str(), "{}");

        EXPECT_EQUAL(to_string(base), "VectorOutput[ValuesOutput[],ValuesOutput[]]");
    }
}


CASE("MultiDimensionalOutput") {
    GridInput a("{grid: [10, 10], area: [10, 0, 0, 10]}");
    GridInput b("{grid: [10, 10], area: [10, 0, 0, 10]}");
    for (auto& x : b.values) {
        x += 100.;
    }

    // components are owned
    auto* multi = new input::MultiDimensionalInput;
    std::unique_ptr<input::MIRInput> input(multi);
    multi->append(new input::RawInput(a.values.data(), a.values.size(), a.metadata));
    multi->append(new input::RawInput(b.values.data(), b.values.size(), b.metadata));
    ASSERT(input->next());

    auto* out1 = new output::ValuesOutput;
    auto* out2 = new output::ValuesOutput;
    output::MultiDimensionalOutput out;
    out.appendDimensionalOutput(out1);
    out.appendDimensionalOutput(out2);
    MIROutput& base = out;

    util::MIRStatistics statistics;
    param::SimpleParametrisation user;
    const param::CombinedParametrisation param(user);


    SECTION("save, copy, set") {
        context::Context ctx(*input, statistics);
        EXPECT_EQUAL(ctx.field().dimensions(), 2);
        base.save(param, ctx);
        EXPECT(out1->values() == a.values);
        EXPECT(out2->values() == b.values);

        context::Context ctx1(*input, statistics);
        base.copy(param, ctx1);
        EXPECT(out2->values() == b.values);

        context::Context ctx2(*input, statistics);
        base.set(param, ctx2);
        EXPECT(out1->values() == a.values);
    }


    SECTION("not a multi-dimensional input") {
        context::Context ctx(*a.input, statistics);
        EXPECT_THROWS_AS(base.copy(param, ctx), exception::SeriousBug);
        EXPECT_THROWS_AS(base.save(param, ctx), exception::SeriousBug);
        EXPECT_THROWS_AS(base.set(param, ctx), exception::SeriousBug);
    }


    SECTION("comparisons") {
        EXPECT(base.sameParametrisation(param, param));

        std::ostringstream str;
        EXPECT_NOT(base.printParametrisation(str, param));

        std::ostringstream json;
        eckit::JSON j(json);
        j.startObject();
        base.jsonParametrisation(j, param);
        j.endObject();
        EXPECT_EQUAL(json.str(), "{}");

        output::MultiDimensionalOutput shorter;
        shorter.appendDimensionalOutput(new output::ValuesOutput);
        EXPECT_NOT(base.sameAs(shorter));
        EXPECT_NOT(base.sameAs(*out1));

        EXPECT(to_string(base).find("MultiDimensionalOutput[") == 0);
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
