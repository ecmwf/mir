// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/input/ArtificialInput.h"


namespace mir::input {


class GridSpecInput final : public ArtificialInput {
public:
    explicit GridSpecInput();
    explicit GridSpecInput(const std::string& gridspec, bool gridded = true);

private:
    data::MIRField field() const override;
    void setAuxiliaryInformation(const util::ValueMap&) override;

    size_t size_;
};


}  // namespace mir::input
