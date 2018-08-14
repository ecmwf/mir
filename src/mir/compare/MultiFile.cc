/*
 * (C) Copyright 1996- ECMWF.
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
#include "eckit/filesystem/PathName.h"
#include "eckit/io/MultiHandle.h"
#include "eckit/memory/ScopedPtr.h"

namespace mir {
namespace compare {

MultiFile::MultiFile(const std::string& name, const std::string& from):
    name_(name),
    from_(from)
{
}


MultiFile::MultiFile(const std::string& path):
    name_(path),
    from_(path)
{
    add(path);
}

bool MultiFile::operator<(const MultiFile& other) const  {
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

const std::string& MultiFile::name() const {
    return name_;
}

const std::string& MultiFile::from() const {
    return from_;
}


void MultiFile::save() const {
    eckit::PathName out(name_ + "." + from_);
    eckit::MultiHandle mh;
    for (size_t i = 0; i < paths_.size(); i++) {
        eckit::PathName p(paths_[i]);
        mh += p.fileHandle();
    }
    eckit::ScopedPtr<eckit::DataHandle> h(out.fileHandle());
    eckit::Log::info() << "Save " << mh << " into " << (*h) << std::endl;
    mh.saveInto(*h);
}

void MultiFile::save(const std::string& path, off_t offset, size_t length, size_t n) const {
    std::ostringstream oss;
    oss << name_ << '.' << from_ << '.' << n;

    eckit::PathName in(path);

    eckit::PathName out(oss.str());

    eckit::ScopedPtr<eckit::DataHandle> ih(in.partHandle(offset, length));
    eckit::ScopedPtr<eckit::DataHandle> oh(out.fileHandle());

    eckit::Log::info() << "Save " << (*ih) << " into " << (*oh) << std::endl;
    ih->saveInto(*oh);
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

void MultiFile::print(std::ostream& out)  const {
    if (name_ == from_) {
        out << name_;
    }
    else {
        out << name_ << " (" << from_ << ")";
    }
}

void MultiFile::whiteListEntries(std::ostream& out) const {
    out << "# " << from_ << std::endl;
    std::string s(name_);
    for (size_t i = 3; i < 3 + 16; i++) {
        if (i >= s.size()) {
            break;
        }
        if (s[i] == '_') {
            break;
        }
        s[i] = '.';
    }
    out << s;
}


eckit::Length MultiFile::length() const {
    if (length_ == eckit::Length(0)) {
        for (size_t i = 0; i < paths_.size(); i++) {
            eckit::PathName p(paths_[i]);
            try {
                length_ += p.size();
            }
            catch (std::exception& e) {
                eckit::Log::error() << e.what() << std::endl;
            }
        }
    }
    return length_;
}

const std::vector<std::string>& MultiFile::paths() const {
    return paths_;
}

} // namespace compare
} // namespace mir
