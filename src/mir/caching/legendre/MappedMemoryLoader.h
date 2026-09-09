// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/caching/legendre/LegendreLoader.h"


namespace mir::caching::legendre {


class MappedMemoryLoader : public LegendreLoader {
public:
    MappedMemoryLoader(const param::MIRParametrisation&, const eckit::PathName& path);

    ~MappedMemoryLoader() override;

    static bool shared();

protected:
    void print(std::ostream&) const override;

private:
    const void* address() const override;
    size_t size() const override;
    bool inSharedMemory() const override;

private:
    int fd_;
    void* address_;
    size_t size_;
};


}  // namespace mir::caching::legendre
