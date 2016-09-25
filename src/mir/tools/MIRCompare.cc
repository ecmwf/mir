/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#include "mir/tools/MIRCompare.h"

#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "mir/compare/Comparator.h"


namespace mir {
namespace tools {


MIRCompare::MIRCompare(int argc, char** argv) : MIRTool(argc, argv) {
    using namespace eckit::option;

    options_.push_back(new SimpleOption<bool>("multi-files",             "Arguments are text files containing the names of GRIB/BUFR files"));
    options_.push_back(new SimpleOption<bool>("save-fields",             "Save fields that do not compare"));

    options_.push_back(new SimpleOption<bool>("normalise-longitudes",    "(Not yet used) Compare normalised values of east/west longitude (e.g. -1 == 359)"));
    options_.push_back(new SimpleOption<long>("round-degrees",           "(Not yet used) Number of decimal digits to round degrees to (away from zero)"));

    options_.push_back(new SimpleOption<bool>("file-names-only",         "Only check that the list of files created are the same"));
    options_.push_back(new SimpleOption<bool>("list-file-names",         "Create two files with extension '.list' containing the files names"));
    options_.push_back(new SimpleOption<bool>("ignore-exceptions",       "Ignore exceptions"));
    options_.push_back(new SimpleOption<bool>("ignore-count-mismatches", "Ignore field count mismatches"));
    options_.push_back(new SimpleOption<bool>("ignore-fields-not-found", "Ignore fields not found"));
    options_.push_back(new SimpleOption<bool>("ignore-missing-files",    "Ignore missing files"));
    options_.push_back(new SimpleOption<bool>("ignore-duplicates",       "Ignore duplicate fields"));
    options_.push_back(new SimpleOption<bool>("compare-statistics",      "Compare field statistics"));

    options_.push_back(new SimpleOption<std::string>("ignore",           "Slash separated list of request keys to ignore when comparing fields"));
    options_.push_back(new SimpleOption<std::string>("files-white-list",       "Slash separated list regex if file names to ignore"));
    options_.push_back(new SimpleOption<std::string>("parameters-white-list",       "Slash separated list of parameters to ignore"));

}


void MIRCompare::usage(const std::string& tool) const {
    eckit::Log::info()
            << "\n" << "Usage: " << tool << " [options] file1 file2"
            << std::endl;
}

void mir::tools::MIRCompare::execute(const eckit::option::CmdArgs& args) {

    // Straightforward two-file comparison
    compare::Comparator(args).compare(args(0), args(1));

}


}  // namespace tools
}  // namespace mir
