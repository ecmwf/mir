// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/TmpDir.h"
#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/ArtificialInput.h"
#include "mir/input/GeoPointsFileInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/MultiDimensionalInput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/ValueMap.h"

#include "input.h"


namespace mir::api::tests {


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


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
