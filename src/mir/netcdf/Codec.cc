/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015


#include "mir/netcdf/Codec.h"

#include <iostream>

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include "mir/config/LibMir.h"
#include "mir/netcdf/Exceptions.h"


namespace mir {
namespace netcdf {

Codec::Codec() {
}

Codec::~Codec() {
}

void Codec::decode(std::vector<double> &) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<double> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::decode(std::vector<float> &) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<float> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::decode(std::vector<long> &) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<long> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::decode(std::vector<short> &) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<short> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::decode(std::vector<unsigned char> &) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<unsigned char> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::decode(std::vector<long long> &) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<long long> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::encode(std::vector<double> &) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<double> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::encode(std::vector<float> &) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<float> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::encode(std::vector<long> &) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<long> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::encode(std::vector<short> &) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<short> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::encode(std::vector<unsigned char> &) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<unsigned char> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::encode(std::vector<long long> &) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<long long> &) not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::addAttributes(Variable &) const {
    std::ostringstream os;
    os << "Variable::addAttributes() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Codec::updateAttributes(int nc, int varid, const std::string &path) {
    std::ostringstream os;
    os << "Variable::updateAttributes() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

bool Codec::timeAxis() const {
    return false;
}


//=========================================================================


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map< std::string, CodecFactory* >* m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, CodecFactory* >();
}
}  // (anonymous namespace)


CodecFactory::CodecFactory(const std::string &name):
    name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("CodecFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


CodecFactory::~CodecFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->erase(name_);
}


Codec *CodecFactory::build(const std::string& name, const Variable& variable) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "CodecFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "CodecFactory: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("CodecFactory: unknown '" + name + "'");
    }

    return (*j).second->make(variable);
}


void CodecFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}
}
