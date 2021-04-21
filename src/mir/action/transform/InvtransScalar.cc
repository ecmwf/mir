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


#include "mir/action/transform/InvtransScalar.h"

#include <ostream>
#include <vector>

#include "mir/data/MIRField.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir {
namespace action {
namespace transform {


void InvtransScalar::print(std::ostream& out) const {
    out << "invtrans=<scalar>";
}


void InvtransScalar::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                             const param::MIRParametrisation&) const {
    auto& log = Log::debug();
    trace::Timer mainTimer("InvtransScalar::sh2grid", log);


    // set invtrans options
    atlas::util::Config config;
    config.set(atlas::option::global());

    size_t F = field.dimensions();
    ASSERT(F > 0);


    // set input working area (avoid copies for one field only)
    MIRValuesVector input;
    if (F > 1) {
        trace::Timer timer("InvtransScalar: interlacing spectra", log);

        auto T = size_t(trans.truncation());
        ASSERT(T > 0);

        auto N = repres::sh::SphericalHarmonics::number_of_complex_coefficients(T);
        ASSERT(N > 0);

        input.resize(F * N * 2);

        for (size_t f = 0; f < F; ++f) {
            repres::sh::SphericalHarmonics::interlace_spectra(input, field.values(f), T, N, f, F);
            field.direct(f).clear();
        }
    }
    else {
        input.swap(field.direct(0));
    }


    // set output working area
    auto Ngp = size_t(trans.grid().size());
    ASSERT(Ngp > 0);

    MIRValuesVector output(F * Ngp);


    // inverse transform
    {
        trace::Timer timer("InvtransScalar: invtrans", log);
        trans.invtrans(int(F), input.data(), output.data(), config);

        input.clear();
    }


    // set field values (again, avoid copies for one field only)
    if (F > 1) {
        trace::Timer timer("InvtransScalar: copying grid-point values", log);

        MIRValuesVector values;
        auto here = output.cbegin();

        for (size_t f = 0; f < F; ++f) {
            values.assign(here, here + Ngp);

            field.update(values, f);
            here += Ngp;
        }
    }
    else {
        field.update(output, 0);
    }
}


}  // namespace transform
}  // namespace action
}  // namespace mir
