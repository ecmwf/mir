// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
