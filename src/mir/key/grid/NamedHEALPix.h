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


#include "mir/key/grid/NamedGrid.h"


namespace mir::key::grid {


class NamedHEALPix : public NamedGrid {
public:
    // -- Constructors

    NamedHEALPix(const std::string& name, size_t Nside, bool nested);

protected:
    // -- Overridden methods

    void print(std::ostream&) const override;
    size_t gaussianNumber() const override;
    const repres::Representation* representation() const override;
    const repres::Representation* representation(const util::Rotation&) const override;

private:
    // -- Members

    const size_t Nside_;
    const bool nested_;
};


}  // namespace mir::key::grid
