/*
 * (C) Copyright 2021- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include <set>
#include <string>
#include <vector>

#include "eckit/codec/codec.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Bytes.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "eccodes.h"

#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRGribToCodec : public MIRTool {
    using MIRTool::MIRTool;

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool << "--lat=FILE --lon=FILE FILE" << std::endl;
    }

    MIRGribToCodec(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::SimpleOption;
        options_.push_back(new SimpleOption<std::string>("lat", "GRIB file describing latitude"));
        options_.push_back(new SimpleOption<std::string>("lon", "GRIB file describing longitude"));
        options_.push_back(new SimpleOption<bool>("ignore-id", "Ignore recognizing latitude/longitude paramId"));
    }

    int numberOfPositionalArguments() const override { return 1; }

    void execute(const eckit::option::CmdArgs& args) override {
        const std::set<long> LATS{250001, 250003, 250005, 250007, 250009, 250011,
                                  260295, 260297, 260299, 260421, 260425, 500236};
        const std::set<long> LONS{250002, 250004, 250006, 250008, 250010, 250012,
                                  260296, 260298, 260300, 260422, 260426, 500237};

        auto field_values = [](const std::string& path, const std::set<long>& recognized_ids, bool ignore_id = false) {
            auto* file = fopen(path.c_str(), "rb");
            ASSERT(file != nullptr);

            int error = 0;
            auto* h   = codes_handle_new_from_file(nullptr, file, PRODUCT_GRIB, &error);
            if (h == nullptr || error != GRIB_SUCCESS) {
                throw eckit::ReadError(path + ", " + codes_get_error_message(error), Here());
            }

            if (!ignore_id) {
                long id = 0;
                CODES_CHECK(codes_get_long(h, "paramId", &id), "codes_get_long(\"paramId\")");
                if (auto it = recognized_ids.find(id); it == recognized_ids.end()) {
                    throw eckit::UserError("paramId=" + std::to_string(id) + " not recognized", Here());
                }
            }

            size_t len = 0;
            CODES_CHECK(codes_get_size(h, "values", &len), "codes_get_size(\"values\")");

            std::vector<double> values(len);
            CODES_CHECK(codes_get_double_array(h, "values", values.data(), &len), "codes_get_double_array(\"values\")");

            codes_handle_delete(h);
            fclose(file);

            return values;
        };

        const auto ignore_id = args.getBool("ignore-id", false);
        const auto lat       = field_values(args.getString("lat"), LATS, ignore_id);
        const auto lon       = field_values(args.getString("lon"), LONS, ignore_id);
        ASSERT(lon.size() == lat.size());

        const int32_t version = 0;
        const std::vector<int32_t> shape{static_cast<int32_t>(lat.size())};

        struct Configuration : eckit::LocalConfiguration {
            Configuration() { set("compression", "none"); }
        } static const config;

        eckit::codec::RecordWriter record;
        record.set("version", eckit::codec::copy(version), config);
        record.set("shape", eckit::codec::copy(shape), config);
        record.set("longitude", eckit::codec::ref(lon), config);
        record.set("latitude", eckit::codec::ref(lat), config);

        ASSERT(args.count() == 1);
        auto length = record.write(args(0));

        Log::info() << "Written " << eckit::Bytes(static_cast<double>(length)) << " to '" << args(0) << "'"
                    << std::endl;
    }
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRGribToCodec tool(argc, argv);
    return tool.start();
}
