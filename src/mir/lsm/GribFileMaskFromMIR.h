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

#include "mir/lsm/GribFileMask.h"


namespace mir::lsm {


class GribFileMaskFromMIR : public GribFileMask {
public:
    GribFileMaskFromMIR(const std::string& name, const eckit::PathName&, const param::MIRParametrisation&,
                        const repres::Representation&, const std::string& which);

private:
    std::string name_;

    bool cacheable() const override;
    std::string cacheName() const override;
};


}  // namespace mir::lsm
