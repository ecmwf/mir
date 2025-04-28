/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "netcdf"

#include "eckit/codec/codec.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/runtime/Main.h"

#include "eckit/codec/codec.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Bytes.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "eccodes.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct uuid_t {
    explicit uuid_t(const std::string& str) : str_(str) {}
    explicit uuid_t(std::string&& str) : str_(str) {}

    //    uuid_t(const uuid_t& uuid) : uuid_t(uuid.str_) {}

    explicit operator std::string() const { return str_; }
    const std::string str_;
};


using attribute_map_t = std::map<std::string, std::variant<int, double, std::string, uuid_t>>;


std::ostream& operator<<(std::ostream& out, const attribute_map_t::value_type::second_type& p) {
    std::visit([&](const auto& p) { out << p; }, p);
    return out;
}


static const attribute_map_t ATTRIBUTES{{
    {"Creator", ""},
    {"ICON_grid_file_uri", ""},
    {"NCO", ""},
    {"centre", 0},
    {"crs_id", ""},
    {"crs_name", ""},
    {"ellipsoid_name", ""},
    {"global_grid", 0},
    {"grid_ID", 0},
    {"grid_level", 0},
    {"grid_mapping_name", ""},
    {"grid_root", 0},
    {"history", ""},
    {"institution", ""},
    {"inverse_flattening", double{}},
    {"max_childdom", 0},
    {"max_refin_c_ctrl", 0},
    {"number_of_grid_used", 0},
    {"outname_style", 0},
    {"parent_grid_ID", 0},
    {"semi_major_axis", double{}},
    {"source", ""},
    {"subcentre", 0},
    {"title", ""},
    {"uuidOfChiHGrid_1", uuid_t{""}},
    {"uuidOfHGrid", uuid_t{""}},
    {"uuidOfOriginalHGrid", uuid_t{""}},
    {"uuidOfParHGrid", uuid_t{""}},
}};


struct MIRNcToCodec : public MIRTool {
    using MIRTool::MIRTool;

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool << "--lat=VAR --lon=VAR FILE [FILE]" << std::endl;
    }

    MIRNcToCodec(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::SimpleOption;
        options_.push_back(new SimpleOption<std::string>("lat", "NetCDF variable for latitude (default 'clat')"));
        options_.push_back(new SimpleOption<std::string>("lon", "NetCDF variable for longitude (default 'clon')"));
        options_.push_back(new SimpleOption<bool>("ignore-id", "Ignore recognizing latitude/longitude paramId"));
    }

    int numberOfPositionalArguments() const override { return -1; }
    int minimumPositionalArguments() const override { return 1; }

    void execute(const eckit::option::CmdArgs& args) override {
        ASSERT(args.count() == 1 || args.count() == 2);
        struct data_t {
            size_t version = 0;
            attribute_map_t attributes;
            std::vector<double> lat;
            std::vector<double> lon;
        } data;


        // read
        eckit::PathName fn = args(0);
        if (!fn.exists()) {
            throw eckit::ReadError(fn, Here());
        }

        try {
            auto ll = [](const netCDF::NcFile& f, const std::string& name, std::vector<double>& value) {
                auto var = f.getVar(name);
                auto dim = var.getDims();
                ASSERT(dim.size() == 1);

                value.resize(dim[0].getSize());
                var.getVar(value.data());

                if (auto atts = var.getAtts(); atts.find("units") != atts.end()) {
                    std::string units;
                    var.getAtt("units").getValues(units);

                    if (units == "radian") {
                        std::for_each(value.begin(), value.end(), [](double& angle) { angle *= 180. * M_1_PI; });
                    }
                }
            };

            netCDF::NcFile f(fn.asString(), netCDF::NcFile::read);

            for (const auto& att : f.getAtts()) {
                if (auto a = ATTRIBUTES.find(att.first); a != ATTRIBUTES.end()) {
                    if (std::holds_alternative<int>(a->second)) {
                        int value = 0;
                        att.second.getValues(&value);
                        data.attributes[att.first] = value;
                    }
                    else if (std::holds_alternative<double>(a->second)) {
                        double value = 0;
                        att.second.getValues(&value);
                        data.attributes[att.first] = value;
                    }
                    else if (std::holds_alternative<std::string>(a->second) ||
                             std::holds_alternative<uuid_t>(a->second)) {
                        std::string value;
                        att.second.getValues(value);
                        data.attributes[att.first] = value;
                    }
                    else if (std::holds_alternative<uuid_t>(a->second)) {
                        std::string value;
                        att.second.getValues(&value);
                        data.attributes[att.first] = value;
                    }
                }
            }

            for (const auto& a : data.attributes) {
                std::cout << "'" << a.first << "': " << a.second << std::endl;
            }

            // will be used on output
            if (args.count() == 2) {
                ll(f, args.getString("lat", "clat"), data.lat);
                ll(f, args.getString("lon", "clon"), data.lon);
                ASSERT(data.lat.size() == data.lon.size());
            }
        }
        catch (netCDF::exceptions::NcException& e) {
            throw eckit::SeriousBug("NetCDF reading failed", Here());
        }


        // write
        if (args.count() == 2) {
            const auto version = static_cast<int32_t>(data.version);
            const std::vector<int32_t> shape{static_cast<int32_t>(data.lat.size())};

            struct Configuration : eckit::LocalConfiguration {
                Configuration() { set("compression", "none"); }
            } static const config;

            eckit::codec::RecordWriter record;
            record.set("version", eckit::codec::copy(version), config);
            record.set("shape", eckit::codec::copy(shape), config);
            record.set("longitude", eckit::codec::ref(data.lon), config);
            record.set("latitude", eckit::codec::ref(data.lat), config);

            eckit::PathName out(args(1));
            auto length = record.write(out);
            ASSERT(out.exists());

            Log::info() << "Written " << eckit::Bytes(static_cast<double>(length)) << " to '" << out << "'"
                        << std::endl;
        }
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRNcToCodec tool(argc, argv);
    return tool.start();
}
