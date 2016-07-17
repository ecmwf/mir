/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mir/caching/InMemoryCacheStatistics.h"
#include "eckit/serialisation/Stream.h"


namespace mir {

//----------------------------------------------------------------------------------------------------------------------

InMemoryCacheStatistics::InMemoryCacheStatistics():
    insertions_(0),
    evictions_(0),
    accesses_(0),
    youngest_(0),
    oldest_(0) {
}

InMemoryCacheStatistics::InMemoryCacheStatistics(eckit::Stream &s) {
    s >> insertions_;
    s >> evictions_;
    s >> accesses_;
    s >> youngest_;
    s >> oldest_;
}

void InMemoryCacheStatistics::encode(eckit::Stream &s) const {
    s << insertions_;
    s << evictions_;
    s << accesses_;
    s << youngest_;
    s << oldest_;
}

InMemoryCacheStatistics &InMemoryCacheStatistics::operator+=(const InMemoryCacheStatistics &other) {
    insertions_ += other.insertions_;
    evictions_ += other.evictions_;
    accesses_ += other.accesses_;
    if()
    youngest_ += other.youngest_;
    oldest_ += other.oldest_;
    return *this;
}


InMemoryCacheStatistics &InMemoryCacheStatistics::operator/=(size_t n) {
    insertions_ /= n;
    evictions_ /= n;
    accesses_ /= n;
    youngest_ /= n;
    oldest_ /= n;
    return *this;
}

void InMemoryCacheStatistics::report(const char *title, std::ostream &out, const char *indent) const {

    std::string t(title);

    reportCount(out, (t + ", number of insertions").c_str(), insertions_, indent);
    reportCount(out, (t + ", number of evictions").c_str(), evictions_, indent);
    reportCount(out, (t + ", number of accesses").c_str(), accesses_, indent);
    reportTime(out, (t + ", oldest eviction").c_str(), oldest_, indent);
    reportTime(out, (t + ", youngest eviction").c_str(), youngest_, indent);



}

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

} // namespace pgen
