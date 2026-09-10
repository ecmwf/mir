// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "eckit/filesystem/PathName.h"

#include "mir/lsm/Mask.h"


namespace mir::lsm {


class GribFileMask : public Mask {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GribFileMask(const eckit::PathName&, const param::MIRParametrisation&, const repres::Representation&,
                 const std::string& which);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    bool active() const override;
    bool cacheable() const override = 0;
    void hash(eckit::MD5&) const override;
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    const eckit::PathName path_;


private:
    // -- Members

    std::vector<bool> mask_;

    // -- Methods
    // None

    // -- Overridden methods

    const std::vector<bool>& mask() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::lsm
