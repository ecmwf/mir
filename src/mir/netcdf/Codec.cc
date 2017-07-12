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
#include "mir/netcdf/Exceptions.h"

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Once.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"

#include <iostream>

namespace mir {
namespace netcdf {

static eckit::Mutex *local_mutex = 0;
static std::map<std::string, CodecFactory *> *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, CodecFactory *>();
}



Codec::Codec() {
}

Codec::~Codec() {
}

void Codec::decode(std::vector<double> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::decode(std::vector<float> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::decode(std::vector<long> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::decode(std::vector<short> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::decode(std::vector<unsigned char> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::decode(std::vector<long long> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::encode(std::vector<double> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::encode(std::vector<float> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::encode(std::vector<long> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::encode(std::vector<short> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::encode(std::vector<unsigned char> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::encode(std::vector<long long> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::addAttributes(Variable &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::updateAttributes(int nc, int varid, const std::string &path) {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

bool Codec::timeAxis() const {
    return false;
}

//=================================================================

CodecFactory::CodecFactory(const std::string &name):
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("CodecFactory: duplication action: " + name);
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


    auto j = m->find(name);
    if (j == m->end()) {


        if (j == m->end()) {
            eckit::Log::error() << "No CodecFactory for [" << name << "]" << std::endl;
            eckit::Log::error() << "CodecFactories are:" << std::endl;
            for (j = m->begin() ; j != m->end() ; ++j)
                eckit::Log::error() << "   " << (*j).first << std::endl;
            throw eckit::SeriousBug(std::string("No CodecFactory called ") + name);
        }
    }

    return (*j).second->make(variable);
}


void CodecFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map<std::string, CodecFactory *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}
}
}
