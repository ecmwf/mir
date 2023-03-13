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


#include <map>
#include <memory>
#include <sstream>

#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/grib/Packing.h"
#include "mir/input/MIRInput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


CASE("Packing") {
    // setup input/output
    std::map<std::string, param::SimpleParametrisation> fields;

#define COMMON "artificialInput:constant,constant:0.,accuracy:16,truncation:1279"
    fields["spectral_complex, edition=1"].set("input", "{" COMMON ",edition:1,spectral:true,packing:complex}");
    fields["spectral_complex, edition=2"].set("input", "{" COMMON ",edition:2,spectral:true,packing:complex}");
    fields["grid_simple, edition=1"].set("input", "{" COMMON ",edition:1,packing:simple,gridded:true}");
    fields["grid_simple, edition=2"].set("input", "{" COMMON ",edition:2,packing:simple,gridded:true}");
    fields["grid_ccsds, edition=2"].set("input", "{" COMMON ",edition:2,packing:ccsds,gridded:true}");
#undef COMMON

    static const param::DefaultParametrisation defaults;

    struct Combine {
        Combine(const api::MIRJob& job, const param::MIRParametrisation& field) :
            input(input::MIRInputFactory::build("constant", field)),
            param(new param::CombinedParametrisation(job, input->parametrisation(), defaults)) {}
        std::unique_ptr<input::MIRInput> input;
        std::unique_ptr<param::MIRParametrisation> param;
    };


    auto check_packing_edition_bits = [](const grib::Packing& pack, const std::string& packing = "", long edition = 0,
                                         long bits = 0, long precision = 0) {
        std::ostringstream str;
        pack.printParametrisation(str);
        const auto calculated = "Packing[" + str.str() + "]";

        auto app = [](const std::string& str, const std::string& key, long value) {
            return value == 0 ? "" : ((str.empty() ? "" : ",") + key + "=" + std::to_string(value));
        };

        std::string expected(packing.empty() ? "" : ("packing=" + packing));
        expected += app(expected, "edition", edition);
        expected += app(expected, "accuracy", bits);
        expected += app(expected, "precision", precision);
        expected = "Packing[" + expected + "]";

        Log::info() << expected << " (expected)\n" << calculated << " (calculated)" << std::endl;
        EXPECT_EQUAL(expected, calculated);
    };


    SECTION("grid=off, packing=off, edition=off, accuracy=off") {
        api::MIRJob job;

        for (const auto& field : fields) {
            std::unique_ptr<grib::Packing> pack(grib::Packing::build(*Combine(job, field.second).param));
            check_packing_edition_bits(*pack);
        }
    }


    SECTION("grid=1/1: modify only spectral->gridded packing, no edition, accuracy") {
        api::MIRJob job;
        job.set("grid", "1/1");

        for (const auto& field : fields) {
            std::unique_ptr<grib::Packing> pack(grib::Packing::build(*Combine(job, field.second).param));
            field.first == "spectral_complex, edition=1"   ? check_packing_edition_bits(*pack, "simple")
            : field.first == "spectral_complex, edition=2" ? check_packing_edition_bits(*pack, "simple")
                                                           : check_packing_edition_bits(*pack);
        }
    }


    SECTION("packing=ccsds") {
        api::MIRJob job;
        job.set("packing", "ccsds");
        EXPECT_THROWS_AS(grib::Packing::build(*Combine(job, fields["grid_simple, edition=1"]).param),
                         exception::UserError);

        job.set("grid", "1/1");
        EXPECT_THROWS_AS(grib::Packing::build(*Combine(job, fields["spectral_complex, edition=1"]).param),
                         exception::UserError);

        // (edition conversion)
        job.set("edition", 2);
        for (const auto& field : fields) {
            std::unique_ptr<grib::Packing> pack(grib::Packing::build(*Combine(job, field.second).param));
            bool same_packing = field.first == "grid_ccsds, edition=2";
            bool same_edition = field.first.back() == '2';
            check_packing_edition_bits(*pack, same_packing ? "" : "ccsds", same_edition ? 0 : 2);
        }
    }


    SECTION("packing=ieee") {
        api::MIRJob job;
        job.set("packing", "ieee");

        for (const auto& field : fields) {
            // std::unique_ptr<grib::Packing> pack(grib::Packing::build(*Combine(job, field.second).param));
            // check_packing_edition_bits(*pack, "ieee", 0, 32, 1);

            for (long accuracy : {31, 32, 33, 63, 64, 65}) {
                job.set("accuracy", accuracy);

                if (accuracy == 32) {
                    std::unique_ptr<grib::Packing> pack(grib::Packing::build(*Combine(job, field.second).param));
                    check_packing_edition_bits(*pack, "ieee", 0, 32, 1);
                    continue;
                }

                if (accuracy == 64) {
                    std::unique_ptr<grib::Packing> pack(grib::Packing::build(*Combine(job, field.second).param));
                    check_packing_edition_bits(*pack, "ieee", 0, 64, 2);
                    continue;
                }

                EXPECT_THROWS_AS(grib::Packing::build(*Combine(job, field.second).param), exception::UserError);
            }
        }
    }


    SECTION("grib-packing-always-set=true") {
        api::MIRJob job;
        job.set("grid", "1/1");
        job.set("grib-packing-always-set", true);

        for (const auto& field : fields) {
            Log::info() << "'" << field.first << "'" << std::endl;
            std::unique_ptr<grib::Packing> pack(grib::Packing::build(*Combine(job, field.second).param));

            bool same_packing = field.first == "grid_simple, edition=1" || field.first == "grid_simple, edition=2";
            check_packing_edition_bits(*pack, same_packing ? "" : "simple");
        }
    }


    SECTION("grib-packing-always-set=true, grib-packing-gridded=ccsds") {
        api::MIRJob job;
        job.set("grid", "1/1");
        job.set("grib-edition-conversion", true);
        job.set("grib-packing-always-set", true);
        job.set("grib-packing-gridded", "ccsds");

        for (const auto& field : fields) {
            Log::info() << "'" << field.first << "'" << std::endl;
            std::unique_ptr<grib::Packing> pack(grib::Packing::build(*Combine(job, field.second).param));
            bool same_packing = field.first == "grid_ccsds, edition=2";
            bool same_edition = field.first.back() == '2';
            check_packing_edition_bits(*pack, same_packing ? "" : "ccsds", same_edition ? 0 : 2);
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
