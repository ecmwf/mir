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
#include <cstdint>
#include <fstream>
#include <vector>

#include "eckit/codec/codec.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


const uint64_t FESOM_CODEC_VERSION = 0;

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

    static size_t static_size() { return N; }
};


template <int N, int T>
struct elem_t : std::array<size_t, N> {
    void read(std::istream& in) {
        std::for_each_n(elem_t::begin(), N, [&](auto& entry) { ASSERT(in >> entry && entry >= 1 && in); });
    }

    friend std::ostream& operator<<(std::ostream& out, const elem_t& e) {
        out << T << " 0";
        std::for_each_n(e.begin(), N, [&](auto& entry) { out << ' ' << entry; });
        return out;
    }

    static size_t static_size() { return N; }
};


template <typename T>
struct vector_t : std::vector<T> {
    auto flatten(bool minus_one = false) const {
        std::vector<typename vector_t::value_type::value_type> flat(
            vector_t::empty() ? 0 : (vector_t::size() * vector_t::front().size()));

        std::for_each(vector_t::begin(), vector_t::end(), [&flat, minus_one](const auto& entry) {
            std::for_each(entry.begin(), entry.end(),
                          [&flat, minus_one](const auto& value) { flat.push_back(minus_one ? value - 1 : value); });
        });

        return flat;
    }

    std::array<size_t, 2> shape() const { return {vector_t::size(), T::static_size()}; }
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
            std::for_each(nod.begin(), nod.end(), [&](const auto& n) { out << "\n" << n; });
            out << "\n$EndNodes";

            out << "\n$Elements\n" << elem.size();
            std::for_each(elem.begin(), elem.end(), [&](const auto& e) {
                static auto i = 1;
                out << "\n" << i++ << ' ' << e;
            });
            out << "\n$EndElements";
        };


        // Read FESOM data structures in 2D/3D (only nod2d is mandatory)

        vector_t<nod_t<2>> nod2d;
        read_file(args.getString("nod2d"), nod2d);
        ASSERT(!nod2d.empty());

        vector_t<elem_t<3, 2>> elem2d;
        if (auto path = args.getString("elem2d", ""); !path.empty()) {
            read_file(path, elem2d);
        }

        vector_t<nod_t<3>> nod3d;
        if (auto path = args.getString("nod3d", ""); !path.empty()) {
            read_file(path, nod3d);
        }

        vector_t<elem_t<4, 4>> elem3d;
        if (auto path = args.getString("elem3d", ""); !path.empty()) {
            read_file(path, elem3d);
        }


        // Write Gmsh .msh(s)

        if (auto path = args.getString("output-gmsh-2d", ""); !path.empty()) {
            if (!elem2d.empty()) {
                write_gmsh(path, nod2d, elem2d);
            }
        }

        if (auto path = args.getString("output-gmsh-3d", ""); !path.empty()) {
            if (!nod3d.empty() && !elem3d.empty()) {
                write_gmsh(path, nod3d, elem3d);
            }
        }


        // Write eckit::codec file

        eckit::codec::RecordWriter record;
        record.set("version", FESOM_CODEC_VERSION);

        if (elem2d.empty() && nod3d.empty() && elem3d.empty()) {
            ASSERT(nod2d.shape()[1] == 2);
            auto n = nod2d.shape()[0];

            std::vector<double> longitude(n);
            std::transform(nod2d.begin(), nod2d.end(), longitude.begin(), [](const auto& n) { return n.lon; });

            std::vector<double> latitude(n);
            std::transform(nod2d.begin(), nod2d.end(), latitude.begin(), [](const auto& n) { return n.lat; });

            record.set("n", eckit::codec::ref(n));
            record.set("longitude", eckit::codec::ref(longitude));
            record.set("latitude", eckit::codec::ref(latitude));
        }
        else {
            record.set("nod2d_shape", eckit::codec::ref(nod2d.shape()));
            record.set("elem2d_shape", eckit::codec::ref(elem2d.shape()));
            record.set("nod3d_shape", eckit::codec::ref(nod3d.shape()));
            record.set("elem3d_shape", eckit::codec::ref(elem3d.shape()));

            record.set("nod2d", eckit::codec::ref(nod2d.flatten()));
            record.set("elem2d", eckit::codec::ref(elem2d.flatten(true)));
            record.set("nod3d", eckit::codec::ref(nod3d.flatten()));
            record.set("elem3d", eckit::codec::ref(elem3d.flatten(true)));
        }

        eckit::PathName path(args(0));
        record.write(path);
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    return mir::tools::MIRFESOMGridToCodec{argc, argv}.start();
}
