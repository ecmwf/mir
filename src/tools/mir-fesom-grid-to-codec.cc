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
#include "eckit/eckit_config.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/MD5.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


const uint64_t FESOM_CODEC_VERSION = 0;


struct PointXYZ : std::array<double, 3> {
    using container_type = array;
    using container_type::value_type;

    PointXYZ(value_type x, value_type y, value_type z) : container_type({x, y, z}) {}

    PointXYZ(const PointXYZ& other) : container_type(other) {}
    PointXYZ(PointXYZ&& other) : container_type(other) {}

    ~PointXYZ() = default;

    PointXYZ& operator=(const PointXYZ& other) {
        container_type::operator=(other);
        return *this;
    }

    PointXYZ& operator=(PointXYZ&& other) {
        container_type::operator=(other);
        return *this;
    }

    friend PointXYZ operator-(const PointXYZ& p, const PointXYZ& q) { return {p.x - q.x, p.y - q.y, p.z - q.z}; }

    friend PointXYZ operator*(const PointXYZ& p, const PointXYZ& q) {
        return {p.y * q.z - p.z * q.y, p.z * q.x - p.x * q.z, p.x * q.y - p.y * q.x};
    }

    static PointXYZ make_from_lonlat(double lon, double lat, double r = 1.) {
        const double lonr = lon * M_PI / 180.;
        const double latr = lat * M_PI / 180.;

        return {std::cos(latr) * std::cos(lonr), std::cos(latr) * std::sin(lonr), std::sin(latr)};
    }

    std::string to_string() const { return std::to_string(x) + ' ' + std::to_string(y) + ' ' + std::to_string(z); }

    const value_type& x = container_type::operator[](0);
    const value_type& y = container_type::operator[](1);
    const value_type& z = container_type::operator[](2);
};


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
        return out << n.idx << ' ' << PointXYZ::make_from_lonlat(n.lon, n.lat, 6371229. + n.height).to_string();
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
    auto flatten() const {
        std::vector<typename vector_t::value_type::value_type> flat(
            vector_t::empty() ? 0 : (vector_t::size() * vector_t::front().size()));

        std::for_each(vector_t::begin(), vector_t::end(), [&flat](const auto& entry) {
            std::for_each(entry.begin(), entry.end(), [&flat](const auto& value) { flat.push_back(value); });
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
            out << "\n" << i++;
            std::for_each(e.begin(), e.end(), [&](const auto& i) {
                out << ' ' << (i + 1);  // 0 to 1-based indexing
            });
        });
        out << "\n$EndElements";
    }
};


