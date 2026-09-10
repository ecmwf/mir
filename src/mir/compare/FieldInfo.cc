// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/compare/FieldInfo.h"

#include <ostream>

#include "eckit/log/JSON.h"


namespace mir::compare {


FieldInfo::FieldInfo(const std::string& path, off_t offset, size_t length) :
    path_(path), offset_(offset), length_(length) {}

void FieldInfo::print(std::ostream& out) const {
    out << "FieldInfo[path=" << path_ << ",offset=" << offset_ << ",length=" << length_ << "]";
}


void FieldInfo::json(eckit::JSON& json) const {
    json << "path" << path_ << "offset" << offset_ << "length" << length_;
}


off_t FieldInfo::offset() const {
    return offset_;
}


size_t FieldInfo::length() const {
    return length_;
}


const std::string& FieldInfo::path() const {
    return path_;
}


}  // namespace mir::compare
