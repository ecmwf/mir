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


#include "mir/netcdf/GregorianCalendar.h"

#include <netcdf.h>

#include <algorithm>
#include <ostream>
#include <sstream>

#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/OutputAttribute.h"
#include "mir/netcdf/Value.h"
#include "mir/netcdf/Variable.h"


namespace mir {
namespace netcdf {


static long long offset = 0;


static eckit::DateTime reference(const std::string& /*units*/) {
    // Log::info() << "===== " << units << std::endl;
    return eckit::DateTime();
}


GregorianCalendar::GregorianCalendar(const Variable& variable) :
    reference_(reference(variable.getAttributeValue<std::string>("units"))),
    zero_(0),
    units_(variable.getAttributeValue<std::string>("units")),
    calendar_(variable.getAttributeValue<std::string>("calendar")) {

    auto reference_time = static_cast<long long>(eckit::Second(reference_.time()));
    auto reference_date = static_cast<long long>(reference_.date().julian());

    constexpr long long DAY_IN_SECONDS = 24 * 60 * 60;
    offset_                            = reference_date * DAY_IN_SECONDS + reference_time;
    if (offset == 0) {  // Not thread safe
        offset = offset_;
    }
    offset_ -= offset;
}


GregorianCalendar::~GregorianCalendar() = default;


void GregorianCalendar::print(std::ostream& out) const {
    out << "GregorianCalendar[reference=" << reference_ << ", calendar=" << calendar_ << ", offset=" << offset_ << "]";
}


template <class T>
void GregorianCalendar::_decode(std::vector<T>& v) const {
    for (size_t i = 0; i < v.size(); i++) {
        ASSERT(T(v[i] + offset_) - offset_ == v[i]);
        v[i] += offset_;
    }
}


void GregorianCalendar::decode(std::vector<double>& v) const {
    _decode(v);
}


void GregorianCalendar::decode(std::vector<float>& v) const {
    _decode(v);
}


void GregorianCalendar::decode(std::vector<long>& v) const {
    _decode(v);
}


void GregorianCalendar::decode(std::vector<short>& v) const {
    _decode(v);
}


void GregorianCalendar::decode(std::vector<unsigned char>& v) const {
    _decode(v);
}


void GregorianCalendar::decode(std::vector<long long>& v) const {
    _decode(v);
}


template <class T>
static T _encode(std::vector<T>& v) {
    ASSERT(v.size());
    T zero = *std::min_element(v.begin(), v.end());
    if (zero != v[0]) {
        for (size_t i = 0; i < v.size(); i++) {
            ASSERT(T(v[i] - zero) + zero == v[i]);
            v[i] -= zero;
        }
        return zero;
    }
    return 0;
}


void GregorianCalendar::encode(std::vector<double>& v) const {
    zero_ = static_cast<long long>(_encode(v));
}


void GregorianCalendar::encode(std::vector<float>& v) const {
    zero_ = static_cast<long long>(_encode(v));
}


void GregorianCalendar::encode(std::vector<long>& v) const {
    zero_ = _encode(v);
}


void GregorianCalendar::encode(std::vector<short>& v) const {
    zero_ = _encode(v);
}


void GregorianCalendar::encode(std::vector<unsigned char>& v) const {
    zero_ = _encode(v);
}


void GregorianCalendar::encode(std::vector<long long>& v) const {
    zero_ = _encode(v);
}


void GregorianCalendar::addAttributes(Variable& v) const {
    v.add(new OutputAttribute(v, "units", Value::newFromString("seconds since YYYY-MM-DD HH-MM-SS")));
    v.add(new OutputAttribute(v, "calendar", Value::newFromString(calendar_)));
}


void GregorianCalendar::updateAttributes(int nc, int varid, const std::string& path) {
    std::ostringstream s;
    eckit::DateTime dt = reference_ + eckit::Second(zero_);
    s << "seconds since " << dt.date() << " " << dt.time();
    std::string value = s.str();
    NC_CALL(nc_put_att_text(nc, varid, "units", value.size(), value.c_str()), path);
}


static const CodecBuilder<GregorianCalendar> builder1("gregorian");
static const CodecBuilder<GregorianCalendar> builder2("standard");


}  // namespace netcdf
}  // namespace mir
