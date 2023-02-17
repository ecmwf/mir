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


#pragma once

#include <string>
#include <vector>

#include "eckit/io/Length.h"


namespace eckit {
class Stream;
}


namespace mir::compare {


class MultiFile {
public:
    MultiFile(const std::string& path);
    MultiFile(const std::string& name, const std::string& from);
    MultiFile(eckit::Stream& s);

    bool operator<(const MultiFile&) const;

    void add(const std::string& path);

    const std::vector<std::string>& paths() const;

    void encode(eckit::Stream& s) const;

    void print(std::ostream&) const;

    void save() const;

    const std::string& from() const;
    const std::string& name() const;

    eckit::Length length() const;
    void whiteListEntries(std::ostream&) const;

    void save(const std::string& path, off_t offset, size_t length, size_t n) const;


private:
    std::string name_;
    std::string from_;
    std::vector<std::string> paths_;

    mutable eckit::Length length_;

private:
    friend eckit::Stream& operator<<(eckit::Stream& s, const MultiFile& x) {
        x.encode(s);
        return s;
    }

    friend std::ostream& operator<<(std::ostream& out, const MultiFile& x) {
        x.print(out);
        return out;
    }
};


}  // namespace mir::compare
