// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once


#include <string>
#include <vector>

#include "mir/input/MIRInput.h"


namespace mir::input {


class GribAllFileInput : public MIRInput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GribAllFileInput(const std::string& path);

    // -- Destructor

    ~GribAllFileInput() override;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::string path_;
    std::vector<MIRInput*> inputs_;
    size_t count_;

    // -- Methods
    // None

    // -- Overridden methods

    const param::MIRParametrisation& parametrisation(size_t which) const override;
    data::MIRField field() const override;
    bool next() override;

    bool sameAs(const MIRInput&) const override;
    void print(std::ostream&) const override;

    grib_handle* gribHandle(size_t which = 0) const override;
    size_t dimensions() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::input
