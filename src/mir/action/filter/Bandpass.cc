// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/filter/Bandpass.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/util/Exceptions.h"


namespace mir::action::filter {


Bandpass::Bandpass(const param::MIRParametrisation& param) : Action(param) {
    ASSERT(param.get("truncation", truncation_));

    std::vector<long> b;
    ASSERT(param.userParametrisation().get("bandpass", b) && b.size() == 4);

    minM_ = static_cast<size_t>(b[0]);
    maxM_ = static_cast<size_t>(b[1]);
    minN_ = static_cast<size_t>(b[2]);
    maxN_ = static_cast<size_t>(b[3]);
    ASSERT(minM_ <= maxM_ && maxM_ <= truncation_);
    ASSERT(minN_ <= maxN_ && maxN_ <= truncation_);
}


bool Bandpass::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Bandpass*>(&other);
    return (o != nullptr) && minM_ == o->minM_ && maxM_ == o->maxM_ && minN_ == o->minN_ && maxN_ == o->maxN_;
}


void Bandpass::print(std::ostream& out) const {
    out << "Bandpass[M=" << minM_ << "/" << maxM_ << ",N=" << minN_ << "/" << maxN_ << "]";
}


void Bandpass::execute(context::Context& ctx) const {
    const auto N = repres::sh::SphericalHarmonics::number_of_complex_coefficients(truncation_);

    auto& field = ctx.field();
    for (size_t i = 0; i < field.dimensions(); ++i) {
        const auto& in = field.values(i);
        ASSERT(in.size() == N * 2);

        MIRValuesVector out(in.size(), 0.);

        for (size_t m = 0, j = 0; m <= truncation_; m++) {
            for (size_t n = m; n <= truncation_; n++, j += 2) {
                if (minM_ <= m && m <= maxM_ && minN_ <= n && n <= maxN_) {
                    out[j]     = in[j];
                    out[j + 1] = in[j + 1];
                }
            }
        }

        field.update(out, i);
    }
}


const char* Bandpass::name() const {
    return "Bandpass";
}


static const ActionBuilder<Bandpass> __action("filter.sh-bandpass");


}  // namespace mir::action::filter
