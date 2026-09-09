// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/caching/legendre/LegendreLoader.h"


namespace mir::caching::legendre {

/// NoLoader turns off loading of coefficients but will not prevent the creation of the file
class NoLoader : public LegendreLoader {
public:
    NoLoader(const param::MIRParametrisation&, const eckit::PathName& path);

    ~NoLoader() override;

    static bool shared();

protected:
    void print(std::ostream&) const override;

private:
    const void* address() const override;
    size_t size() const override;
    bool inSharedMemory() const override;
};


}  // namespace mir::caching::legendre
