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

#include "eckit/io/Buffer.h"

#include "mir/caching/legendre/LegendreLoader.h"


namespace mir::caching::legendre {


class FileLoader : public LegendreLoader {
public:
    FileLoader(const param::MIRParametrisation&, const eckit::PathName&);

    ~FileLoader() override;

    static bool shared();

protected:
    void print(std::ostream&) const override;

private:
    const void* address() const override;
    size_t size() const override;
    bool inSharedMemory() const override;

    eckit::Buffer buffer_;
};


}  // namespace mir::caching::legendre
