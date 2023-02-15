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

#include <algorithm>  // FIXME: temporary, for pgen/src/pgen/data/WeatherParameter.cc using std::sort
#include <iosfwd>

#include "mir/input/MIRInput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Types.h"


namespace mir::input {


class GeoPointsFileInput : public MIRInput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GeoPointsFileInput(const std::string& path, int which = -1);

    // -- Destructor

    ~GeoPointsFileInput() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    const std::vector<double>& latitudes() const;
    const std::vector<double>& longitudes() const;
    const MIRValuesVector& values() const;

    size_t footprint() const;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::string path_;
    param::SimpleParametrisation fieldParametrisation_;
    size_t dimensions_;
    size_t next_;
    size_t footprint_;
    double missingValue_;

    std::vector<double> latitudes_;
    std::vector<double> longitudes_;
    MIRValuesVector values_;

    int which_;
    bool hasMissing_;

    // -- Methods

    bool resetMissingValue(double& missingValue);

    size_t readText(std::ifstream&);
    size_t readBinary(std::ifstream&);

    // -- Overridden methods

    // From MIRInput
    void print(std::ostream&) const override;
    bool sameAs(const MIRInput&) const override;

    const param::MIRParametrisation& parametrisation(size_t which) const override;
    data::MIRField field() const override;

    bool next() override;
    size_t dimensions() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::input
