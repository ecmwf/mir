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


#include "eckit/option/CmdArgs.h"

#include <cmath>

#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"

#include "mir/api/MIRJob.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/grib/Config.h"
#include "mir/grib/Packing.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/MIRInput.h"
#include "mir/output/ValuesOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/repres/proxy/HEALPix.h"
#include "mir/repres/unsupported/HEALPixNested.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


namespace util {
constexpr double DEGREE_TO_RADIAN = M_PI / 180.;
constexpr double RADIAN_TO_DEGREE = M_1_PI * 180.;
}  // namespace util


struct healpix_t {
    const size_t Nside;
    const repres::proxy::HEALPix::Ordering ordering;
    size_t size() const { return 12 * Nside * Nside; }

    static healpix_t make(const repres::Representation& rep) {
        try {
            const auto& h = dynamic_cast<const repres::proxy::HEALPix&>(rep);
            return {h.Nside(), h.ordering()};
        }
        catch (const std::bad_cast&) {
        }

        try {
            const auto& h = dynamic_cast<const repres::unsupported::HEALPixNested&>(rep);
            return {h.Nside(), h.ordering()};
        }
        catch (const std::bad_cast&) {
        }

        throw exception::UserError("Only HEALPix ring or nested representations are supported");
    }

    size_t ni(size_t j) const {
        ASSERT(j < nj());
        return j < Nside ? 4 * (j + 1) : j < 3 * Nside ? 4 * Nside : ni(nj() - 1 - j);
    }

    size_t nj() const { return 4 * Nside - 1; }

    std::vector<double> latitudes() const {
        const auto Nj = nj();
        std::vector<double> l(Nj);

        auto i = l.begin();
        auto j = l.rbegin();
        for (size_t ring = 1; ring < 2 * Nside; ++ring, ++i, ++j) {
            const auto f = ring < Nside
                               ? 1. - static_cast<double>(ring * ring) / (3 * static_cast<double>(Nside * Nside))
                               : 4. / 3. - 2 * static_cast<double>(ring) / (3 * static_cast<double>(Nside));

            *i = 90. - util::RADIAN_TO_DEGREE * std::acos(f);
            *j = -*i;
        }
        *i = 0.;

        ASSERT(l.size() == Nj);
        return l;
    }

    std::vector<double> longitudes(size_t j) const {
        const auto Ni    = ni(j);
        const auto step  = 360. / static_cast<double>(Ni);
        const auto start = j < Nside || 3 * Nside - 1 < j || static_cast<bool>((j + Nside) % 2) ? step / 2. : 0.;

        std::vector<double> l(Ni);
        std::generate_n(l.begin(), Ni,
                        [start, step, n = 0ULL]() mutable { return start + static_cast<double>(n++) * step; });

        return l;
    }
};


struct MIRHEALPixMesh : MIRTool {
    MIRHEALPixMesh(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<bool>("ll", "Latitude/longitude coordinates"));
    }

    int minimumPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override {

        const param::ConfigurationWrapper args_wrap(args);

        size_t j = 0;
        for (const auto& arg : args) {
            Log::info() << arg << std::endl;
            std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(arg, args_wrap));

            while (input->next()) {
                auto field = input->field();
                ASSERT(field.dimensions() == 1);

                repres::RepresentationHandle rep(field.representation());
                auto H = healpix_t::make(*rep);

                if (args.getBool("ll")) {
                    auto& out = Log::info();

                    // const auto Nj        = H.nj();
                    // const auto latitudes = H.latitudes();

                    // for (size_t j = 0; j < Nj; ++j) {
                    //     const auto longitudes = H.longitudes(j);
                    //     for (const auto& lon : longitudes) {
                    //         out << lon << " ";
                    //     }
                    //     out << std::endl;
                    // }

                    out << ""
                           "$MeshFormat\n"
                           "2.2 0 8\n"
                           "$EndMeshFormat\n"
                           "$Nodes\n"
                           "4\n"
                           "1 0 0 0\n"
                           "2 1 0 0\n"
                           "3 0.5 1 0\n"
                           "4 0.5 0.5 0\n"
                           "$EndNodes\n"
                           "$Elements\n"
                           "3\n"
                           "1 2 2 0 1 2 3\n"
                           "2 2 2 0 2 3 4\n"
                           "3 2 2 0 3 1 4\n"
                           "$EndElements\n";

                    out << "$ElementData"
                           "0\n"                        // Number of string, real and integer tags (each, integer number of elements)
                           "0\n"
                           "1\n";

                    const auto& values=field.values(0);
                    out << values.size() << "\n"; // Number of elements
                    size_t i = 1;
                    for (const auto& v : field.values(0)) {
                        out << i++ << " " << v << "\n";
                    }
                    out << "$EndElementData\n";

                    out.flush();
                }
            }
        }
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRHEALPixMesh tool(argc, argv);
    return tool.start();
}
