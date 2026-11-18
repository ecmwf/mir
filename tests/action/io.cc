// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/filesystem/TmpFile.h"

#include "mir/action/io/Copy.h"
#include "mir/action/io/Save.h"
#include "mir/action/io/Set.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/action/plan/Job.h"
#include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"

#include "action.h"


namespace mir::action::tests {


// a small GRIB message (F8, 512 points, CCSDS-packed)
const std::string GRIB = "gridType=regular_gg,gridName=F8,shortName=q.grib2";


/// Job (plan) executed on the GRIB message, saving to a file; the plan end action type is returned
std::string execute(const api::MIRJob& job, const eckit::PathName& path) {
    input::GribFileInput input(GRIB);
    ASSERT(input.next());

    output::GribFileOutput output(path);
    const action::Job plan(job, input, output, true);

    util::MIRStatistics statistics;
    plan.execute(statistics);

    const auto& p = plan.plan();
    ASSERT(!p.empty());
    return p.action(p.size() - 1).type();
}


/// Messages in a GRIB file, checking their number of values
size_t messages(const eckit::PathName& path, size_t numberOfValues) {
    input::GribFileInput file(path);
    input::MIRInput& in = file;

    size_t n = 0;
    for (; in.next(); ++n) {
        EXPECT_EQUAL(in.field().values(0).size(), numberOfValues);
    }

    return n;
}


CASE("io.copy: no post-processing") {
    eckit::TmpFile path;

    // nothing to do, the input is copied
    EXPECT_EQUAL(execute(api::MIRJob(), path), "copy");
    EXPECT_EQUAL(messages(path, 512), 1);

    // the same values
    input::GribFileInput inFile(GRIB);
    input::GribFileInput outFile(path);
    input::MIRInput& in  = inFile;
    input::MIRInput& out = outFile;

    EXPECT(in.next() && out.next());
    EXPECT(in.field().values(0) == out.field().values(0));
}


CASE("io.set: packing only") {
    eckit::TmpFile path;

    api::MIRJob job;
    job.set("packing", "simple");

    // the input (CCSDS-packed) is re-packed
    EXPECT_EQUAL(execute(job, path), "set");
    EXPECT_EQUAL(messages(path, 512), 1);

    input::GribFileInput outFile(path);
    input::MIRInput& out = outFile;
    EXPECT(out.next());

    std::string packingType;
    EXPECT(out.parametrisation().get("packingType", packingType));
    EXPECT_EQUAL(packingType, "grid_simple");
}


CASE("io.save: post-processing") {
    eckit::TmpFile path;

    api::MIRJob job;
    job.set("grid", std::vector<double>{10, 10});

    EXPECT_EQUAL(execute(job, path), "save");
    EXPECT_EQUAL(messages(path, 19 * 36), 1);
}


CASE("io.*, properties") {
    param::SimpleParametrisation user;
    user.set("accuracy", 9L);

    param::SimpleParametrisation field;
    field.set("gridded", true).set("packing", "simple");

    const param::CombinedParametrisation param(user, field);

    eckit::TmpFile path1;
    eckit::TmpFile path2;
    output::GribFileOutput file1(path1);
    output::GribFileOutput file2(path2);
    output::EmptyOutput empty;

    const io::Copy copy(param, file1);
    const io::Set set(param, file1);
    const io::Save save(param, file1);

    const std::vector<const action::Action*> actions{&copy, &set, &save};

    EXPECT_EQUAL(actions[0]->type(), "copy");
    EXPECT_EQUAL(actions[1]->type(), "set");
    EXPECT_EQUAL(actions[2]->type(), "save");

    for (const auto* a : actions) {
        EXPECT(a->isEndAction());
        EXPECT(!a->isCropAction());
        EXPECT(!a->isRegridAction());
    }

    // printed with the output parametrisation, and the output (but not for plan dumps)
    EXPECT_EQUAL(str(copy), "Copy[]");
    EXPECT_EQUAL(custom_str(copy), "Copy[]");

    EXPECT(str(set).rfind("Set[", 0) == 0);
    EXPECT(str(set).find("output=") != std::string::npos);
    EXPECT(custom_str(set).rfind("Set[", 0) == 0);
    EXPECT(custom_str(set).find("output=...]") != std::string::npos);
    EXPECT(str(set) != custom_str(set));

    EXPECT_EQUAL(str(io::Save(param, empty)), "Save[accuracy=9,output=EmptyOutput[]]");
    EXPECT_EQUAL(custom_str(io::Save(param, empty)), "Save[accuracy=9,output=...]");
    EXPECT_EQUAL(io::Save(param, empty).json_str(), R"({"type":"save","accuracy":9})");

    // the same output (file) and parametrisation
    EXPECT(save == io::Save(param, file1));
    EXPECT(save != io::Save(param, file2));
    EXPECT(save != *make("noop"));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
