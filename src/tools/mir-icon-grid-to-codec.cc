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


#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "eckit/codec/codec.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"

#include "mir/api/mir_config.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"

#include <eccodes.h>
#if mir_HAVE_NETCDF
#include "netcdf"
#endif


namespace mir::tools {


const uint64_t ICON_CODEC_VERSION = 0;


struct ICONData {
    std::vector<double> lat;
    std::vector<double> lon;
};


void read_grib(const std::string& path, ICONData& icon) {
    FILE* file = fopen(path.c_str(), "rb");
    ASSERT(file != nullptr);

    codes_handle* handle = nullptr;

    auto read_values = [](const codes_handle* handle, std::vector<double>& values) {
        ASSERT(values.empty());

        size_t n = 0;
        ASSERT(codes_get_size(handle, "values", &n) == CODES_SUCCESS && n > 0);

        values.resize(n);
        ASSERT(codes_get_double_array(handle, "values", values.data(), &n) == CODES_SUCCESS);
    };

    // Loop through all messages in the file
    for (int err = 0; (handle = codes_handle_new_from_file(nullptr, file, PRODUCT_GRIB, &err)) != nullptr;) {
        ASSERT(err == CODES_SUCCESS);

        char shortName[20] = {
            0,
        };
        size_t shortNameLen = sizeof(shortName);

        ASSERT(codes_get_string(handle, "shortName", shortName, &shortNameLen) == CODES_SUCCESS);
        const std::string s(shortName);
        if (s == "tlat") {
            read_values(handle, icon.lat);
        }
        else if (s == "tlon") {
            read_values(handle, icon.lon);
        }
    }

    ASSERT(icon.lat.size() == icon.lon.size());
    ASSERT(!icon.lat.empty());
}


void read_netcdf(const std::string& path, ICONData& icon) {
#if mir_HAVE_NETCDF
    using atttribute_t = std::variant<int, double, std::string>;

    static atttribute_t a_int{int{}};
    static atttribute_t a_double{double{}};
    static atttribute_t a_string{std::string{}};

    static const std::map<std::string, atttribute_t&> ATTRIBUTES{
        {"Creator", a_string},
        {"ICON_grid_file_uri", a_string},
        {"NCO", a_string},
        {"centre", a_int},
        {"crs_id", a_string},
        {"crs_name", a_string},
        {"ellipsoid_name", a_string},
        {"global_grid", a_int},
        {"grid_ID", a_int},
        {"grid_level", a_int},
        {"grid_mapping_name", a_string},
        {"grid_root", a_int},
        {"history", a_string},
        {"institution", a_string},
        {"inverse_flattening", a_double},
        {"max_childdom", a_int},
        {"max_refin_c_ctrl", a_int},
        {"number_of_grid_used", a_int},
        {"outname_style", a_int},
        {"parent_grid_ID", a_int},
        {"semi_major_axis", a_double},
        {"source", a_string},
        {"subcentre", a_int},
        {"title", a_string},
        {"uuidOfChiHGrid_1", a_string},
        {"uuidOfHGrid", a_string},
        {"uuidOfOriginalHGrid", a_string},
        {"uuidOfParHGrid", a_string},
    };


    try {
        auto read_values = [](const netCDF::NcVar& var, std::vector<double>& values) {
            ASSERT(values.empty());

            auto dim = var.getDims();
            ASSERT(dim.size() == 1);

            values.resize(dim[0].getSize());
            var.getVar(values.data());

            if (auto atts = var.getAtts(); atts.find("units") != atts.end()) {
                std::string units;
                var.getAtt("units").getValues(units);

                if (units == "radian") {
                    std::for_each(values.begin(), values.end(), [](double& angle) { angle *= 180. * M_1_PI; });
                }
            }
        };

        netCDF::NcFile f(path, netCDF::NcFile::read);
        eckit::JSON j(Log::info());

        for (const auto& att : f.getAtts()) {
            if (auto a = ATTRIBUTES.find(att.first); a != ATTRIBUTES.end()) {
                if (std::holds_alternative<int>(a->second)) {
                    int value = 0;
                    att.second.getValues(&value);
                    j << att.first << value;
                }
                else if (std::holds_alternative<double>(a->second)) {
                    double value = 0;
                    att.second.getValues(&value);
                    j << att.first << value;
                }
                else if (std::holds_alternative<std::string>(a->second)) {
                    std::string value;
                    att.second.getValues(value);
                    j << att.first << value;
                }
            }
        }

        Log::info() << std::endl;

        read_values(f.getVar("clat"), icon.lat);
        read_values(f.getVar("clon"), icon.lon);

        ASSERT(icon.lat.size() == icon.lon.size());
        ASSERT(!icon.lat.empty());
    }
    catch (netCDF::exceptions::NcException& e) {
        throw eckit::SeriousBug("NetCDF reading failed", Here());
    }
#else
    NOTIMP;
#endif
}


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

        // Detect format
        bool is_netcdf = [](const std::string& path) -> bool {
            std::ifstream f(path, std::ios::binary);
            ASSERT(f);

            int magic = 0;
            for (size_t i = 0; i < 4; ++i) {
                if (unsigned char c = 0; f >> c) {
                    magic <<= 8;
                    magic |= c;
                }
            }

            return magic == 0x89484446 || magic == 0x43444601 || magic == 0x43444602;
        }(args(0));

        // Read data
        ICONData icon;
        if (is_netcdf) {
            read_netcdf(args(0), icon);
        }
        else {
            read_grib(args(0), icon);
        }

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