struct FESOMData {
    explicit FESOMData(const eckit::option::CmdArgs& args) {
        READ_FILE(args.getString("nod2d"), nod2d);  // mandatory

        bool base0 = args.getBool("base-0", true);
        bool flip  = args.getBool("flip", true);

        if (auto path = args.getString("elem2d", ""); !path.empty()) {
            READ_FILE(path, elem2d);

            for (auto& tri : elem2d) {
                ASSERT(tri.size() == 3);

                if (base0) {
                    // convert 1 to 0-based indexing
                    std::for_each(tri.begin(), tri.end(), [](auto& index) {
                        ASSERT(index > 0);
                        --index;
                    });
                }

                if (flip) {
                    // flip triangles with inward normals to be outwards facing
                    auto a = PointXYZ::make_from_lonlat(nod2d[tri[0]].lon, nod2d[tri[0]].lat);
                    auto b = PointXYZ::make_from_lonlat(nod2d[tri[1]].lon, nod2d[tri[1]].lat);
                    auto c = PointXYZ::make_from_lonlat(nod2d[tri[2]].lon, nod2d[tri[2]].lat);

                    auto cross = (c - b) * (a - b);
                    auto dot   = b.x * cross.x + b.y * cross.y + b.z * cross.z;
                    if (dot < 0.) {
                        std::swap(tri[1], tri[2]);
                    }
                }
            }
        }

        if (auto path = args.getString("nod3d", ""); !path.empty()) {
            READ_FILE(path, nod3d);
        }

        if (auto path = args.getString("elem3d", ""); !path.empty()) {
            READ_FILE(path, elem3d);

            for (auto& e : elem3d) {
                if (base0) {
                    // convert 1 to 0-based indexing
                    std::for_each(e.begin(), e.end(), [](auto& index) {
                        ASSERT(index > 0);
                        --index;
                    });
                }
            }
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
        record.set("elem2d", eckit::codec::ref(fesom.elem2d.flatten()));
        record.set("nod3d", eckit::codec::ref(fesom.nod3d.flatten()));
        record.set("elem3d", eckit::codec::ref(fesom.elem3d.flatten()));

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

        for (const auto& tri : fesom.elem2d) {
            ASSERT(0 <= tri[0] && tri[0] < n);  // numbering is 0-based
            ASSERT(0 <= tri[1] && tri[1] < n);
            ASSERT(0 <= tri[2] && tri[2] < n);

            std::array<double, 3> tlat{fesom.nod2d[tri[0]].lat, fesom.nod2d[tri[1]].lat, fesom.nod2d[tri[2]].lat};
            std::array<double, 3> tlon{fesom.nod2d[tri[0]].lon, fesom.nod2d[tri[1]].lon, fesom.nod2d[tri[2]].lon};
            auto lon_min = *std::min_element(tlon.begin(), tlon.end());

            for (auto& lon : tlon) {
                if (lon > lon_min + 180.) {
                    lon -= 360.;
                }
            }

            lat.push_back((tlat[0] + tlat[1] + tlat[2]) / 3.);
            lon.push_back((tlon[0] + tlon[1] + tlon[2]) / 3.);
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


struct CalculateUIDN : Format {
    void write(const eckit::PathName&, const FESOMData& fesom) const override {
        eckit::MD5 hash;
        calculate_uid_n(fesom, hash);
        Log::info() << hash.digest() << std::endl;
    }

    static void calculate_uid_n(const FESOMData& fesom, eckit::MD5& hash) {
        ASSERT(eckit_LITTLE_ENDIAN);

        std::vector<double> lon;
        std::vector<double> lat;
        lon.reserve(fesom.nod2d.size());
        lat.reserve(fesom.nod2d.size());

        for (const auto& n : fesom.nod2d) {
            lon.push_back(n.lon);
            lat.push_back(n.lat);
        }

        hash.add(lat.data(), static_cast<long>(lat.size() * sizeof(double)));
        hash.add(lon.data(), static_cast<long>(lon.size() * sizeof(double)));
    }
};


struct CalculateUIDC : Format {
    void write(const eckit::PathName&, const FESOMData& fesom) const override {
        ASSERT(eckit_LITTLE_ENDIAN);

        eckit::MD5 hash;

        std::make_unique<CalculateUIDN>()->calculate_uid_n(fesom, hash);

        ASSERT(!fesom.elem2d.empty());
        hash.add(fesom.elem2d.data(),
                 static_cast<long>(fesom.elem2d.size() * sizeof(decltype(FESOMData::elem2d)::value_type)));

        Log::info() << hash.digest() << std::endl;
    }
};


Format* Format::build(const std::string& name) {
    return name == "none"              ? static_cast<Format*>(new FormatNone)
           : name == "codec-all"       ? static_cast<Format*>(new FormatCodecAll)
           : name == "codec-c"         ? static_cast<Format*>(new FormatCodecC)
           : name == "codec-n"         ? static_cast<Format*>(new FormatCodecN)
           : name == "gmsh-2d"         ? static_cast<Format*>(new FormatGmsh2D)
           : name == "gmsh-3d"         ? static_cast<Format*>(new FormatGmsh3D)
           : name == "calculate-uid-c" ? static_cast<Format*>(new CalculateUIDC)
           : name == "calculate-uid-n" ? static_cast<Format*>(new CalculateUIDN)
                                       : throw exception::SeriousBug("Format: unknown '" + name + "'");
}


void Format::list(std::ostream& out) {
    out << "none, "
           "codec-all, "
           "codec-c, "
           "codec-n, "
           "gmsh-2d, "
           "gmsh-3d, "
           "calculate-uid-c, "
           "calculate-uid-n";
}


struct MIRFESOMGridToCodec : public MIRTool {
    MIRFESOMGridToCodec(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::FactoryOption;
        using eckit::option::SimpleOption;

        options_.push_back(new SimpleOption<std::string>("nod2d", "nod2d.out file"));
        options_.push_back(new SimpleOption<std::string>("elem2d", "elem2d.out file"));

        options_.push_back(new SimpleOption<std::string>("nod3d", "nod3d.out file"));
        options_.push_back(new SimpleOption<std::string>("elem3d", "elem3d.out file"));

        options_.push_back(new SimpleOption<bool>("base-0", "elements 1 to 0-based numbering"));
        options_.push_back(new SimpleOption<bool>("flip", "elements flipped to ensure outwards normals"));

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
