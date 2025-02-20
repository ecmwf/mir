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
#include <type_traits>

#include "eckit/codec/RecordReader.h"
#include "eckit/codec/codec.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/linalg/SparseMatrix.h"
#include "eckit/linalg/allocator/NonOwningAllocator.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


static_assert(std::is_same_v<eckit::linalg::Scalar, double>, "Scalar == double");
static_assert(std::is_same_v<eckit::linalg::Index, std::int32_t>, "Index == std::int32_t");


using Index  = eckit::linalg::Index;
using Scalar = eckit::linalg::Scalar;
using Size   = eckit::linalg::Size;


struct MIRCodecToWeightMatrix : public MIRTool {
    MIRCodecToWeightMatrix(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::SimpleOption;

        options_.push_back(new SimpleOption<std::string>("nr", "Matrix number of rows"));
        options_.push_back(new SimpleOption<std::string>("nc", "Matrix number of columns"));
        options_.push_back(new SimpleOption<std::string>("nnz", "CSR matrix number of non-zeros (for checking)"));
        options_.push_back(new SimpleOption<std::string>("ia", "CSR matrix row indices key, compressed"));
        options_.push_back(new SimpleOption<std::string>("ja", "CSR matrix column indices key"));
        options_.push_back(new SimpleOption<std::string>("a", "CSR matrix values key"));
    }

    int numberOfPositionalArguments() const override { return 2; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                    << "Conversion of specificly formated eckit::codec file into eckit::linalg::SparseMatrix file\n"
                    << "\n"
                    << "Usage: " << tool << " <file.codec> <file.mat>" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override {
        ASSERT(args.count() == numberOfPositionalArguments());

        auto Nr_key = args.getString("nr", "nr");
        auto Nc_key = args.getString("nc", "nc");
        auto ia_key = args.getString("ia", "ia");
        auto ja_key = args.getString("ja", "ja");
        auto a_key  = args.getString("a", "a");

        // eckit::codec file
        eckit::PathName fcodec(args(0));
        if (!fcodec.exists()) {
            throw exception::UserError("File does not exist: '" + fcodec + "'", Here());
        }

        eckit::codec::RecordReader reader(fcodec);

        std::uint64_t Nr = 0;
        std::uint64_t Nc = 0;
        std::vector<std::int32_t> ia;
        std::vector<std::int32_t> ja;
        std::vector<double> a;

        std::uint64_t version = 0;
        reader.read("version", version).wait();

        if (version == 0) {
            reader.read(Nr_key, Nr);
            reader.read(Nc_key, Nc);
            reader.read(ia_key, ia);
            reader.read(ja_key, ja);
            reader.read(a_key, a);
            reader.wait();

            if (std::string nnz_key; args.get("nnz", nnz_key)) {
                std::uint64_t nnz = 0;
                reader.read(nnz_key, nnz).wait();

                ASSERT(a.size() == nnz);
            }

            ASSERT(0 < Nr);
            ASSERT(0 < Nc);
            ASSERT(ia.size() == (Nr + 1));
            ASSERT(ja.size() == a.size());
        }
        else {
            throw exception::SeriousBug("unsupported version: " + std::to_string(version), Here());
        }

        // ensure 0-based indexing
        if (const auto base = ia.front(); base != 0) {
            std::for_each(ia.begin(), ia.end(), [base](auto& i) { i -= base; });
        }

        // ensure safe casting
        auto size = [](const auto& v) {
            auto vs = static_cast<Size>(v);
            ASSERT(v == static_cast<decltype(v)>(vs));
            return vs;
        };

        // create matrix
        eckit::linalg::SparseMatrix M(new eckit::linalg::allocator::NonOwningAllocator(
            size(Nr), size(Nc), size(a.size()), const_cast<Index*>(ia.data()), const_cast<Index*>(ja.data()),
            const_cast<Scalar*>(a.data())));

        // eckit::linalg::SparseMatrix file
        eckit::PathName fmat(args(1));
        M.save(fmat);
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    return mir::tools::MIRCodecToWeightMatrix{argc, argv}.start();
}
