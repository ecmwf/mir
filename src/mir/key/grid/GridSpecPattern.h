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

#include "mir/key/grid/GridPattern.h"


namespace mir::key::grid {


class GridSpecPattern : public GridPattern {
public:
    explicit GridSpecPattern(const std::string& pattern);

private:
    void print(std::ostream&) const override;
    const Grid* make(const std::string& name) const override;
    std::string canonical(const std::string& name, const param::MIRParametrisation&) const override;
};


}  // namespace mir::key::grid
