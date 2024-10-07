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
#include <memory>
#include <vector>

#include "eckit/codec/codec.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


const uint64_t FESOM_CODEC_VERSION = 0;


template <int N>
struct nod_t : std::array<double, 3> {
    static_assert(N == 2 || N == 3);

    nod_t() : array{0., 0., 0.} {}

    size_t idx = 0;
    size_t tag = 0;

    value_type& lon    = (*this)[0];
    value_type& lat    = (*this)[1];
    value_type& height = (*this)[2];

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


constexpr auto READ_FILE = [](const std::string& path, auto& list) {
    std::ifstream in(path);
    ASSERT(in);

    size_t n = 0;
    ASSERT(in >> n);
    ASSERT(0 < n);

    list.resize(n);
    std::for_each(list.begin(), list.end(), [&](auto& entry) { entry.read(in); });
};


constexpr auto WRITE_GMSH = [](const eckit::PathName& path, const auto& nod, const auto& elem) {
    ASSERT(path.extension() == ".msh");

    std::ofstream out(path);
    ASSERT(out);

    out << "$MeshFormat"
           "\n2.2 0 8"
           "\n$EndMeshFormat";

    out << "\n$Nodes\n" << nod.size();
    std::for_each(nod.begin(), nod.end(), [&](const auto& n) { out << "\n" << n; });
    out << "\n$EndNodes";

    if (!elem.empty()) {
        out << "\n$Elements\n" << elem.size();
        std::for_each(elem.begin(), elem.end(), [&](const auto& e) {
            static auto i = 1;
            out << "\n" << i++ << ' ' << e;
        });
        out << "\n$EndElements";
    }
};


struct FESOMData {
    explicit FESOMData(const eckit::option::CmdArgs& args) {
        READ_FILE(args.getString("nod2d"), nod2d);  // mandatory

        if (auto path = args.getString("elem2d", ""); !path.empty()) {
            READ_FILE(path, elem2d);
        }

        if (auto path = args.getString("nod3d", ""); !path.empty()) {
            READ_FILE(path, nod3d);
        }

        if (auto path = args.getString("elem3d", ""); !path.empty()) {
            READ_FILE(path, elem3d);
        }
    }

    vector_t<nod_t<2>> nod2d;
    vector_t<elem_t<3, 2>> elem2d;
    vector_t<nod_t<3>> nod3d;
    vector_t<elem_t<4, 4>> elem3d;
};


struct Format {
    Format()          = default;
    virtual ~Format() = default;

    Format(const Format&)            = delete;
    Format(Format&&)                 = delete;
    Format& operator=(const Format&) = delete;
    Format& operator=(Format&&)      = delete;

    virtual void write(const eckit::PathName&, const FESOMData&) const = 0;

