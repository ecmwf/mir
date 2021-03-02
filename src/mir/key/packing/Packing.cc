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


#include "mir/key/packing/Packing.h"

#include <map>
#include <mutex>
#include <set>
#include <sstream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Gridded.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir {
namespace key {
namespace packing {


static std::once_flag once;
static std::mutex* local_mutex                    = nullptr;
static std::map<std::string, PackingFactory*>* ms = nullptr;
static std::map<std::string, PackingFactory*>* mg = nullptr;
static void init() {
    local_mutex = new std::mutex();
    ms          = new std::map<std::string, PackingFactory*>();
    mg          = new std::map<std::string, PackingFactory*>();
}


Packing::Packing(const param::MIRParametrisation& param, bool gridded) :
    userPacking_(param.userParametrisation().get("packing", packing_) && packing_ != "av" &&
                 packing_ != "archived-value"),
    userAccuracy_(param.userParametrisation().get("accuracy", accuracy_)),
    userEdition_(param.userParametrisation().get("edition", edition_)),
    gridded_(gridded) {}


Packing::~Packing() = default;


void Packing::saveAccuracy(grib_info& info) const {
    if (userAccuracy_) {
        info.packing.accuracy     = CODES_UTIL_ACCURACY_USE_PROVIDED_BITS_PER_VALUES;
        info.packing.bitsPerValue = accuracy_;
    }
    else {
        info.packing.accuracy = CODES_UTIL_ACCURACY_SAME_BITS_PER_VALUES_AS_INPUT;
    }
}


void Packing::saveEdition(grib_info& info) const {
    if (userEdition_) {
        info.packing.editionNumber = edition_;
    }
    else {
        info.packing.editionNumber = 0;
    }
}


void Packing::savePacking(grib_info& info, long pack) const {
    if (userPacking_) {
        info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
        info.packing.packing_type = pack;
    }
    else {
        info.packing.packing = CODES_UTIL_PACKING_SAME_AS_INPUT;
    }
}


void Packing::setAccuracy(grib_handle* h) const {
    if (userAccuracy_) {
        GRIB_CALL(codes_set_long(h, "bitsPerValue", accuracy_));
    }
}


void Packing::setEdition(grib_handle* h) const {
    if (userEdition_) {
        GRIB_CALL(codes_set_long(h, "edition", edition_));
    }
}


void Packing::setPacking(grib_handle* h, const std::string& type) const {
    if (userPacking_) {
        auto len = type.length();
        GRIB_CALL(codes_set_string(h, "packingType", type.c_str(), &len));
    }
}


PackingFactory::PackingFactory(const std::string& name, const std::string& alias, bool spectral, bool gridded) :
    name_(name), spectral_(spectral), gridded_(gridded) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    ASSERT(gridded || spectral);

    if (gridded) {
        ASSERT_MSG(mg->insert({name, this}).second, "PackingFactory: duplicate gridded packing '" + name + "'");
        if (!alias.empty()) {
            ASSERT_MSG(mg->insert({alias, this}).second, "PackingFactory: duplicate gridded packing '" + name + "'");
        }
    }

    if (spectral) {
        ASSERT_MSG(ms->insert({name, this}).second, "PackingFactory: duplicate spectral packing '" + name + "'");
        if (!alias.empty()) {
            ASSERT_MSG(ms->insert({alias, this}).second, "PackingFactory: duplicate spectral packing '" + name + "'");
        }
    }
}


PackingFactory::~PackingFactory() {
    std::lock_guard<std::mutex> lock(*local_mutex);

    mg->erase(name_);
    ms->erase(name_);
}


Packing* PackingFactory::build(const std::string& name, const param::MIRParametrisation& param,
                               const repres::Representation* repres) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    Log::debug() << "PackingFactory: looking for '" << name << "'" << std::endl;

    auto list = [](std::ostream& out, const std::map<std::string, PackingFactory*>& m) {
        for (const auto& j : m) {
            out << ", " << j.first;
        }
    };

    ASSERT(repres != nullptr);
    bool gridded = dynamic_cast<const repres::Gridded*>(repres) != nullptr;
    if (gridded) {
        auto j = mg->find(name);
        if (j != mg->end()) {
            return j->second->make(param, true);
        }

        list(Log::error() << "PackingFactory: unknown gridded packing '" << name << "', choices are: ", *mg);
        throw exception::SeriousBug("PackingFactory: unknown gridded packing '" + name + "'");
    }

    auto j = ms->find(name);
    if (j != ms->end()) {
        return j->second->make(param, false);
    }

    list(Log::error() << "PackingFactory: unknown spectral packing '" << name << "', choices are: ", *ms);
    throw exception::SeriousBug("PackingFactory: unknown spectral packing '" + name + "'");
}


Packing* PackingFactory::build(const param::MIRParametrisation& param, const repres::Representation* repres) {
    std::string name;
    if (!get(name, param)) {
        bool gridded = dynamic_cast<const repres::Gridded*>(repres) != nullptr;
        name         = gridded ? "simple" : "complex";
    }
    return build(name, param, repres);
}


void PackingFactory::list(std::ostream& out) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    std::set<std::string> p;
    for (const auto& j : *ms) {
        p.insert(j.first);
    }
    for (const auto& j : *mg) {
        p.insert(j.first);
    }

    const char* sep = "";
    for (const auto& j : p) {
        out << sep << j;
        sep = ", ";
    }
}


bool PackingFactory::get(std::string& name, const param::MIRParametrisation& param) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    auto& user  = param.userParametrisation();
    auto& field = param.fieldParametrisation();

    // When converting from spectral to gridded, default to simple packing
    if (!user.has("packing") && user.has("grid") && field.has("spectral")) {
        name = "simple";
        return true;
    }

    // When user requests archived-value, return field packing
    std::string packing;
    if (param.get("packing", packing) && (packing == "av" || packing == "archived-value")) {
        return field.get("packing", name);
    }

    // Return only proper names, no alias
    auto i = mg->find(packing);
    if (i != mg->end()) {
        name = i->second->name_;
        return true;
    }

    auto j = ms->find(packing);
    if (j != ms->end()) {
        name = j->second->name_;
        return true;
    }

    return false;
}


}  // namespace packing
}  // namespace key
}  // namespace mir
