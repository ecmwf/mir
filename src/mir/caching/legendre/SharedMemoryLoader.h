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

#include "mir/caching/legendre/LegendreLoader.h"


namespace mir::caching::legendre {


class SharedMemoryLoader : public LegendreLoader {
public:
    SharedMemoryLoader(const param::MIRParametrisation&, const eckit::PathName&);

    ~SharedMemoryLoader() override;

    static void loadSharedMemory(const eckit::PathName&);
    static void unloadSharedMemory(const eckit::PathName&);

    static bool shared();

protected:
    void print(std::ostream&) const override;

private:
    const void* address() const override;
    size_t size() const override;
    bool inSharedMemory() const override;

    void* address_;
    size_t size_;
    bool unload_;
};


}  // namespace mir::caching::legendre
