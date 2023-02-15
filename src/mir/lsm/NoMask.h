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

#include "mir/lsm/Mask.h"


namespace mir::lsm {


class NoMask : public Mask {

    bool active() const override;

    bool cacheable() const override;

    void hash(eckit::MD5&) const override;

    const std::vector<bool>& mask() const override;

    void print(std::ostream&) const override;

    std::string cacheName() const override;
};


}  // namespace mir::lsm
