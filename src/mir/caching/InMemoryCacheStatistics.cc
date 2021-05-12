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


#include "mir/caching/InMemoryCacheStatistics.h"

#include "eckit/serialisation/Stream.h"


namespace mir {
namespace caching {


InMemoryCacheStatistics::InMemoryCacheStatistics() :
    hits_(0), misses_(0), evictions_(0), insertions_(0), oldest_(0), youngest_(0), unique_(0) {}


InMemoryCacheStatistics::InMemoryCacheStatistics(eckit::Stream& s) {
    s >> insertions_;
    s >> evictions_;
    s >> hits_;
    s >> misses_;
    s >> youngest_;
    s >> oldest_;
    s >> capacity_;
    s >> footprint_;
    s >> unique_;
    s >> required_;
}


void InMemoryCacheStatistics::encode(eckit::Stream& s) const {
    s << insertions_;
    s << evictions_;
    s << hits_;
    s << misses_;
    s << youngest_;
    s << oldest_;
    s << capacity_;
    s << footprint_;
    s << unique_;
    s << required_;
}


InMemoryCacheStatistics& InMemoryCacheStatistics::operator+=(const InMemoryCacheStatistics& other) {
    insertions_ += other.insertions_;
    evictions_ += other.evictions_;
    hits_ += other.hits_;
    misses_ += other.misses_;
    youngest_ += other.youngest_;
    oldest_ += other.oldest_;
    capacity_ += other.capacity_;
    footprint_ += other.footprint_;
    unique_ += other.unique_;
    required_ += other.required_;
    return *this;
}


InMemoryCacheStatistics& InMemoryCacheStatistics::operator/=(size_t n) {
    insertions_ /= n;
    evictions_ /= n;
    hits_ /= n;
    misses_ /= n;
    youngest_ /= double(n);
    oldest_ /= double(n);
    capacity_ /= n;
    footprint_ /= n;
    unique_ /= n;
    required_ /= n;
    return *this;
}


void InMemoryCacheStatistics::report(const char* title, std::ostream& out, const char* indent) const {

    std::string t(title);
    reportBytes(out, (t + ", capacity").c_str(), capacity_.memory(), indent);
    reportBytes(out, (t + ", capacity - shared").c_str(), capacity_.shared(), indent);

    reportBytes(out, (t + ", footprint").c_str(), footprint_.memory(), indent);
    reportBytes(out, (t + ", footprint - shared").c_str(), footprint_.shared(), indent);


    reportCount(out, (t + ", insertions").c_str(), insertions_, indent);
    reportCount(out, (t + ", evictions").c_str(), evictions_, indent);
    reportCount(out, (t + ", hits").c_str(), hits_, indent);
    reportCount(out, (t + ", misses").c_str(), misses_, indent);

    reportTime(out, (t + ", oldest eviction").c_str(), oldest_, indent);
    reportTime(out, (t + ", youngest eviction").c_str(), youngest_, indent);

    reportCount(out, (t + ", unique keys").c_str(), unique_, indent);

    reportBytes(out, (t + ", required").c_str(), required_.memory(), indent);
    reportBytes(out, (t + ", required - shared").c_str(), required_.shared(), indent);
}


}  // namespace caching
}  // namespace mir
