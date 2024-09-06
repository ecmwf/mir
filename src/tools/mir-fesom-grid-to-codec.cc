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
#include <cmath>
#include <fstream>
#include <vector>
// #include <type_traits>

#include "eckit/codec/codec.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


// static_assert(std::is_same_v<eckit::linalg::Scalar, double>, "Scalar == double");
// static_assert(std::is_same_v<eckit::linalg::Index, std::int32_t>, "Index == std::int32_t");


struct xyz_t {
    double xyz[3] = {0, 0, 0};

    double& x = xyz[0];
    double& y = xyz[1];
    double& z = xyz[2];

    xyz_t(double lon, double lat) {
        const auto r = 6371229.; // [m]

        auto lonr = lon * M_PI / 180.;
        auto latr = lat * M_PI / 180.;

        x = r * std::cos(latr) * std::cos(lonr);
        y = r * std::cos(latr) * std::sin(lonr);
        z = r * std::sin(latr);
    }
};


struct nod2d_t {
    size_t idx = 0;
    size_t tag = 0;
    double ll[2] = {0,0};

    double& lon = ll[0];
    double& lat = ll[1];

    bool read(std::istream& in) { return (in >> idx >> lon >> lat >> tag) && in; }
};


struct elem2d_t {
    size_t abc[3] = {0,0,0};

    size_t& a = abc[0];
    size_t& b = abc[1];
    size_t& c = abc[2];

    bool read(std::istream& in) { return (in >> a >> b >> c) && in; }
};


struct MIRFESOMGridToCodec : public MIRTool {
    MIRFESOMGridToCodec(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::SimpleOption;

        options_.push_back(new SimpleOption<std::string>("nod2d", "nod2d.out file"));
        options_.push_back(new SimpleOption<std::string>("elem2d", "elem2d.out file"));

        options_.push_back(new SimpleOption<std::string>("output-gmsh", "Gmsh output file (.msh)"));
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

        auto read_file = [](const eckit::PathName& path, auto& list) {
            ASSERT(path.exists());
            std::ifstream in(path);

            size_t n = 0;
            ASSERT(in >> n);

            list.resize(n);
            for (auto& entry : list) {
                ASSERT(entry.read(in));
            }
        };

        std::vector<nod2d_t> nod2d;
        read_file(args.getString("nod2d"), nod2d);

        std::vector<elem2d_t> elem2d;
        read_file(args.getString("elem2d"), elem2d);


        // Output Gmsh .msh format
        if (auto fn = args.getString("output-gmsh", ""); !fn.empty()){
            std::ofstream out(fn);
            ASSERT(out);

            out << "$MeshFormat"
                   "\n2.2 0 8"
                   "\n$EndMeshFormat";

            out << "\n$Nodes\n" << nod2d.size();
            for (size_t i = 0; i < nod2d.size(); ++i) {
                xyz_t p(nod2d[i].lon, nod2d[i].lat);
                out << "\n" << (i + 1) << ' ' << p.x << ' ' << p.y << ' ' << p.z;
            }
            out << "\n$EndNodes";

            out << "\n$Elements\n" << elem2d.size();
            for (size_t i = 0; i < elem2d.size(); ++i) {
                out << "\n" << (i + 1) << " 2 0 " << elem2d[i].a << ' ' << elem2d[i].b << ' ' << elem2d[i].c;
            }
            out << "\n$EndElements";
        }


#if 0
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
#endif
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    return mir::tools::MIRFESOMGridToCodec{argc, argv}.start();
}
