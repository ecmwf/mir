/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "eckit/log/Log.h"
#include "eckit/utils/Tokenizer.h"

#include "mir/input/GribFileInput.h"
#include "mir/tools/MIRTool.h"


class MIRInspect : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

    int minimumPositionalArguments() const { return 1; }

public:
    // -- Contructors

    MIRInspect(int argc, char** argv) : mir::tools::MIRTool(argc, argv) {}
};


void MIRInspect::usage(const std::string& tool) const {
    eckit::Log::info()
        << "\n" "Usage: " << tool << " [key1 key2 ...] file.grib"
           "\n" "Examples:"
           "\n" "% " << tool << " grid area file.grib"
           "\n" "% " << tool << " reduced file.grib"
           "\n" "% " << tool << " regular file.grib"
           "\n" "% " << tool << " truncation file.grib"
           "\n" "% " << tool << " octahedral file.grib"
        << std::endl;
}

void MIRInspect::execute(const eckit::option::CmdArgs&) {

    mir::input::GribFileInput file(argv(argc() - 1));
    while (file.next()) {
        mir::input::MIRInput& input = file;

        const mir::param::MIRParametrisation& parametrisation = input.parametrisation();

        const char* sep = "";
        std::string value;

        for (int i = 1; i < argc() - 1; i++) {
            eckit::Log::info() << sep << argv(i) << "=";
            if (parametrisation.get(argv(i), value)) {
                eckit::Log::info() << value;
            }
            else {
                eckit::Log::info() << "<not found>";
            }
            sep = ", ";
        }

        eckit::Log::info() << std::endl;
    }
}


int main(int argc, char** argv) {
    MIRInspect tool(argc, argv);
    return tool.start();
}
