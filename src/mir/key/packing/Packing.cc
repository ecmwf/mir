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
#include <ostream>
#include <set>
#include <sstream>

#include "mir/param/MIRParametrisation.h"
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


Packing::Packing(const param::MIRParametrisation& param) :
    defineAccuracy_(param.get("accuracy", accuracy_)),
    defineEdition_(param.get("edition", edition_)),
    definePacking_(param.get("packing", packing_)),
    gridded_(param.userParametrisation().has("grid") || param.fieldParametrisation().has("gridded")) {}


Packing::~Packing() = default;


bool Packing::sameAs(Packing* other) const {
    if (definePacking_ != other->definePacking_ || defineAccuracy_ != other->defineAccuracy_ ||
        defineEdition_ != other->defineEdition_) {
        return false;
    }
    return (definePacking_ && packing_ == other->packing_) && (defineAccuracy_ && accuracy_ == other->accuracy_) &&
           (defineEdition_ && edition_ == other->edition_);
}


bool Packing::printParametrisation(std::ostream& out) const {
    out << "packing=" << packing_;
    if (defineEdition_) {
        out << ",edition=" << edition_;
    }
    if (defineAccuracy_) {
        out << ",accuracy=" << accuracy_;
    }
    return true;
}


void Packing::setAccuracy(long value) {
    accuracy_       = value;
    defineAccuracy_ = true;
}


void Packing::setEdition(long value) {
    accuracy_      = value;
    defineEdition_ = true;
}


void Packing::setPacking(const std::string& value) {
    packing_       = value;
    definePacking_ = true;
}


void Packing::saveAccuracy(grib_info& info) const {
    if (defineAccuracy_) {
        info.packing.accuracy     = CODES_UTIL_ACCURACY_USE_PROVIDED_BITS_PER_VALUES;
        info.packing.bitsPerValue = accuracy_;
    }
    else {
        info.packing.accuracy = CODES_UTIL_ACCURACY_SAME_BITS_PER_VALUES_AS_INPUT;
    }
}


void Packing::saveEdition(grib_info& info) const {
    if (defineEdition_) {
        info.packing.editionNumber = edition_;
    }
    else {
        info.packing.editionNumber = 0;
    }
}


void Packing::savePacking(grib_info& info, long pack) const {
    if (definePacking_) {
        info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
        info.packing.packing_type = pack;
    }
    else {
        info.packing.packing = CODES_UTIL_PACKING_SAME_AS_INPUT;
    }
}


void Packing::setAccuracy(grib_handle* h) const {
    if (defineAccuracy_) {
        GRIB_CALL(codes_set_long(h, "bitsPerValue", accuracy_));
    }
}


void Packing::setEdition(grib_handle* h) const {
    if (defineEdition_) {
        GRIB_CALL(codes_set_long(h, "edition", edition_));
    }
}


void Packing::setPacking(grib_handle* h, const std::string& type) const {
    if (definePacking_) {
        auto len = type.length();
        GRIB_CALL(codes_set_string(h, "packingType", type.c_str(), &len));
    }
}


bool Packing::getAccuracy(long& value) const {
    if (defineAccuracy_) {
        value = accuracy_;
    }
    return defineAccuracy_;
}


bool Packing::getEdition(long& value) const {
    if (defineEdition_) {
        value = edition_;
    }
    return defineEdition_;
}


bool Packing::getPacking(std::string& value) const {
    if (definePacking_) {
        value = packing_;
    }
    return definePacking_;
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


Packing* PackingFactory::build(const std::string& name, const param::MIRParametrisation& param) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    Log::debug() << "PackingFactory: looking for '" << name << "'" << std::endl;

    auto list = [](std::ostream& out, const std::map<std::string, PackingFactory*>& m) {
        for (const auto& j : m) {
            out << ", " << j.first;
        }
    };

    bool gridded = param.userParametrisation().has("grid") || param.fieldParametrisation().has("gridded");
    if (gridded) {
        auto j = mg->find(name);
        if (j != mg->end()) {
            return j->second->make(param);
        }

        list(Log::error() << "PackingFactory: unknown gridded packing '" << name << "', choices are: ", *mg);
        throw exception::SeriousBug("PackingFactory: unknown gridded packing '" + name + "'");
    }

    auto j = ms->find(name);
    if (j != ms->end()) {
        return j->second->make(param);
    }

    list(Log::error() << "PackingFactory: unknown spectral packing '" << name << "', choices are: ", *ms);
    throw exception::SeriousBug("PackingFactory: unknown spectral packing '" + name + "'");
}


void PackingFactory::list(std::ostream& out) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    std::set<std::string> p;
    p.insert("archived-value");
    p.insert("av");

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


}  // namespace packing
}  // namespace key
}  // namespace mir