    static Format* build(const std::string&);
    static void list(std::ostream&);
};


struct FormatNone : Format {
    void write(const eckit::PathName&, const FESOMData&) const override {}
};


struct FormatCodecAll : Format {
    void write(const eckit::PathName& path, const FESOMData& fesom) const override {
        eckit::codec::RecordWriter record;
        record.set("version", FESOM_CODEC_VERSION);

        record.set("nod2d_shape", eckit::codec::ref(fesom.nod2d.shape()));
        record.set("elem2d_shape", eckit::codec::ref(fesom.elem2d.shape()));
        record.set("nod3d_shape", eckit::codec::ref(fesom.nod3d.shape()));
        record.set("elem3d_shape", eckit::codec::ref(fesom.elem3d.shape()));

        record.set("nod2d", eckit::codec::ref(fesom.nod2d.flatten()));
        record.set("elem2d", eckit::codec::ref(fesom.elem2d.flatten(true)));
        record.set("nod3d", eckit::codec::ref(fesom.nod3d.flatten()));
        record.set("elem3d", eckit::codec::ref(fesom.elem3d.flatten(true)));

        record.write(path);
    }
};


struct FormatCodecC : Format {
    void write(const eckit::PathName& path, const FESOMData& fesom) const override {
        eckit::codec::RecordWriter record;
        record.set("version", FESOM_CODEC_VERSION);

        ASSERT(!fesom.nod2d.empty());
        ASSERT(!fesom.elem2d.empty());

        auto n = fesom.nod2d.shape()[0];
        auto e = fesom.elem2d.shape()[0];
        record.set("n", eckit::codec::ref(e));  // vectors size (# elements)

        std::vector<double> lat;
        std::vector<double> lon;
        lat.reserve(e);
        lon.reserve(e);

        for (const auto& e : fesom.elem2d) {
            ASSERT(e.size() == 3);
            ASSERT(0 < e[0] && e[0] <= n);  // numbering is 1-based
            ASSERT(0 < e[1] && e[1] <= n);
            ASSERT(0 < e[2] && e[2] <= n);

            lat.push_back((fesom.nod2d[e[0] - 1].lat + fesom.nod2d[e[1] - 1].lat + fesom.nod2d[e[2] - 1].lat) / 3.);
            lon.push_back((fesom.nod2d[e[0] - 1].lon + fesom.nod2d[e[1] - 1].lon + fesom.nod2d[e[2] - 1].lon) / 3.);
        }

        record.set("latitude", eckit::codec::ref(lat));
        record.set("longitude", eckit::codec::ref(lon));

        record.write(path);
    }
};


struct FormatCodecN : Format {
    void write(const eckit::PathName& path, const FESOMData& fesom) const override {
        eckit::codec::RecordWriter record;
        record.set("version", FESOM_CODEC_VERSION);

        auto n = fesom.nod2d.shape()[0];
        record.set("n", eckit::codec::ref(n));

        std::vector<double> latitude(n);
        std::transform(fesom.nod2d.begin(), fesom.nod2d.end(), latitude.begin(), [](const auto& n) { return n.lat; });
        record.set("latitude", eckit::codec::ref(latitude));

        std::vector<double> longitude(n);
        std::transform(fesom.nod2d.begin(), fesom.nod2d.end(), longitude.begin(), [](const auto& n) { return n.lon; });
        record.set("longitude", eckit::codec::ref(longitude));

        record.write(path);
    }
};


struct FormatGmsh2D : Format {
    void write(const eckit::PathName& path, const FESOMData& fesom) const override {
        WRITE_GMSH(path, fesom.nod2d, fesom.elem2d);
    }
};


struct FormatGmsh3D : Format {
    void write(const eckit::PathName& path, const FESOMData& fesom) const override {
        WRITE_GMSH(path, fesom.nod3d, fesom.elem3d);
    }
};


Format* Format::build(const std::string& name) {
    return name == "none"        ? static_cast<Format*>(new FormatNone)
           : name == "codec-all" ? static_cast<Format*>(new FormatCodecAll)
           : name == "codec-c"   ? static_cast<Format*>(new FormatCodecC)
           : name == "codec-n"   ? static_cast<Format*>(new FormatCodecN)
           : name == "gmsh-2d"   ? static_cast<Format*>(new FormatGmsh2D)
           : name == "gmsh-3d"   ? static_cast<Format*>(new FormatGmsh3D)
                                 : throw exception::SeriousBug("Format: unknown '" + name + "'");
}


void Format::list(std::ostream& out) {
    out << "none, "
           "codec-all, "
           "codec-c, "
           "codec-n, "
           "gmsh-2d, "
           "gmsh-3d";
}


struct MIRFESOMGridToCodec : public MIRTool {
    MIRFESOMGridToCodec(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::FactoryOption;
        using eckit::option::SimpleOption;

        options_.push_back(new SimpleOption<std::string>("nod2d", "nod2d.out file"));
        options_.push_back(new SimpleOption<std::string>("elem2d", "elem2d.out file"));

        options_.push_back(new SimpleOption<std::string>("nod3d", "nod3d.out file"));
        options_.push_back(new SimpleOption<std::string>("elem3d", "elem3d.out file"));

        options_.push_back(new FactoryOption<Format>("format", "output format"));
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

        // Read FESOMData data structures in 2D/3D
        FESOMData fesom(args);

        // Write eckit::codec/Gmsh file
        std::unique_ptr<Format> {
            Format::build(args.getString("format", "codec-latitude-longitude"))
        } -> write(args(0), fesom);
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    return mir::tools::MIRFESOMGridToCodec{argc, argv}.start();
}
