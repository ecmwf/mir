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
