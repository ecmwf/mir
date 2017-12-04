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
/// @author Tiago Quintino
/// @date   Oct 2016


#include "mir/caching/InMemoryCacheUsage.h"
#include "eckit/serialisation/Stream.h"
#include "eckit/log/Bytes.h"
#include "eckit/utils/Translator.h"
#include "eckit/parser/Tokenizer.h"

namespace mir {

//----------------------------------------------------------------------------------------------------------------------


InMemoryCacheUsage::InMemoryCacheUsage():
    memory_(0),
    shared_(0) {

}

InMemoryCacheUsage::InMemoryCacheUsage(unsigned long long memory, unsigned long long shared):
    memory_(memory),
    shared_(shared) {

}

InMemoryCacheUsage::InMemoryCacheUsage(unsigned long long size, bool inSharedMemory):
    memory_(inSharedMemory ? 0 : size),
    shared_(inSharedMemory ? size : 0) {
}

InMemoryCacheUsage::InMemoryCacheUsage(const std::string& s):
    memory_(0),
    shared_(0) {

    eckit::Tokenizer parse(",");
    eckit::Translator<std::string, unsigned long long> s2l;

    std::vector<std::string> v;
    parse(s, v);

    if (v.size() >= 1) {
        memory_ = s2l(v[0]);
    }

    if (v.size() >= 2) {
        shared_ = s2l(v[1]);
    }

}

InMemoryCacheUsage::operator std::string() const {
    std::ostringstream oss;
    oss << memory_ << "," << shared_;
    return oss.str();
}


InMemoryCacheUsage::InMemoryCacheUsage(eckit::Stream &s)  {
    s >> memory_;
    s >> shared_;
}

InMemoryCacheUsage &InMemoryCacheUsage::operator+=(const InMemoryCacheUsage &rhs)  {
    memory_ += rhs.memory_;
    shared_ += rhs.shared_;
    return *this;
}

InMemoryCacheUsage &InMemoryCacheUsage::operator/=(size_t n) {
    memory_ /= n;
    shared_ /= n;
    return *this;
}

InMemoryCacheUsage InMemoryCacheUsage::operator/(size_t n) const {
    return  InMemoryCacheUsage(memory_ / n, shared_ / n);
}

InMemoryCacheUsage InMemoryCacheUsage::operator-(const InMemoryCacheUsage& other) const {

    ASSERT(memory_ >= other.memory_);
    ASSERT(shared_ >= other.shared_);

    return  InMemoryCacheUsage(memory_ - other.memory_, shared_ - other.shared_);
}


bool InMemoryCacheUsage::operator>(const InMemoryCacheUsage& other) const {
    // Warning, this is not a complete order, don't use to sort
    if (memory_ > other.memory_) {
        return true;
    }

    if (shared_ > other.shared_) {
        return true;
    }

    return false;
}

bool InMemoryCacheUsage::operator !() const {
    return !(*this);
}

InMemoryCacheUsage::operator bool() const {
    return memory_ || shared_;
}


void InMemoryCacheUsage::encode(eckit::Stream &s) const {
    s << memory_;
    s << shared_;
}

void InMemoryCacheUsage::decode(eckit::Stream &s) {
    s >> memory_;
    s >> shared_;
}

void InMemoryCacheUsage::print(std::ostream& out) const {
    out << "[memory=" << eckit::Bytes(memory_) << ",shared=" << eckit::Bytes(shared_) << "]";
}

unsigned long long InMemoryCacheUsage::memory() const {
    return memory_;
}

unsigned long long InMemoryCacheUsage::shared() const {
    return shared_;
}


//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir


