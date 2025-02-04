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

#include <vector>

#include "mir/input/MIRInput.h"
#include "mir/param/SimpleParametrisation.h"


namespace eckit {
class PathName;
}


namespace mir::input {


class GriddefInput final : public MIRInput {
public:
    GriddefInput(const eckit::PathName&);

    static void load(const eckit::PathName&, std::vector<double>& latitudes, std::vector<double>& longitudes);

private:
    std::vector<double> latitudes_;
    std::vector<double> longitudes_;
    size_t calls_;

    param::SimpleParametrisation parametrisation_;

    bool next() override;
    size_t dimensions() const override;
    const param::MIRParametrisation& parametrisation(size_t which = 0) const override;
    data::MIRField field() const override;
    bool sameAs(const MIRInput&) const override;
    void print(std::ostream&) const override;

    friend std::ostream& operator<<(std::ostream& s, const GriddefInput& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::input
