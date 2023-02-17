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

#include "eckit/filesystem/PathName.h"

#include "mir/util/Atlas.h"
#include "mir/util/Types.h"


namespace atlas::trans {
class LegendreCache;
}  // namespace atlas::trans

namespace mir::param {
class MIRParametrisation;
}  // namespace mir::param


namespace mir::caching::legendre {


class LegendreLoader {

public:
    LegendreLoader(const param::MIRParametrisation&, const eckit::PathName&);
    virtual ~LegendreLoader();

    LegendreLoader(const LegendreLoader&)            = delete;
    LegendreLoader& operator=(const LegendreLoader&) = delete;

    virtual const void* address() const = 0;
    virtual size_t size() const         = 0;
    virtual bool inSharedMemory() const = 0;

    atlas::trans::LegendreCache transCache();

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
    virtual bool shared() const                                                                 = 0;

    LegendreLoaderFactory(const LegendreLoaderFactory&)            = delete;
    LegendreLoaderFactory& operator=(const LegendreLoaderFactory&) = delete;

protected:
    LegendreLoaderFactory(const std::string&);
    virtual ~LegendreLoaderFactory();

public:
    static LegendreLoader* build(const param::MIRParametrisation&, const eckit::PathName&);
    static void list(std::ostream&);
    static bool inSharedMemory(const param::MIRParametrisation&);
};


template <class T>
class LegendreLoaderBuilder : public LegendreLoaderFactory {

    LegendreLoader* make(const param::MIRParametrisation& param, const eckit::PathName& path) override {
        return new T(param, path);
    }

    bool shared() const override { return T::shared(); }

public:
    LegendreLoaderBuilder(const std::string& name) : LegendreLoaderFactory(name) {}
};


}  // namespace mir::caching::legendre
