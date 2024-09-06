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
#include <array>
#include <cmath>
#include <fstream>
#include <vector>

#include "eckit/codec/codec.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


template <int N>
struct nod_t : std::array<double, 3> {
    size_t idx = 0;
    size_t tag = 0;

    value_type& lon    = (*this)[0];
    value_type& lat    = (*this)[1];
    value_type& height = (*this)[2];

    nod_t() : array{} { static_assert(N == 2 || N == 3); }

    void read(std::istream& in) {
        in >> idx;
        std::for_each_n(begin(), N, [&](auto& entry) { ASSERT(in >> entry && in); });
        in >> tag;
    }

    friend std::ostream& operator<<(std::ostream& out, const nod_t& n) {
        const auto r    = 6371229. + n.height;  // [m]
        const auto lonr = n.lon * M_PI / 180.;  // [rad]
        const auto latr = n.lat * M_PI / 180.;  // [rad]

        const auto x = r * std::cos(latr) * std::cos(lonr);
        const auto y = r * std::cos(latr) * std::sin(lonr);
        const auto z = r * std::sin(latr);

        return out << n.idx << ' ' << x << ' ' << y << ' ' << z;
    }
};


template <int N, int T>
struct elem_t : std::array<size_t, N> {

    void read(std::istream& in) {
        std::for_each_n(elem_t::begin(), N, [&](auto& entry) { ASSERT(in >> entry && entry > 0 && in); });
    }

    friend std::ostream& operator<<(std::ostream& out, const elem_t& e) {
        out << T << " 0";
        std::for_each_n(e.begin(), N, [&](auto& entry) { out << ' ' << entry; });
        return out;
    }
};


struct MIRFESOMGridToCodec : public MIRTool {
    MIRFESOMGridToCodec(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::SimpleOption;

        options_.push_back(new SimpleOption<std::string>("nod2d", "nod2d.out file"));
        options_.push_back(new SimpleOption<std::string>("elem2d", "elem2d.out file"));

        options_.push_back(new SimpleOption<std::string>("nod3d", "nod3d.out file"));
        options_.push_back(new SimpleOption<std::string>("elem3d", "elem3d.out file"));

        options_.push_back(new SimpleOption<std::string>("aux3d", "aux3d.out file"));
        options_.push_back(new SimpleOption<std::string>("depth", "depth.out file"));

        options_.push_back(new SimpleOption<std::string>("output-gmsh-2d", "Gmsh output file, 2D structures (.msh)"));
        options_.push_back(new SimpleOption<std::string>("output-gmsh-3d", "Gmsh output file, 3D structures (.msh)"));
    }

    int numberOfPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                    << "Conversion of FESOM grid .out files into eckit::codec file\n"
                    << "\n"
                    << "Usage: " << tool << " --nod2d=nod2d.out --elem2d=elem2d.out <file.codec>" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override {
        ASSERT(args.count() == numberOfPositionalArguments());

        auto read_file = [](const std::string& path, auto& list) {
            std::ifstream in(path);
            ASSERT(in);

            size_t n = 0;
            ASSERT(in >> n);

            list.resize(n);
            std::for_each(list.begin(), list.end(), [&](auto& entry) { entry.read(in); });
        };

        auto write_gmsh = [](const std::string& path, const auto& nod, const auto& elem) {
            std::ofstream out(path);
            ASSERT(out);

            out << "$MeshFormat"
                   "\n2.2 0 8"
                   "\n$EndMeshFormat";

            out << "\n$Nodes\n" << nod.size();
            std::for_each(nod.begin(), nod.end(), [&](const auto& n) {
                out << "\n" << n;
            });
            out << "\n$EndNodes";

            out << "\n$Elements\n" << elem.size();
            std::for_each(elem.begin(), elem.end(), [&](const auto& e) {
                static auto i = 1;
                out << "\n" << i++ << ' ' << e;
            });
            out << "\n$EndElements";
        };


        // Read FESOM data structures in 2D (mandatory) and 3D (optional)

        std::vector<nod_t<2>> nod2d;
        read_file(args.getString("nod2d"), nod2d);
        ASSERT(!nod2d.empty());

        std::vector<elem_t<3, 2>> elem2d;
        read_file(args.getString("elem2d"), elem2d);
        ASSERT(!elem2d.empty());

        std::vector<nod_t<3>> nod3d;
        if (auto path = args.getString("nod3d", ""); !path.empty()) {
            read_file(path, nod3d);
        }

        std::vector<elem_t<4, 4>> elem3d;
        if (auto path = args.getString("elem3d", ""); !path.empty()) {
            read_file(path, elem3d);
        }


        // Write Gmsh .msh(s)

        if (auto path = args.getString("output-gmsh-2d", ""); !path.empty()) {
            write_gmsh(path, nod2d, elem2d);
        }

        if (auto path = args.getString("output-gmsh-3d", ""); !path.empty()) {
            write_gmsh(path, nod3d, elem3d);
        }


#if 0
        eckit::PathName fcodec(args(0));
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
#endif
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    return mir::tools::MIRFESOMGridToCodec{argc, argv}.start();
}
