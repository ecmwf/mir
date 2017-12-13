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
/// @author Pedro Maciel
/// @author Tiago Quintino
///
/// @date Apr 2015

#ifndef mir_caching_LegendreLoader_H
#define mir_caching_LegendreLoader_H

#include <iosfwd>

#include "eckit/filesystem/PathName.h"
#include "eckit/memory/NonCopyable.h"

namespace mir {

namespace param {
class MIRParametrisation;
}

namespace caching {
namespace legendre {

class LegendreLoader : public eckit::NonCopyable {

public:
    LegendreLoader(const param::MIRParametrisation& parametrisation, const eckit::PathName&);

    virtual ~LegendreLoader();

    virtual const void* address() const = 0;
    virtual size_t size() const = 0;
    virtual bool inSharedMemory() const = 0;

protected:
    const param::MIRParametrisation& parametrisation_;
    eckit::PathName path_;

    virtual void print(std::ostream&) const = 0;

private:
    friend std::ostream& operator<<(std::ostream& s, const LegendreLoader& p) {
        p.print(s);
        return s;
    }
};

class LegendreLoaderFactory {
    std::string name_;
    virtual LegendreLoader* make(const param::MIRParametrisation&, const eckit::PathName& path) = 0;
    virtual bool shared() const = 0;

protected:
    LegendreLoaderFactory(const std::string&);
    virtual ~LegendreLoaderFactory();

public:
    static LegendreLoader* build(const param::MIRParametrisation&, const eckit::PathName& path);
    static void list(std::ostream&);
    static bool inSharedMemory(const param::MIRParametrisation&);
};

template <class T>
class LegendreLoaderBuilder : public LegendreLoaderFactory {

    virtual LegendreLoader* make(const param::MIRParametrisation& param, const eckit::PathName& path) {
        return new T(param, path);
    }

    virtual bool shared() const {
        return T::shared();
    }

public:
    LegendreLoaderBuilder(const std::string& name) : LegendreLoaderFactory(name) {}
};

}  // namespace legendre
}  // namespace caching
}  // namespace mir

#endif
