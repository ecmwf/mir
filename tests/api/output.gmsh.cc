// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>

#include "eckit/filesystem/TmpDir.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/output/GmshOutput.h"
#include "mir/param/SimpleParametrisation.h"

#include "output.h"


namespace mir::api::tests {


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


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
