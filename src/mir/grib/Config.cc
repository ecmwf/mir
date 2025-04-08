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


#include "mir/grib/Config.h"

#include <algorithm>
#include <string>

#include "eckit/filesystem/PathName.h"
#include "eckit/log/JSON.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/utils/StringTools.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"
#include "mir/util/Translator.h"
#include "mir/util/ValueMap.h"


namespace mir::grib {


Config::Config(const eckit::PathName& path, bool silent) : silent_(silent) {
    readConfigurationFiles(path);
}


Config::~Config() {
    for (auto& rule : fixes_) {
        delete rule.first;
        delete rule.second;
    }
}


const param::SimpleParametrisation& Config::find(const param::MIRParametrisation& param) const {
    static util::recursive_mutex mtx;
    util::lock_guard<util::recursive_mutex> lock(mtx);

    // select best fix by number of matching keys
    static const param::SimpleParametrisation empty;
    size_t match      = 0;
    const auto* fixes = &empty;

    for (const auto& f : fixes_) {
        if ((f.first)->matchAny(param) && match < (f.first)->size()) {
            ASSERT(f.second);
            match = (f.first)->size();
            fixes = f.second;
        }
    }

    if (!silent_ && fixes->size() > 0) {
        Log::warning() << "Config: " << *fixes << std::endl;
    }

    return *fixes;
}


void Config::print(std::ostream& s) const {
    eckit::JSON json(s);
    json.startObject();
    for (const auto& fix : fixes_) {
        json << *(fix.first) << *(fix.second);
    }
    json.endObject();
}


void Config::readConfigurationFiles(const eckit::PathName& path) {
    static util::recursive_mutex mtx;
    util::lock_guard<util::recursive_mutex> lock(mtx);

    ASSERT(fixes_.empty());

    if (!path.exists()) {
        return;
    }

    // value type conversions
    using eckit::StringTools;

    auto k = [](const std::string& key) {
        ASSERT(key.size() >= 2);
        return key.substr(0, key.size() - 2);
    };

    auto vi = [](const std::vector<std::string>& values) {
        std::vector<long> v(values.size());
        std::transform(values.begin(), values.end(), v.begin(),
                       [](const std::string& s) { return util::from_string<long>(s); });
        return v;
    };

    auto vd = [](const std::vector<std::string>& values) {
        std::vector<double> v(values.size());
        std::transform(values.begin(), values.end(), v.begin(),
                       [](const std::string& s) { return util::from_string<double>(s); });
        return v;
    };

    eckit::ValueMap rules(eckit::YAMLParser::decodeFile(path));
    for (const auto& rule : rules) {

        // how the input is to be identified
        auto* id = new param::SimpleParametrisation;
        ASSERT(id);

        for (auto& ids : StringTools::split(",", rule.first)) {
            auto keyValue = StringTools::split("=", ids);
            ASSERT(keyValue.size() == 2);

            auto key   = StringTools::trim(keyValue[0]);
            auto value = StringTools::trim(keyValue[1]);

            if (value.find('/') != std::string::npos) {
                auto values = StringTools::split("/", value);
                StringTools::endsWith(key, ":i")   ? id->set(k(key), vi(values))
                : StringTools::endsWith(key, ":d") ? id->set(k(key), vd(values))
                                                   : id->set(key, values);
            }
            else {
                StringTools::endsWith(key, ":i")   ? id->set(k(key), util::from_string<long>(value))
                : StringTools::endsWith(key, ":d") ? id->set(k(key), util::from_string<double>(value))
                                                   : id->set(key, value);
            }
        }

        // how the input is to be corrected
        auto* fix = new param::SimpleParametrisation;
        ASSERT(fix);

        for (const auto& fixes : static_cast<const eckit::ValueList&>(rule.second)) {
            util::ValueMap map(fixes);
            map.set(*fix);
        }

        fixes_.emplace_back(id, fix);
    }
}


}  // namespace mir::grib
