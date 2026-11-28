// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cmath>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/TmpDir.h"
#include "eckit/log/JSON.h"
#include "eckit/testing/Test.h"

#include "mir/action/io/Save.h"
#include "mir/action/plan/ActionGraph.h"
#include "mir/action/plan/Job.h"
#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/grib/Packing.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/GribMemoryInput.h"
#include "mir/input/MultiDimensionalGribFileInput.h"
#include "mir/input/RawInput.h"
#include "mir/input/VectorInput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/output/GribMemoryOutput.h"
#include "mir/output/MultiDimensionalOutput.h"
#include "mir/output/VectorOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir::api::tests {


using input::MIRInput;
using output::MIROutput;
using input_t = std::unique_ptr<MIRInput>;


template <typename T>
std::string to_string(const T& p) {
    std::ostringstream str;
    str << p;
    return str.str();
}


std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}


/// GRIB messages, as written to memory (one at a time)
struct Memory {
    Memory() : buffer(4 * 1024 * 1024), output(buffer.data(), buffer.size()) {}

    /// The message written, as an input
    input_t message() const { return input_t(new input::GribMemoryInput(buffer.data(), output.length())); }

    std::vector<char> buffer;
    output::GribMemoryOutput output;
};


/// GRIB messages in a file (copies, to read with GribMemoryInput)
std::vector<std::string> messages(const std::string& path) {
    std::vector<std::string> result;

    input_t in(new input::GribFileInput(path));
    while (in->next()) {
        const void* message = nullptr;
        size_t size         = 0;
        GRIB_CALL(codes_get_message(in->gribHandle(), &message, &size));

        result.emplace_back(static_cast<const char*>(message), size);
    }

    return result;
}


long grib_long(const MIRInput& in, const std::string& key) {
    long value = 0;
    GRIB_CALL(codes_get_long(in.gribHandle(), key.c_str(), &value));
    return value;
}


std::string grib_string(const MIRInput& in, const std::string& key) {
    char value[1024];
    size_t length = sizeof(value);
    GRIB_CALL(codes_get_string(in.gribHandle(), key.c_str(), value, &length));
    return value;
}


/// Input of a field on a grid (by its gridSpec), values by point index
struct GridInput {
    explicit GridInput(const std::string& gridspec) : grid(gridspec), metadata(grid) {
        for (size_t i = 0; i < grid.grid().size(); ++i) {
            values.push_back(static_cast<double>(i));
        }
        input = std::make_unique<input::RawInput>(values.data(), values.size(), metadata);
        ASSERT(input->next());
    }

    param::GridSpecParametrisation grid;
    param::RuntimeParametrisation metadata;
    std::vector<double> values;
    input_t input;
};


