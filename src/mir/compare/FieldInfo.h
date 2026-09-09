// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
