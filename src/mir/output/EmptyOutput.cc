// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/output/EmptyOutput.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"


namespace mir::output {


EmptyOutput::EmptyOutput() = default;


EmptyOutput::EmptyOutput(const std::string& /*unused*/) {}


size_t EmptyOutput::save(const param::MIRParametrisation& /*unused*/, context::Context& /*unused*/) {
    return 0;
}


bool EmptyOutput::sameAs(const MIROutput& other) const {
    const auto* o = dynamic_cast<const EmptyOutput*>(&other);
    return (o != nullptr);
}


bool EmptyOutput::sameParametrisation(const param::MIRParametrisation& /*unused*/,
                                      const param::MIRParametrisation& /*unused*/) const {
    return false;
}


bool EmptyOutput::printParametrisation(std::ostream& out, const param::MIRParametrisation& param) const {
    bool ok = false;

    long bits;
    if (param.userParametrisation().get("accuracy", bits)) {
        out << "accuracy=" << bits;
        ok = true;
    }

    std::string packing;
    if (param.userParametrisation().get("packing", packing)) {
        if (ok) {
            out << ",";
        }
        out << "packing=" << packing;
        ok = true;
    }

    long edition;
    if (param.userParametrisation().get("edition", edition)) {
        if (ok) {
            out << ",";
        }
        out << "edition=" << edition;
        ok = true;
    }

    return ok;
}


void EmptyOutput::print(std::ostream& out) const {
    out << "EmptyOutput[]";
}


static const MIROutputBuilder<EmptyOutput> output("empty");


}  // namespace mir::output