CASE("GribOutput: copy, set or save (action plan)") {
    // setup input/output
    param::SimpleParametrisation field;
    field.set("input",
              "{"
              "artificialInput:constant,"
              "constant:0.,"
              "spectral:true,"
              "truncation:1279,"
              "gridType:sh,"
              "packing:spectral_complex,"
              "edition:2,"
              "accuracy:24"
              "}");

    input_t in(input::MIRInputFactory::build("constant", field));
    std::unique_ptr<MIROutput> out(new output::GribFileOutput(""));

    auto graph_dump = [](MIRInput& in, MIROutput& out, const MIRJob& mj1, const MIRJob& mj2) {
        // prepare ActionPlan(s)
        std::unique_ptr<action::Job> aj1(new action::Job(mj1, in, out, false));
        std::unique_ptr<action::Job> aj2(new action::Job(mj2, in, out, false));

        // coallesce ActionPlan(s)
        action::ActionGraph graph;
        graph.add(aj1->plan(), nullptr);
        graph.add(aj2->plan(), nullptr);

        std::ostringstream ss;
        graph.dump(ss, 1);
        return ss.str();
    };


    SECTION("GribOutput::copy()") {
        MIRJob mj1;
        MIRJob mj2;

        auto result = graph_dump(*in, *out, mj1, mj2);
        Log::info() << "ActionGraph:\n" << result << std::endl;

        EXPECT_EQUAL(result, "   Copy[]\n");
    }


    SECTION("GribOutput::set()") {
        MIRJob mj1;
        mj1.set("accuracy", 12);

        MIRJob mj2;
        mj2.set("accuracy", 16);

        auto result = graph_dump(*in, *out, mj1, mj2);
        Log::info() << "ActionGraph:\n" << result << std::endl;

        const auto* reference =
            "   Set[accuracy=12,output=GribFileOutput[path=]]\n"
            "   Set[accuracy=16,output=GribFileOutput[path=]]\n";
        EXPECT_EQUAL(result, reference);
    }


    SECTION("GribOutput::save()") {
        MIRJob mj1;
        mj1.set("accuracy", 12).set("truncation", 20);

        MIRJob mj2;
        mj2.set("accuracy", 16).set("truncation", 20);

        auto result = graph_dump(*in, *out, mj1, mj2);
        Log::info() << "ActionGraph:\n" << result << std::endl;

        const auto* reference =
            "   ShTruncate[truncation=20]\n"
            "      Save[accuracy=12,output=GribFileOutput[path=]]\n"
            "      Save[accuracy=16,output=GribFileOutput[path=]]\n";
        EXPECT_EQUAL(result, reference);
    }
}


CASE("GribOutput: save action comparison (by packing parameters)") {
    std::unique_ptr<MIROutput> out(new output::GribMemoryOutput(nullptr, 0));

    struct Param : param::SimpleParametrisation {
        explicit Param(long edition = 0) {
            field_.set("edition", edition).set("accuracy", 0L).set("packing", "not_simple").set("gridded", true);
        }
        const MIRParametrisation& userParametrisation() const override { return *this; }
        const MIRParametrisation& fieldParametrisation() const override { return field_; }

    private:
        SimpleParametrisation field_;
    };


    SECTION("defaults") {
        Param defaults;

        std::unique_ptr<action::Action> a(new action::io::Save(defaults, *out));
        std::unique_ptr<action::Action> b(new action::io::Save(defaults, *out));
        EXPECT(a->sameAs(*b));
    }


    SECTION("packing changes") {
        Param param[4];
        param[1].set("packing", "simple");
        param[2].set("accuracy", 12L);
        param[3].set("edition", 2L);

        for (size_t i = 0; i < 4; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                std::unique_ptr<action::Action> a(new action::io::Save(param[i], *out));
                std::unique_ptr<action::Action> b(new action::io::Save(param[j], *out));
                if (i == j) {
                    EXPECT(a->sameAs(*b));
                }
                else {
                    EXPECT_NOT(a->sameAs(*b));
                }
            }
        }
    }


    SECTION("compatibility changes") {
        Param a;
        Param b;
        b.set("compatibility", "no-local-extension");

        const MIROutput& o = *out;
        EXPECT(o.sameParametrisation(a, a));
        EXPECT_NOT(o.sameParametrisation(a, b));
    }


    SECTION("control grib-edition-conversion") {
        std::unique_ptr<grib::Packing> ptr;

        Param param(1L);
        param.set("packing", "ccsds");  // edition=2 only

        param.set("grib-edition-conversion", false);
        EXPECT_THROWS_AS(ptr.reset(grib::Packing::build(param)), exception::UserError);

        param.set("grib-edition-conversion", true);
        EXPECT_NO_THROW(ptr.reset(grib::Packing::build(param)));
    }
}


