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


#include "mir/input/GribFixes.h"

#include <algorithm>
#include <string>

#include "eckit/filesystem/PathName.h"
#include "eckit/log/JSON.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Translator.h"

#include "mir/config/LibMir.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"
#include "mir/util/ValueMap.h"


namespace mir::input {


GribFixes::GribFixes() {
    readConfigurationFiles();
}


GribFixes::~GribFixes() {
    for (auto& rule : fixes_) {
        delete rule.first;
        delete rule.second;
    }
}


const param::SimpleParametrisation& GribFixes::find(const param::MIRParametrisation& param) const {
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

    if (fixes->size() > 0) {
        Log::warning() << "GribFixes: " << *fixes << std::endl;
    }

    return *fixes;
}


void GribFixes::print(std::ostream& s) const {
    s << "GribFixes";
    eckit::JSON json(s);

    json.startObject();
    for (const auto& fix : fixes_) {
        json << *(fix.first) << *(fix.second);
    }
    json.endObject();
}


void GribFixes::readConfigurationFiles() {
    static util::recursive_mutex mtx;
    util::lock_guard<util::recursive_mutex> lock(mtx);

    ASSERT(fixes_.empty());

    const eckit::PathName path = LibMir::configFile(LibMir::config_file::GRIB);
    if (!path.exists()) {
        return;
    }

    // value type conversions
    using eckit::StringTools;

    eckit::Translator<std::string, long> i;
    eckit::Translator<std::string, double> d;

    auto k = [](const std::string& key) {
        ASSERT(key.size() >= 2);
        return key.substr(0, key.size() - 2);
    };

    auto vi = [&i](const std::vector<std::string>& values) {
        std::vector<long> v(values.size());
        std::transform(values.begin(), values.end(), v.begin(), [&i](const std::string& s) { return i(s); });
        return v;
    };

    auto vd = [&d](const std::vector<std::string>& values) {
        std::vector<double> v(values.size());
        std::transform(values.begin(), values.end(), v.begin(), [&d](const std::string& s) { return d(s); });
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
                StringTools::endsWith(key, ":i")   ? id->set(k(key), i(value))
                : StringTools::endsWith(key, ":d") ? id->set(k(key), d(value))
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


}  // namespace mir::input
