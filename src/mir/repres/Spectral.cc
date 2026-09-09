// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/Spectral.h"

#include "mir/util/UnsupportedFunctionality.h"


namespace mir::repres {


void Spectral::reorder(MIRValuesVector&) const {
    // do nothing
}


struct UnsupportedSpectral : Spectral, util::UnsupportedFunctionality {
    explicit UnsupportedSpectral(const param::MIRParametrisation& p) :
        util::UnsupportedFunctionality("grid " + [this]() {
            std::ostringstream ss;
            this->print(ss);
            return ss.str();
        }() + " is currently unsupported") {}
};


struct RotatedSH final : UnsupportedSpectral {
    using UnsupportedSpectral::UnsupportedSpectral;
    void print(std::ostream& out) const override { out << "RotatedSH[]"; }
};


struct StretchedSH final : UnsupportedSpectral {
    using UnsupportedSpectral::UnsupportedSpectral;
    void print(std::ostream& out) const override { out << "StretchedSH[]"; }
};


struct StretchedRotatedSH final : UnsupportedSpectral {
    using UnsupportedSpectral::UnsupportedSpectral;
    void print(std::ostream& out) const override { out << "StretchedRotatedSH[]"; }
};


static const RepresentationBuilder<RotatedSH> ROTATED_SH("rotated_sh");
static const RepresentationBuilder<StretchedSH> STRETCHED_SH("stretched_sh");
static const RepresentationBuilder<StretchedRotatedSH> STRETCHED_ROTATED_SH("stretched_rotated_sh");


}  // namespace mir::repres
