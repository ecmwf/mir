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
#include <string>
#include <vector>

#include <eccodes.h>

#include "eckit/codec/codec.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


const uint64_t ICON_CODEC_VERSION = 0;


struct ICONData {
    explicit ICONData(const std::string& path) {
        FILE* file = fopen(path.c_str(), "rb");
        ASSERT(file != nullptr);

        codes_handle* handle = nullptr;
        int err              = 0;

        // Loop through all messages in the file
        while ((handle = codes_handle_new_from_file(nullptr, file, PRODUCT_GRIB, &err)) != nullptr) {
            char shortName[20] = {
                0,
            };
            size_t shortNameLen = sizeof(shortName);

            ASSERT(codes_get_string(handle, "shortName", shortName, &shortNameLen) == CODES_SUCCESS);

            if (std::string s(shortName); s == "tlat") {
                ASSERT(lat.empty());

                size_t n = 0;
                ASSERT(codes_get_size(handle, "values", &n) == CODES_SUCCESS && n > 0);

                lat.resize(n);
                ASSERT(codes_get_double_array(handle, "values", lat.data(), &n) == CODES_SUCCESS);
            }
            else if (s == "tlon") {
                ASSERT(lon.empty());

                size_t n = 0;
                ASSERT(codes_get_size(handle, "values", &n) == CODES_SUCCESS && n > 0);

                lon.resize(n);
                ASSERT(codes_get_double_array(handle, "values", lon.data(), &n) == CODES_SUCCESS);
            }
        }

        ASSERT(lat.size() == lon.size());
        ASSERT(!lat.empty());
    }

    std::vector<double> lat;
    std::vector<double> lon;
};


struct Format {
    virtual void write(const eckit::PathName&, const ICONData&) const = 0;
    virtual ~Format()                                                 = default;

    static Format* build(const std::string&);
    static void list(std::ostream&);
};


struct FormatNone : Format {
    void write(const eckit::PathName&, const ICONData&) const override {}
};


struct FormatCodecLL : Format {
    void write(const eckit::PathName& path, const ICONData& icon) const override {
        eckit::codec::RecordWriter record;
        record.set("version", ICON_CODEC_VERSION);

        auto n = icon.lat.size();
        ASSERT(n == icon.lon.size());
        record.set("n", eckit::codec::ref(n));

        record.set("latitude", eckit::codec::ref(icon.lat));
        record.set("longitude", eckit::codec::ref(icon.lon));

        record.write(path);
    }
};


struct FormatGmsh2D : Format {
    void write(const eckit::PathName& path, const ICONData& icon) const override {
        ASSERT(path.extension() == ".msh");

        auto n = icon.lat.size();
        ASSERT(n == icon.lon.size());

        std::ofstream out(path);
        ASSERT(out);

        out << "$MeshFormat"
               "\n2.2 0 8"
               "\n$EndMeshFormat";

        out << "\n$Nodes\n" << n;
        for (size_t i = 0; i < n; ++i) {
            out << "\n" << i << " " << icon.lon[i] << " " << icon.lat[i] << " 0";
        }
        out << "\n$EndNodes";
    }
};


struct FormatGmsh3D : Format {
    void write(const eckit::PathName& path, const ICONData& icon) const override {
        ASSERT(path.extension() == ".msh");

        auto n = icon.lat.size();
        ASSERT(n == icon.lon.size());

        std::ofstream out(path);
        ASSERT(out);

        out << "$MeshFormat"
               "\n2.2 0 8"
               "\n$EndMeshFormat";

        auto to_xyz = [](double lat, double lon) -> std::array<double, 3> {
            const auto r    = 6371229.;           // [m]
            const auto lonr = lon * M_PI / 180.;  // [rad]
            const auto latr = lat * M_PI / 180.;  // [rad]

            const auto x = r * std::cos(latr) * std::cos(lonr);
            const auto y = r * std::cos(latr) * std::sin(lonr);
            const auto z = r * std::sin(latr);

            return {x, y, z};
        };

        out << "\n$Nodes\n" << n;
        for (size_t i = 0; i < n; ++i) {
            const auto p = to_xyz(icon.lat[i], icon.lon[i]);
            out << "\n" << i << " " << p[0] << " " << p[1] << " " << p[2];
        }
        out << "\n$EndNodes";
    }
};


Format* Format::build(const std::string& name) {
    return name == "none"                       ? static_cast<Format*>(new FormatNone)
           : name == "codec-latitude-longitude" ? static_cast<Format*>(new FormatCodecLL)
           : name == "gmsh-2d"                  ? static_cast<Format*>(new FormatGmsh2D)
           : name == "gmsh-3d"                  ? static_cast<Format*>(new FormatGmsh3D)
                                                : throw exception::SeriousBug("Format: unknown '" + name + "'");
}


void Format::list(std::ostream& out) {
    out << "none, "
           "codec-all, "
           "codec-latitude-longitude,"
           "gmsh-2d, "
           "gmsh-3d";
}


struct MIRICONGridToCodec : public MIRTool {
    MIRICONGridToCodec(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::FactoryOption<Format>("format", "output format"));
    }

    int numberOfPositionalArguments() const override { return 2; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                    << "Conversion of ICON grid GRIB2 files into eckit::codec file\n"
                    << "\n"
                    << "Usage: " << tool << " <file.g2> <file.codec>" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override {
        ASSERT(args.count() == numberOfPositionalArguments());

        // Read ICONData data structures in 2D/3D
        ICONData icon(args(0));

        // Write eckit::codec/Gmsh file
        std::unique_ptr<Format> {
            Format::build(args.getString("format", "codec-latitude-longitude"))
        } -> write(args(1), icon);
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    return mir::tools::MIRICONGridToCodec{argc, argv}.start();
}