CASE("GribOutput: print and json parametrisation") {
    input_t in(new input::GribFileInput("regular_ll.2-2.grib2"));
    ASSERT(in->next());

    output::GribMemoryOutput out(nullptr, 0);
    const MIROutput& base = out;

    param::SimpleParametrisation user;
    const param::CombinedParametrisation param(user, in->parametrisation());

    std::ostringstream none;
    EXPECT_NOT(base.printParametrisation(none, param));

    user.set("accuracy", 12L).set("packing", "ccsds");

    std::ostringstream str;
    EXPECT(base.printParametrisation(str, param));
    EXPECT(str.str().find("accuracy=12") != std::string::npos);
    EXPECT(str.str().find("packing=ccsds") != std::string::npos);

    std::ostringstream json;
    eckit::JSON j(json);
    j.startObject();
    base.jsonParametrisation(j, param);
    j.endObject();
    EXPECT(json.str().find(R"("accuracy":12)") != std::string::npos);
    EXPECT(json.str().find(R"("packing":"ccsds")") != std::string::npos);
}


CASE("GribFileOutput") {
    eckit::TmpDir dir;
    auto path = (dir / "out.grib").asString();

    const std::string inPath = "regular_ll.2-2.grib2";
    input_t in(new input::GribFileInput(inPath));
    ASSERT(in->next());


    SECTION("copy") {
        {
            output::GribFileOutput out(path);
            MIRJob().execute(*in, out);
        }
        EXPECT(read_file(path) == read_file(inPath));

        // appending
        {
            output::GribFileOutput out(path, true);
            MIRJob().execute(*in, out);
        }
        EXPECT(read_file(path) == read_file(inPath) + read_file(inPath));

        // overwriting
        {
            output::GribFileOutput out(path);
            MIRJob().execute(*in, out);
        }
        EXPECT(read_file(path) == read_file(inPath));
    }


    SECTION("comparisons") {
        output::GribFileOutput out(path);
        const MIROutput& base = out;

        EXPECT(base.sameAs(output::GribFileOutput(path)));
        EXPECT_NOT(base.sameAs(output::GribFileOutput(path, true)));
        EXPECT_NOT(base.sameAs(output::GribFileOutput(path + "x")));
        EXPECT_EQUAL(to_string(base), "GribFileOutput[path=" + path + "]");
    }
}


CASE("GribMemoryOutput") {
    input_t in(new input::GribFileInput("regular_ll.2-2.grib2"));
    ASSERT(in->next());

    Memory memory;
    MIRJob().execute(*in, memory.output);

    EXPECT_EQUAL(memory.output.length(), read_file("regular_ll.2-2.grib2").size());
    EXPECT_EQUAL(memory.output.interpolated(), 0);
    EXPECT_EQUAL(memory.output.saved(), 1);

    const MIROutput& base = memory.output;
    EXPECT(base.sameAs(memory.output));
    EXPECT_NOT(base.sameAs(output::GribMemoryOutput(nullptr, 0)));
    EXPECT_EQUAL(to_string(base), "GribMemoryOutput[]");

    // too small
    std::vector<char> small(10);
    output::GribMemoryOutput out(small.data(), small.size());
    EXPECT_THROWS_AS(MIRJob().execute(*in, out), eckit::AssertionFailed);
}


CASE("GribOutput: set (encoding only)") {
    input_t in(new input::GribFileInput("regular_ll.2-2.grib2"));
    ASSERT(in->next());

    auto field         = in->field();
    const auto& values = field.values(0);

    struct test_t {
        std::string packing;
        std::string packingType;
    };

    for (const auto& test : std::vector<test_t>{{"ccsds", "grid_ccsds"},
                                                {"second-order", "grid_second_order"},
                                                {"ieee", "grid_ieee"},
                                                {"simple", "grid_simple"}}) {
        Memory memory;

        MIRJob job;
        job.set("packing", test.packing);
        job.execute(*in, memory.output);

        // (the input packing is simple, so it is copied)
        const size_t copied = test.packing == "simple" ? 1 : 0;
        EXPECT_EQUAL(memory.output.interpolated(), 1 - copied);
        EXPECT_EQUAL(memory.output.saved(), copied);

        auto msg = memory.message();
        EXPECT_EQUAL(grib_string(*msg, "packingType"), test.packingType);
        EXPECT_EQUAL(grib_long(*msg, "numberOfDataPoints"), 16380);

        auto encodedField   = msg->field();
        const auto& encoded = encodedField.values(0);
        ASSERT(encoded.size() == values.size());
        for (size_t i = 0; i < values.size(); ++i) {
            EXPECT(std::abs(encoded[i] - values[i]) < 1e-2 * std::max(1., std::abs(values[i])));
        }
    }
}


