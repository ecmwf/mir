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
#include <vector>

#include "netcdf"

#include "eckit/filesystem/PathName.h"
#include "eckit/linalg/SparseMatrix.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/allocator/InPlaceAllocator.h"


namespace mir::tools {


using Index  = eckit::linalg::Index;
using Scalar = eckit::linalg::Scalar;
using Size   = eckit::linalg::Size;


static std::string DEFAULT_DIM_NR  = "dst_grid_size";
static std::string DEFAULT_DIM_NC  = "src_grid_size";
static std::string DEFAULT_DIM_NNZ = "num_links";
static std::string DEFAULT_VAR_IA  = "dst_address";
static std::string DEFAULT_VAR_JA  = "src_address";
static std::string DEFAULT_VAR_A   = "remap_matrix";
static int DEFAULT_BASE            = 1;


struct MIRScripToWeightMatrix : public MIRTool {
    MIRScripToWeightMatrix(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::SimpleOption;

        options_.push_back(
            new SimpleOption<std::string>("nr", "Dimension number of rows (default '" + DEFAULT_DIM_NR + "')"));
        options_.push_back(
            new SimpleOption<std::string>("nc", "Dimension number of columns (default '" + DEFAULT_DIM_NC + "')"));
        options_.push_back(new SimpleOption<size_t>(
            "nnz", "Dimension number of non-zeros (for checking) (default '" + DEFAULT_DIM_NNZ + "')"));
        options_.push_back(new SimpleOption<std::string>(
            "ia", "Variable Matrix destination/rows indices, not compressed (default '" + DEFAULT_VAR_IA + "')"));
        options_.push_back(new SimpleOption<std::string>(
            "ja", "Variable Matrix source/column indices (default '" + DEFAULT_VAR_JA + "')"));
        options_.push_back(new SimpleOption<std::string>(
            "a", "Variable Matrix interpolation weights (default '" + DEFAULT_VAR_A + "')"));

        options_.push_back(
            new SimpleOption<size_t>("base", "Indices base (default " + std::to_string(DEFAULT_BASE) + ")"));
    }

    int numberOfPositionalArguments() const override { return 2; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                    << "Conversion of specificly formated SCRIP interpolation weights file into "
                       "eckit::linalg::SparseMatrix file\n"
                    << "\n"
                    << "Usage: " << tool << " <file.nc> <file.mat>" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override {
        ASSERT(args.count() == numberOfPositionalArguments());

        Size Nr  = 0;
        Size Nc  = 0;
        Size nnz = 0;

        std::vector<Index> ia;
        std::vector<Index> ja;
        std::vector<Scalar> a;


        // read SCRIP file
        try {
            netCDF::NcFile f(args(0), netCDF::NcFile::read);

            Nr  = f.getDim(args.getString("nr", DEFAULT_DIM_NR)).getSize();
            Nc  = f.getDim(args.getString("nc", DEFAULT_DIM_NC)).getSize();
            nnz = f.getDim(args.getString("nnz", DEFAULT_DIM_NNZ)).getSize();

            ASSERT(Nr > 0);
            ASSERT(Nc > 0);
            ASSERT(nnz > 0);

            auto var_ia = f.getVar(args.getString("ia", DEFAULT_VAR_IA));
            ASSERT(var_ia.getDimCount() == 1 && var_ia.getDim(0).getSize() == nnz);
            ia.resize(nnz);  // NOTE: not compressed
            var_ia.getVar(ia.data());

            auto var_ja = f.getVar(args.getString("ja", DEFAULT_VAR_JA));
            ASSERT(var_ja.getDimCount() == 1 && var_ja.getDim(0).getSize() == nnz);
            ja.resize(nnz);
            var_ja.getVar(ja.data());

            auto var_a = f.getVar(args.getString("a", DEFAULT_VAR_A));
            ASSERT(var_a.getDimCount() == 2 && var_a.getDim(0).getSize() == nnz && var_a.getDim(1).getSize() == 1);
            a.resize(nnz);
            var_a.getVar(a.data());
        }
        catch (netCDF::exceptions::NcException& e) {
            throw eckit::SeriousBug("SCRIP reading failed", Here());
        }


        // matrix conversion to 0-based indexing, sorting and compression
        {
            std::vector<size_t> sorted(nnz);
            for (size_t n = 0; n < nnz; ++n) {
                sorted[n] = n;
            }

            std::sort(sorted.begin(), sorted.end(), [&](auto i, auto j) {
                return ia[i] != ia[j] ? ia[i] < ia[j] : ja[i] != ja[j] ? ja[i] < ja[j] : i < j;
            });

            decltype(ia) ia_unsorted(Nr + 1, 0);
            decltype(ja) ja_unsorted(nnz);
            ia.swap(ia_unsorted);
            ja.swap(ja_unsorted);

            const auto base = static_cast<Index>(args.getUnsigned("base", DEFAULT_BASE));
            for (size_t n = 0; n < nnz; ++n) {
                auto r = ia_unsorted[sorted[n]] - base;
                auto c = ja_unsorted[sorted[n]] - base;
                ASSERT(0 <= r && r < Nr);
                ASSERT(0 <= c && c < Nc);

                ia[r + 1]++;
                ja[n] = c;
            }

            ia_unsorted.clear();
            ja_unsorted.clear();

            for (size_t r = 0; r < Nr; ++r) {
                ia[r + 1] += ia[r];
            }

            ASSERT(ia.front() == 0 && ia.back() == nnz);

            decltype(a) a_unsorted(nnz);
            a.swap(a_unsorted);

            for (size_t n = 0; n < nnz; ++n) {
                a[n] = a_unsorted[sorted[n]];
                ASSERT(0. <= a[n] && a[n] <= 1.);
            }
        }

        // create matrix
        eckit::linalg::SparseMatrix M(
            new util::allocator::InPlaceAllocator(Nr, Nc, nnz, ia.data(), ja.data(), a.data()));

        // eckit::linalg::SparseMatrix file
        eckit::PathName fmat(args(1));
        M.save(fmat);
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    return mir::tools::MIRScripToWeightMatrix{argc, argv}.start();
}
