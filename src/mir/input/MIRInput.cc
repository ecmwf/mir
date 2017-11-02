/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/input/MIRInput.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/StdFile.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include "mir/config/LibMir.h"
#include "mir/input/GribFileInput.h"
#include "mir/util/Grib.h"


namespace mir {
namespace input {


MIRInput::MIRInput() {
}


MIRInput::~MIRInput() {
}


grib_handle *MIRInput::gribHandle(size_t which) const {
    //ASSERT(which == 0);
    static grib_handle *handle = 0;
    if (!handle) {
        handle = grib_handle_new_from_samples(0, "GRIB1");
        grib_set_long(handle, "paramId", 255);
        ASSERT(handle);
    }
    return handle;
}


bool MIRInput::next() {
    std::ostringstream os;
    MIRInput &self = *this;
    os << "MIRInput::next() not implemented for " << self;
    throw eckit::SeriousBug(os.str());
}

size_t MIRInput::copy(double *values, size_t size) const {
    std::ostringstream os;
    os << "MIRInput::copy() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


size_t MIRInput::dimensions() const {
    std::ostringstream os;
    os << "MIRInput::dimensions() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void MIRInput::userRules(const param::Rules* rules) {
    std::ostringstream os;
    os << "MIRInput::dimensions() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}
//=========================================================================


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map< unsigned long, MIRInputFactory* >* m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<unsigned long, MIRInputFactory *>();
}
}  // (anonymous namespace)


MIRInputFactory::MIRInputFactory(unsigned long magic):
    magic_(magic) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(magic) != m->end()) {
        std::ostringstream oss;
        oss << "MIRInputFactory: duplicate '" << std::hex << magic << "'";
        throw eckit::SeriousBug(oss.str());
    }

    (*m)[magic] = this;
}


MIRInputFactory::~MIRInputFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->erase(magic_);
}


static void put(std::ostream& out, unsigned long magic) {
    out << "0x" << std::hex <<  std::setfill('0') << std::setw(8)  << magic << std::dec;

    char p[5] = {0,};

    for (int i = 3; i >= 0; i--) {
        unsigned char c = magic & 0xff;
        magic >>= 8;
        if (isprint(c)) {
            p[i] = c;
        } else {
            p[i] = '.';
        }
    }

    out << " (" << p << ")";
}


MIRInput *MIRInputFactory::build(const std::string& path) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::StdFile f(path);
    unsigned long magic = 0;
    char smagic[] = "????";
    char *p = smagic;

    for (size_t i = 0; i < 4; i++) {
        unsigned char c;
        if (fread(&c, 1, 1, f)) {
            magic <<= 8;
            magic |= c;

            if (isprint(c)) {
                *p++ = c;
            } else {
                *p++ = '.';
            }
        }
    }

    std::ostringstream oss;
    oss << "0x" << std::hex << magic << std::dec << " (" << smagic << ")";
    eckit::Log::debug<LibMir>() << "MIRInputFactory: looking for '" << oss.str() << "'" << std::endl;

    auto j = m->find(magic);
    if (j == m->end()) {
        list(eckit::Log::warning() << "MIRInputFactory: unknown '" << oss.str() << "', choices are: ");
        eckit::Log::warning() << std::endl;

        eckit::Log::warning() << "MIRInputFactory: assuming 'GRIB'" << std::endl;
        return new GribFileInput(path);
    }

    return (*j).second->make(path);
}


void MIRInputFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep;
        put(out, j.first);
        sep = ", ";
    }
}




}  // namespace input
}  // namespace mir

