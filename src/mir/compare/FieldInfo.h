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

#include <iosfwd>
#include <string>


namespace eckit {
class JSON;
}


namespace mir::compare {


class FieldInfo {
public:
    FieldInfo(const std::string& path, off_t offset, size_t length);

    off_t offset() const;

    size_t length() const;

    const std::string& path() const;

private:
    std::string path_;
    off_t offset_;
    size_t length_;

    void print(std::ostream&) const;
    void json(eckit::JSON&) const;

    friend std::ostream& operator<<(std::ostream& s, const FieldInfo& x) {
        x.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const FieldInfo& x) {
        x.json(s);
        return s;
    }
};


}  // namespace mir::compare
