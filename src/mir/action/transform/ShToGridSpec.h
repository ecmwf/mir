// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>

#include "mir/action/transform/ShToGridded.h"
#include "mir/param/GridSpecParametrisation.h"


namespace mir::action::transform {


template <class Invtrans>
class ShToGridSpec : public ShToGridded, Invtrans {
public:
    explicit ShToGridSpec(const param::MIRParametrisation&);

private:
    std::unique_ptr<param::GridSpecParametrisation> param_;

    bool sameAs(const Action&) const override;
    void print(std::ostream&) const override;
    const char* name() const override;
    const repres::Representation* outputRepresentation() const override;

    void sh2grid(data::MIRField& field, const atlas_trans_t& trans, const param::MIRParametrisation&) const override;
};


}  // namespace mir::action::transform
