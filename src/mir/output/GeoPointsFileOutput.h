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

#include <memory>

#include "mir/output/GeoPointsOutput.h"


namespace mir::output {


class GeoPointsFileOutput : public GeoPointsOutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GeoPointsFileOutput(const std::string& path, bool binary);

    // -- Destructor

    ~GeoPointsFileOutput() override;

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

    std::string path_;
    mutable std::unique_ptr<eckit::DataHandle> handle_;
    bool binary_;

    // -- Methods
    // None

    // -- Overridden methods

    // From GeoPointsOutput
    eckit::DataHandle& dataHandle() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    // From MIROutput
    void print(std::ostream&) const override;
    bool sameAs(const MIROutput&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::output
