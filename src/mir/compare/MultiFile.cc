/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date   Jul 2016


#include "mir/compare/MultiFile.h"

#include "eckit/serialisation/Stream.h"


namespace mir {
namespace compare {

MultiFile::MultiFile(const std::string& name, const std::string& from):
    name_(name),
    from_(from) {
}


bool MultiFile::operator<(const MultiFile& other) const {
    if (name_ == other.name_) {
        return from_ < other.from_;
    }
    return name_ < other.name_;
}


MultiFile::MultiFile(eckit::Stream& s) {
    s >> name_;
    s >> from_;
    size_t n;
    s >> n;
    for (size_t i = 0; i < n; i++) {
        std::string tmp;
        s >> tmp;
        paths_.push_back(tmp);
    }
}


void MultiFile::add(const std::string& path) {
    paths_.push_back(path);
}


void MultiFile::encode(eckit::Stream& s) const {
    s << name_;
    s << from_;
    s << paths_.size();
    for (size_t i = 0; i < paths_.size(); i++) {
        s << paths_[i];
    }
}


void MultiFile::print(std::ostream& out) const {
    out << name_ << " (" << from_ << ")";
}


const std::vector<std::string>& MultiFile::paths() const {
    return paths_;
}


}  // namespace compare
}  // namespace mir

