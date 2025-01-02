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
