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


#include "InMemoryCacheUsage.h"

#include <sstream>

#include "eckit/serialisation/Stream.h"
#include "eckit/utils/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "mir/util/Log.h"


namespace mir::caching {


InMemoryCacheUsage::InMemoryCacheUsage() : memory_(0), shared_(0) {}

InMemoryCacheUsage::InMemoryCacheUsage(size_t memory, size_t shared) : memory_(memory), shared_(shared) {}

InMemoryCacheUsage::InMemoryCacheUsage(const std::string& s) : memory_(0), shared_(0) {

    eckit::Tokenizer parse(",");
    eckit::Translator<std::string, size_t> s2l;

    std::vector<std::string> v;
    parse(s, v);

    if (!v.empty()) {
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


InMemoryCacheUsage::InMemoryCacheUsage(eckit::Stream& s) {
    s >> memory_;
    s >> shared_;
}

InMemoryCacheUsage& InMemoryCacheUsage::operator+=(const InMemoryCacheUsage& rhs) {
    memory_ += rhs.memory_;
    shared_ += rhs.shared_;
    return *this;
}

InMemoryCacheUsage& InMemoryCacheUsage::operator/=(size_t n) {
    memory_ /= n;
    shared_ /= n;
    return *this;
}

InMemoryCacheUsage InMemoryCacheUsage::operator/(size_t n) const {
    return InMemoryCacheUsage(memory_ / n, shared_ / n);
}

InMemoryCacheUsage InMemoryCacheUsage::operator-(const InMemoryCacheUsage& other) const {
    // Warning, this is not a real substraction

    size_t m = memory_ >= other.memory_ ? memory_ - other.memory_ : 0;
    size_t s = shared_ >= other.shared_ ? shared_ - other.shared_ : 0;

    return InMemoryCacheUsage(m, s);
}

InMemoryCacheUsage InMemoryCacheUsage::operator+(const InMemoryCacheUsage& other) const {

    size_t m = memory_ + other.memory_;
    size_t s = shared_ + other.shared_;

    return InMemoryCacheUsage(m, s);
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

bool InMemoryCacheUsage::operator!() const {
    return !bool(*this);
}

InMemoryCacheUsage::operator bool() const {
    return (memory_ != 0) || (shared_ != 0);
}


void InMemoryCacheUsage::encode(eckit::Stream& s) const {
    s << memory_;
    s << shared_;
}

void InMemoryCacheUsage::decode(eckit::Stream& s) {
    s >> memory_;
    s >> shared_;
}

void InMemoryCacheUsage::print(std::ostream& out) const {
    out << "[memory=" << Log::Bytes(memory_) << ",shared=" << Log::Bytes(shared_) << "]";
}

size_t InMemoryCacheUsage::memory() const {
    return memory_;
}

size_t InMemoryCacheUsage::shared() const {
    return shared_;
}

}  // namespace mir::caching
