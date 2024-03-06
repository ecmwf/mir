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

#include <cstddef>
#include <iosfwd>


namespace eckit {
class Stream;
}


namespace mir::caching {


class InMemoryCacheUsage {
public:
    explicit InMemoryCacheUsage();
    explicit InMemoryCacheUsage(const std::string&);

    explicit InMemoryCacheUsage(size_t memory, size_t shared);
    explicit InMemoryCacheUsage(eckit::Stream&);

    InMemoryCacheUsage& operator+=(const InMemoryCacheUsage&);
    InMemoryCacheUsage& operator/=(size_t);

    InMemoryCacheUsage operator+(const InMemoryCacheUsage&) const;
    InMemoryCacheUsage operator-(const InMemoryCacheUsage&) const;
    InMemoryCacheUsage operator/(size_t) const;


    bool operator>(const InMemoryCacheUsage&) const;
    bool operator!() const;
    operator bool() const;
    operator std::string() const;

    size_t memory() const;
    size_t shared() const;

private:
    size_t memory_;
    size_t shared_;

    void encode(eckit::Stream&) const;
    void decode(eckit::Stream&);

    friend eckit::Stream& operator<<(eckit::Stream& s, const InMemoryCacheUsage& x) {
        x.encode(s);
        return s;
    }

    friend eckit::Stream& operator>>(eckit::Stream& s, InMemoryCacheUsage& x) {
        x.decode(s);
        return s;
    }


    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& s, const InMemoryCacheUsage& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::caching
