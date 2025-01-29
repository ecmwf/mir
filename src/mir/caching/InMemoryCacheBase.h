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

#include "mir/caching/InMemoryCacheUsage.h"


namespace mir::caching {


class InMemoryCacheBase {

public:  // methods
    InMemoryCacheBase();

    InMemoryCacheBase(const InMemoryCacheBase&) = delete;
    InMemoryCacheBase(InMemoryCacheBase&&)      = delete;

    virtual ~InMemoryCacheBase();

    void operator=(const InMemoryCacheBase&) = delete;
    void operator=(InMemoryCacheBase&&)      = delete;

    static InMemoryCacheUsage totalFootprint();

    virtual const std::string& name() const = 0;

protected:
    void checkTotalFootprint();

private:
    virtual InMemoryCacheUsage footprint() const                                    = 0;
    virtual InMemoryCacheUsage capacity() const                                     = 0;
    virtual InMemoryCacheUsage purge(const InMemoryCacheUsage&, bool force = false) = 0;
};


}  // namespace mir::caching
