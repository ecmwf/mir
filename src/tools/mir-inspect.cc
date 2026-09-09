// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/input/GribFileInput.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"


namespace mir::tools {


class MIRInspect : public tools::MIRTool {
    using MIRTool::MIRTool;

    int minimumPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool
                    << " [key1 key2 ...] file.grib"
                       "\n"
                       "Examples:"
                       "\n"
                       "% "
                    << tool
                    << " grid area file.grib"
                       "\n"
                       "% "
                    << tool
                    << " reduced file.grib"
                       "\n"
                       "% "
                    << tool
                    << " regular file.grib"
                       "\n"
                       "% "
                    << tool
                    << " truncation file.grib"
                       "\n"
                       "% "
                    << tool << " octahedral file.grib" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& /*unused*/) override;
};


void MIRInspect::execute(const eckit::option::CmdArgs& /*unused*/) {
    input::GribFileInput file(argv(argc() - 1));

    while (file.next()) {
        input::MIRInput& input = file;

        const param::MIRParametrisation& parametrisation = input.parametrisation();

        const char* sep = "";
        std::string value;

        for (int i = 1; i < argc() - 1; i++) {
            Log::info() << sep << argv(i) << "=";
            if (parametrisation.get(argv(i), value)) {
                Log::info() << value;
            }
            else {
                Log::info() << "<not found>";
            }
            sep = ", ";
        }

        Log::info() << std::endl;
    }
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRInspect tool(argc, argv);
    return tool.start();
}