CASE("GribOutput: save (encoding a new field)") {
    input_t in(new input::GribFileInput("regular_ll.2-2.grib1"));
    ASSERT(in->next());


    SECTION("grid, edition, metadata") {
        Memory memory;

        MIRJob job;
        job.set("grid", 4., 4.).set("caching", false).set("edition", 2L).set("metadata", "paramId=167");
        job.execute(*in, memory.output);

        auto msg = memory.message();
        EXPECT_EQUAL(grib_long(*msg, "edition"), 2);
        EXPECT_EQUAL(grib_long(*msg, "Ni"), 90);
        EXPECT_EQUAL(grib_long(*msg, "Ni") * grib_long(*msg, "Nj"), grib_long(*msg, "numberOfDataPoints"));
        EXPECT_EQUAL(grib_long(*msg, "paramId"), 167);
        EXPECT_EQUAL(grib_string(*msg, "gridType"), "regular_ll");
    }


    SECTION("reduced_gg, ccsds") {
        Memory memory;

        MIRJob job;
        job.set("grid", "O16").set("caching", false).set("packing", "ccsds").set("edition", 2L);
        job.execute(*in, memory.output);

        auto msg = memory.message();
        EXPECT_EQUAL(grib_string(*msg, "gridType"), "reduced_gg");
        EXPECT_EQUAL(grib_string(*msg, "packingType"), "grid_ccsds");
        EXPECT_EQUAL(grib_long(*msg, "numberOfDataPoints"), 1600);
    }


    SECTION("HEALPix, nested") {
        Memory memory;

        MIRJob job;
        job.set("grid", "{grid: H2, order: nested}")
            .set("caching", false)
            .set("interpolation", "nn")
            .set("edition", 2L);
        job.execute(*in, memory.output);

        auto msg = memory.message();
        EXPECT_EQUAL(grib_string(*msg, "gridType"), "healpix");
        EXPECT_EQUAL(grib_string(*msg, "orderingConvention"), "nested");
        EXPECT_EQUAL(grib_long(*msg, "numberOfDataPoints"), 48);
    }


    SECTION("basic angle") {
        input_t in2(new input::GribFileInput("regular_ll.2-2.grib2"));
        ASSERT(in2->next());

        for (const auto& basicAngle : std::vector<std::string>{"decimal", "fraction", "as-input"}) {
            Memory memory;

            MIRJob job;
            job.set("grid", 4., 4.).set("caching", false).set("basic-angle", basicAngle);
            job.execute(*in2, memory.output);

            auto msg = memory.message();
            EXPECT_EQUAL(grib_long(*msg, "Ni"), 90);
        }

        Memory memory;
        MIRJob job;
        job.set("grid", 4., 4.).set("caching", false).set("basic-angle", "unknown");
        EXPECT_THROWS_AS(job.execute(*in2, memory.output), eckit::AssertionFailed);
    }


    SECTION("delete local definition") {
        size_t length[2];

        for (const auto& remove : {false, true}) {
            Memory memory;

            MIRJob job;
            job.set("grid", 4., 4.).set("caching", false).set("delete-local-definition", remove);
            job.execute(*in, memory.output);

            length[remove ? 1 : 0] = memory.output.length();
        }

        EXPECT(length[1] < length[0]);
    }


    SECTION("missing values") {
        for (const auto& missingValue : {0., 9999.}) {
            GridInput raw("{grid: [2, 2], area: [10, 0, 0, 10]}");
            raw.values[0] = missingValue;
            raw.metadata.set("missing_value", missingValue);

            Memory memory;

            MIRJob job;
            job.set("grid", 5., 5.).set("caching", false).set("interpolation", "nn");
            job.execute(*raw.input, memory.output);

            auto msg = memory.message();
            EXPECT_EQUAL(grib_long(*msg, "numberOfDataPoints"), 9);
            EXPECT_EQUAL(grib_long(*msg, "bitmapPresent"), 1);
            EXPECT_EQUAL(grib_long(*msg, "numberOfMissing"), 1);

            auto field = msg->field();
            EXPECT(field.hasMissing());
            EXPECT_EQUAL(field.values(0)[0], field.missingValue());
        }
    }
}


