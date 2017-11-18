/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/action/context/Context.h"
#include "mir/action/misc/AreaCropper.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/MIRStatistics.h"


class MIRCount : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int minimumPositionalArguments() const {
        return 1;
    }
public:
    MIRCount(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using eckit::option::SimpleOption;
        using eckit::option::VectorOption;
        options_.push_back(new VectorOption<double>("area", "Specify the cropping area: north/west/south/east", 4));
        options_.push_back(new SimpleOption<bool>("verbose", "Use verbose output, default false"));
    }
};


void MIRCount::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nCount MIR representation number of values, compared to the GRIB numberOfValues."
               "\n"
               "\nUsage: " << tool << " [--area=N/W/S/E] [--verbose] file.grib [file.grib [...]]"
               "\nExamples:"
               "\n  % " << tool << " 1.grib"
               "\n  % " << tool << " --area=6/0/0/6 1.grib 2.grib"
            << std::endl;
}


void MIRCount::execute(const eckit::option::CmdArgs& args) {
    using mir::action::AreaCropper;


    // setup area crop, disabling crop cache
    const mir::param::ConfigurationWrapper args_wrap(args);
    mir::param::RuntimeParametrisation param(args_wrap);
    param.set("caching", false);

    eckit::ScopedPtr< AreaCropper > area_cropper;
    if (args.has("area")) {
        area_cropper.reset(new AreaCropper(param));
        ASSERT(area_cropper);
    }


    // verbosity
    bool verbose = false;
    args.get("verbose", verbose);


    // dummy statistics
    mir::util::MIRStatistics dummy;


    // count each file(s) message(s)
    for (size_t i = 0; i < args.count(); ++i) {
        if (verbose) {
            eckit::Log::info() << args(i) << std::endl;
        }

        mir::input::GribFileInput grib(args(i));
        const mir::input::MIRInput& input = grib;

        size_t count = 0;
        while (grib.next()) {
            ++count;

            mir::data::MIRField field = input.field();
            ASSERT(field.dimensions() == 1);

            mir::context::Context ctx(field, dummy);
            const size_t n = ctx.field().values(0).size();

            if (area_cropper) {

                area_cropper->execute(ctx);
                const size_t o = ctx.field().values(0).size();

                verbose ? eckit::Log::info() << n << " > " << o << std::endl
                        : eckit::Log::info() << o << std::endl;

            } else {
                eckit::Log::info() << n << std::endl;
            }
        }

        if (verbose) {
            eckit::Log::info() << eckit::Plural(count, "message") << " in " << args(i) << std::endl;
        }
    }

}


int main(int argc, char **argv) {
    MIRCount tool(argc, argv);
    return tool.start();
}

