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


#include "mir/netcdf/Codec.h"

#include <map>
#include <ostream>
#include <sstream>

#include "mir/netcdf/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::netcdf {


Codec::Codec() = default;


Codec::~Codec() = default;


void Codec::decode(std::vector<double>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<double> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::decode(std::vector<float>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<float> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::decode(std::vector<long>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<long> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::decode(std::vector<short>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<short> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::decode(std::vector<unsigned char>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<unsigned char> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::decode(std::vector<long long>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::decode(std::vector<long long> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::encode(std::vector<double>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<double> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::encode(std::vector<float>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<float> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::encode(std::vector<long>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<long> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::encode(std::vector<short>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<short> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::encode(std::vector<unsigned char>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<unsigned char> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::encode(std::vector<long long>& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::encode(std::vector<long long> &) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::addAttributes(Variable& /*unused*/) const {
    std::ostringstream os;
    os << "Variable::addAttributes() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Codec::updateAttributes(int /*nc*/, int /*varid*/, const std::string& /*path*/) {
    std::ostringstream os;
    os << "Variable::updateAttributes() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool Codec::timeAxis() const {
    return false;
}


static util::once_flag once;
static util::recursive_mutex* local_mutex      = nullptr;
static std::map<std::string, CodecFactory*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, CodecFactory*>();
}


CodecFactory::CodecFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (auto j = m->find(name); j == m->end()) {
        (*m)[name] = this;
        return;
    }

    throw exception::SeriousBug("CodecFactory: duplicate '" + name + "'");
}


CodecFactory::~CodecFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


Codec* CodecFactory::build(const std::string& name, const Variable& variable) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "CodecFactory: looking for '" << name << "'" << std::endl;

    if (auto j = m->find(name); j != m->end()) {
        return j->second->make(variable);
    }

    list(Log::error() << "CodecFactory: unknown '" << name << "', choices are: ");
    throw exception::SeriousBug("CodecFactory: unknown '" + name + "'");
}


void CodecFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << "'" << j.first << "'";
        sep = ", ";
    }
}


}  // namespace mir::netcdf
