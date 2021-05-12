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


#include "mir/input/MIRInput.h"

#include <cstdio>
#include <iomanip>
#include <sstream>

#include "eckit/io/StdFile.h"
#include "eckit/parser/YAMLParser.h"

#include "mir/input/ArtificialInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/MultiDimensionalGribFileInput.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"
#include "mir/util/ValueMap.h"


namespace mir {
namespace input {


MIRInput::MIRInput() = default;


MIRInput::~MIRInput() = default;


grib_handle* MIRInput::gribHandle(size_t) const {
    // ASSERT(which == 0);
    static grib_handle* handle = nullptr;
    if (handle == nullptr) {
        handle = codes_grib_handle_new_from_samples(nullptr, "GRIB1");

        constexpr long MISSING = 255;
        codes_set_long(handle, "paramId", MISSING);
        ASSERT(handle);
    }
    return handle;
}


void MIRInput::setAuxiliaryInformation(const util::ValueMap&) {
    std::ostringstream os;
    os << "MIRInput::setAuxiliaryInformation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool MIRInput::next() {
    std::ostringstream os;
    os << "MIRInput::next() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool MIRInput::only(size_t) {
    std::ostringstream os;
    os << "MIRInput::only() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


size_t MIRInput::copy(double*, size_t) const {
    std::ostringstream os;
    os << "MIRInput::copy() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


size_t MIRInput::dimensions() const {
    std::ostringstream os;
    os << "MIRInput::dimensions() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


static util::once_flag once;
static util::recursive_mutex* local_mutex           = nullptr;
static std::map<unsigned long, MIRInputFactory*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<unsigned long, MIRInputFactory*>();
}


MIRInputFactory::MIRInputFactory(unsigned long magic) : magic_(magic) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(magic) != m->end()) {
        std::ostringstream oss;
        oss << "MIRInputFactory: duplicate '" << std::hex << magic << "'";
        throw exception::SeriousBug(oss.str());
    }

    (*m)[magic] = this;
}


MIRInputFactory::~MIRInputFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(magic_);
}


static void put(std::ostream& out, unsigned long magic) {
    out << "0x" << std::hex << std::setfill('0') << std::setw(8) << magic << std::dec << std::setfill(' ');

    unsigned char p[5] = {
        0,
    };

    for (int i = 3; i >= 0; i--) {
        unsigned char c = magic & 0xff;
        magic >>= 8;
        p[i] = isprint(c) != 0 ? c : '.';
    }

    out << " (" << p << ")";
}


MIRInput* MIRInputFactory::build(const std::string& path, const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    util::ValueMap map;
    std::string input;
    if (param.get("input", input) && !input.empty()) {
        map = eckit::YAMLParser::decodeString(input);
    }

    // attach information after construction (pe. extra files), so virtual methods are specific to child class
    auto aux = [&map](MIRInput* in) {
        ASSERT(in);
        if (!map.empty()) {
            in->setAuxiliaryInformation(map);
        }
        return in;
    };

    // Special case: artificial input
    auto ai = map.find("artificialInput");
    if (ai != map.end() && ai->second.isString()) {
        return aux(ArtificialInputFactory::build(ai->second, param));
    }

    // Special case: multi-dimensional input
    auto md  = map.find("multiDimensional");
    size_t N = md != map.end() && md->second.isNumber() ? size_t(md->second) : 1;
    ASSERT(N > 0);

    bool uv2uv  = false;
    bool vod2uv = false;
    param.get("uv2uv", uv2uv);
    param.get("vod2uv", vod2uv);

    if (uv2uv || vod2uv) {
        ASSERT(uv2uv != vod2uv);
        N *= 2;
    }

    if (N > 1) {
        return aux(new MultiDimensionalGribFileInput(path, N));
    }

    eckit::AutoStdFile f(path);
    unsigned long magic    = 0;
    unsigned char smagic[] = "????";
    unsigned char* p       = smagic;

    for (size_t i = 0; i < 4; i++) {
        unsigned char c;
        if (std::fread(&c, 1, 1, f) > 0) {
            magic <<= 8;
            magic |= c;
            p[i] = isprint(c) != 0 ? c : '.';
        }
    }

    std::ostringstream oss;
    oss << "0x" << std::hex << magic << std::dec << " (" << smagic << ")";
    Log::debug() << "MIRInputFactory: looking for '" << oss.str() << "'" << std::endl;

    auto j = m->find(magic);
    if (j == m->end()) {
        list(Log::warning() << "MIRInputFactory: unknown '" << oss.str() << "', choices are: ");
        Log::warning() << std::endl;

        Log::warning() << "MIRInputFactory: assuming 'GRIB'" << std::endl;
        return aux(new GribFileInput(path));
    }

    return aux(j->second->make(path));
}


void MIRInputFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep;
        put(out, j.first);
        sep = ", ";
    }
}


}  // namespace input
}  // namespace mir
