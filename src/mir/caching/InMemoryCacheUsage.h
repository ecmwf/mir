/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   Oct 2016

#ifndef mir_caching_InMemoryCacheUsage_H
#define mir_caching_InMemoryCacheUsage_H

#include <iosfwd>

namespace eckit {
class Stream;
}

namespace mir {

//----------------------------------------------------------------------------------------------------------------------
class InMemoryCacheUsage {
public:

    InMemoryCacheUsage(const std::string&);

    InMemoryCacheUsage(unsigned long long memory = 0, unsigned long long shared = 0);
    InMemoryCacheUsage(unsigned long long size, bool inSharedMemory);
    InMemoryCacheUsage(eckit::Stream &) ;

    InMemoryCacheUsage &operator+=(const InMemoryCacheUsage &rhs) ;
    InMemoryCacheUsage &operator/=(size_t) ;

    bool operator>(const InMemoryCacheUsage& other) const;
    bool operator !() const;
    operator bool() const;
    operator std::string() const;

    unsigned long long memory() const;
    unsigned long long shared() const;

private:

    unsigned long long memory_;
    unsigned long long shared_;

    void encode(eckit::Stream &) const;
    void decode(eckit::Stream &);

    friend eckit::Stream &operator<<(eckit::Stream &s, const InMemoryCacheUsage &x) {
        x.encode(s);
        return s;
    }

    friend eckit::Stream &operator>>(eckit::Stream &s,  InMemoryCacheUsage &x) {
        x.decode(s);
        return s;
    }


    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& s, const InMemoryCacheUsage& p) {
        p.print(s);
        return s;
    }
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir


#endif
