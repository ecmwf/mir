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


#include "mir/key/Area.h"

#include <array>
#include <map>
#include <ostream>
#include <vector>

#include "eckit/filesystem/PathName.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/utils/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::key {


struct map_t : std::map<std::string, std::array<double, 4>> {
    map_t() {
        const auto path = LibMir::configFile(LibMir::config_file::AREA);
        if (!path.exists()) {
            return;
        }

        for (const auto& kv : eckit::ValueMap(eckit::YAMLParser::decodeFile(path))) {
            auto k = kv.first.as<std::string>();
            auto v = kv.second.as<eckit::ValueList>();
            ASSERT_KEYWORD_AREA_SIZE(v.size());

            operator[](k) = {v[0], v[1], v[2], v[3]};
        }
    }
};

static map_t* m                   = nullptr;
static util::recursive_mutex* mtx = nullptr;
static util::once_flag once;


static void init() {
    mtx = new util::recursive_mutex();
    m   = new map_t;
}


void Area::Mode::list(std::ostream& out) {
    out << "crop, mask";
}


std::string Area::action(const param::MIRParametrisation& param) {
    std::string mode = "crop";
    param.get("area-mode", mode);

    if (mode == "crop") {
        return "crop.area";
    }

    if (mode == "mask") {
        return "mask.area";
    }

    list(Log::error() << "Area: unknown '" << mode << "', choices are: ");
    throw exception::UserError("Area: unknown '" + mode + "'");
}


bool Area::get(const param::MIRParametrisation& param, util::BoundingBox& bbox) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*mtx);

    std::string value;
    if (!param.get("area", value)) {
        return false;
    }

    if (match(value, bbox)) {
        return true;
    }

    eckit::Translator<std::string, double> t;
    std::vector<std::string> values;

    eckit::Tokenizer("/")(value, values);
    ASSERT_KEYWORD_AREA_SIZE(values.size());

    bbox = {t(values[0]), t(values[1]), t(values[2]), t(values[3])};
    return true;
}


bool Area::match(const std::string& key, util::BoundingBox& bbox) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*mtx);

    auto it = m->find(key);
    if (it != m->end()) {
        auto& a = it->second;
        bbox    = {a[0], a[1], a[2], a[3]};

        return true;
    }

    return false;
}


void Area::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*mtx);

    out << "North/West/South/East";

    const auto* sep = ", ";
    for (auto& j : *m) {
        out << sep << j.first;
    }
}


}  // namespace mir::key
