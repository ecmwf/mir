// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
