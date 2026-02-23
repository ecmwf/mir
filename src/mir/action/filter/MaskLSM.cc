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


#include "mir/action/filter/MaskLSM.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/lsm/Mask.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::action::filter {


void MaskLSM::Value::list(std::ostream& out) {
    out << "0, 1";
}


bool MaskLSM::Value::get(const param::MIRParametrisation& param, const std::string& key) {
    int value = 0;
    param.get(key, value);

    if (value == 0 || value == 1) {
        return value != 0;
    }

    list(Log::error() << "MaskLSM::Value: invalid " << key << ", choices are: ");
    throw exception::UserError("MaskLSM::Value: invalid " + key);
}


MaskLSM::MaskLSM(const param::MIRParametrisation& param, const std::string& key) :
    Action(param), value_(Value::get(param, key)) {}


void MaskLSM::print(std::ostream& out) const {
    out << name() << "[value=" << value() << "]";
}


void MaskLSM::execute(context::Context& ctx) const {
    auto& field     = ctx.field();
    bool hasMissing = field.hasMissing();

    repres::RepresentationHandle rep(field.representation());
    ASSERT(rep);

    const auto& values_mask = mask(*rep);

    for (size_t d = 0; d < field.dimensions(); ++d) {
        auto& values = field.direct(d);
        ASSERT(values.size() == values_mask.size());

        for (size_t i = 0; i < values.size(); ++i) {
            if (values_mask[i] == value_) {
                values[i]  = field.missingValue();
                hasMissing = true;
            }
        }
    }

    field.hasMissing(hasMissing);
}


struct MaskInputLSM final : MaskLSM {
    explicit MaskInputLSM(const param::MIRParametrisation& param) : MaskLSM(param, "mask-input-lsm-value") {}

    const char* name() const final { return "MaskInputLSM"; }

    bool sameAs(const Action& other) const final {
        const auto* o = dynamic_cast<const MaskInputLSM*>(&other);
        return (o != nullptr) && (value() == o->value());
    }

    const std::vector<bool>& mask(const repres::Representation& rep) const final {
        return lsm::Mask::lookupInput(parametrisation(), rep).mask();
    }
};


struct MaskOutputLSM final : MaskLSM {
    explicit MaskOutputLSM(const param::MIRParametrisation& param) : MaskLSM(param, "mask-output-lsm-value") {}

    const char* name() const final { return "MaskOutputLSM"; }

    bool sameAs(const Action& other) const final {
        const auto* o = dynamic_cast<const MaskOutputLSM*>(&other);
        return (o != nullptr) && (value() == o->value());
    }

    const std::vector<bool>& mask(const repres::Representation& rep) const final {
        return lsm::Mask::lookupOutput(parametrisation(), rep).mask();
    }
};


static const ActionBuilder<MaskInputLSM> ACTION1("filter.mask-input-lsm");
static const ActionBuilder<MaskOutputLSM> ACTION2("filter.mask-output-lsm");


}  // namespace mir::action::filter
