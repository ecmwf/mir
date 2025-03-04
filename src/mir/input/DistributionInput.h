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

#include "mir/input/ArtificialInput.h"


namespace mir::input {


class DistributionInput final : public ArtificialInput {
public:
    DistributionInput() = default;

private:
    data::MIRField field() const override;
};


}  // namespace mir::input
