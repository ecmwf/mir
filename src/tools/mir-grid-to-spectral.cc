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


#include <algorithm>
#include <memory>
#include <vector>

#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "ectrans/transi.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Types.h"


namespace mir::tools {


void dirtrans_rgg(size_t T, const std::vector<long>& pl, data::MIRField& field) {
    ASSERT(T > 0);
    ASSERT(!pl.empty());
    ASSERT(field.dimensions() == 1);

    Trans_t trans{};
    trans_new(&trans);

    trans.myproc = 1;
    trans.nproc  = 1;
    trans.nsmax  = static_cast<int>(T);
    trans.ndgl   = static_cast<int>(pl.size());

    trans.nloen = new int[pl.size()];
    std::transform(pl.begin(), pl.end(), trans.nloen, [](long p) { return static_cast<int>(p); });

    trans_use_mpi(0);
    trans_setup(&trans);
    trans_inquire(&trans,
                  "numpp,ngptotl,nmyms,nasm0,npossp,nptrms,nallms,ndim0g,nvalue,nfrstlat,nlstlat,nptrlat,nptrfrstlat,"
                  "nptrlstlat,nsta,nonl,ldsplitlat,nultpp,nptrls,nnmeng,rmu,rgw,npms,rlapin,ndglu");

    ASSERT(trans.nspec2 == static_cast<int>((T + 1) * (T + 2)));
    MIRValuesVector rspscalar(trans.nspec2);

    DirTrans_t dirtrans{
        field.values(0).data(), rspscalar.data(), nullptr, nullptr, trans.ngptot, 1, 0, 1, 0, &trans, 0};

    trans_dirtrans(&dirtrans);

    field.update(rspscalar, 0);
    field.representation(new repres::sh::SphericalHarmonics(T));

    trans_delete(&trans);
    trans_finalize();
}


struct MIRGridToSpectral : MIRTool {
    MIRGridToSpectral(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<size_t>("truncation", "Spectral truncation wave number"));
        options_.push_back(new eckit::option::SimpleOption<std::string>("out", "Output GRIB file"));
    }

    int minimumPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << tool << " --truncation=1279 <GRIB>" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override {
        static const param::DefaultParametrisation defaults;
        const param::ConfigurationWrapper args_wrap(args);

        int truncation = 0;
        args_wrap.get("truncation", truncation);

        std::string out;
        std::unique_ptr<output::MIROutput> output(args_wrap.get("out", out) ? new output::GribFileOutput(out)
                                                                            : nullptr);

        for (const auto& arg : args) {
            for (std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(arg, args_wrap));
                 input->next();) {
                ASSERT(input->dimensions() == 1);

                std::unique_ptr<param::MIRParametrisation> param(
                    new param::CombinedParametrisation(args_wrap, input->parametrisation(), defaults));

                util::MIRStatistics statistics;
                context::Context ctx(*input, statistics);

                if (std::vector<long> pl; param->get("pl", pl)) {
                    std::string gridType;
                    ASSERT(param->get("gridType", gridType) && gridType == "reduced_gg");

                    dirtrans_rgg(truncation, pl, ctx.field());

                    if (output) {
                        output->save(*param, ctx);
                    }

                    continue;
                }

                ASSERT(false);
            }
        }
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRGridToSpectral tool(argc, argv);
    return tool.start();
}