CASE("GribOutput: vector and multi-dimensional") {
    eckit::TmpDir dir;
    auto pathU = (dir / "u.grib").asString();
    auto pathV = (dir / "v.grib").asString();

    const std::string path = "date=20200308,level=1000,grid=O80,param=u_v";

    struct test_t {
        std::string name;
        std::string key;
        std::string value;
        long Ni;
    };

    const std::vector<test_t> tests{{"copy", "", "", 0}, {"set", "accuracy", "12", 0}, {"save", "grid", "", 36}};


    SECTION("vector") {
        for (const auto& test : tests) {
            Log::info() << "VectorOutput: " << test.name << std::endl;

            {
                input::GribFileInput u(path, 0, 2);
                input::GribFileInput v(path, 1, 2);
                input::VectorInput in(u, v);
                ASSERT(static_cast<MIRInput&>(in).next());

                output::GribFileOutput outU(pathU);
                output::GribFileOutput outV(pathV);
                output::VectorOutput out(outU, outV);

                MIRJob job;
                job.set("caching", false);
                if (test.key == "grid") {
                    job.set("grid", 10., 10.);
                }
                else if (!test.key.empty()) {
                    job.set(test.key, 12L);
                }

                job.execute(in, out);
            }

            auto messagesU = messages(pathU);
            auto messagesV = messages(pathV);
            ASSERT(messagesU.size() == 1 && messagesV.size() == 1);

            input::GribMemoryInput u(messagesU[0].data(), messagesU[0].size());
            input::GribMemoryInput v(messagesV[0].data(), messagesV[0].size());

            EXPECT_EQUAL(grib_long(u, "paramId"), 131);
            EXPECT_EQUAL(grib_long(v, "paramId"), 132);
            if (test.Ni > 0) {
                EXPECT_EQUAL(grib_long(u, "Ni"), test.Ni);
                EXPECT_EQUAL(grib_long(v, "Ni"), test.Ni);
            }
            else {
                EXPECT_EQUAL(grib_string(u, "gridType"), "reduced_gg");
            }
        }
    }


    SECTION("multi-dimensional") {
        for (const auto& test : tests) {
            Log::info() << "MultiDimensionalOutput: " << test.name << std::endl;

            {
                input::MultiDimensionalGribFileInput in(path, 2);
                ASSERT(static_cast<MIRInput&>(in).next());

                output::MultiDimensionalOutput out;
                out.appendDimensionalOutput(new output::GribFileOutput(pathU));
                out.appendDimensionalOutput(new output::GribFileOutput(pathV));

                MIRJob job;
                job.set("caching", false);
                if (test.key == "grid") {
                    job.set("grid", 10., 10.);
                }
                else if (!test.key.empty()) {
                    job.set(test.key, 12L);
                }

                job.execute(in, out);
            }

            auto messagesU = messages(pathU);
            auto messagesV = messages(pathV);
            ASSERT(messagesU.size() == 1 && messagesV.size() == 1);

            input::GribMemoryInput u(messagesU[0].data(), messagesU[0].size());
            input::GribMemoryInput v(messagesV[0].data(), messagesV[0].size());

            EXPECT_EQUAL(grib_long(u, "paramId"), 131);
            EXPECT_EQUAL(grib_long(v, "paramId"), 132);
            if (test.Ni > 0) {
                EXPECT_EQUAL(grib_long(v, "Ni"), test.Ni);
            }
        }
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
