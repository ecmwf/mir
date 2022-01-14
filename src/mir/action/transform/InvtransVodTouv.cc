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


#include "mir/action/transform/InvtransVodTouv.h"

#include <ostream>
#include <vector>

#include "mir/data/MIRField.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"
#include "mir/util/Wind.h"


namespace mir {
namespace action {
namespace transform {


void InvtransVodTouv::print(std::ostream& out) const {
    out << "invtrans=<vod2uv>";
}


void InvtransVodTouv::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                              const param::MIRParametrisation& parametrisation) const {
    trace::Timer mainTimer("InvtransVodTouv::sh2grid");


    // set invtrans options
    atlas::util::Config config;
    config.set(atlas::option::global());

    ASSERT(0 < field.dimensions() && field.dimensions() % 2 == 0);
    size_t F = field.dimensions() / 2;


    // set input working area (avoid copies for one field pair only)
    MIRValuesVector input_vo;
    MIRValuesVector input_d;
    if (F > 1) {
        trace::Timer timer("InvtransVodTouv: interlacing spectra");

        auto T = size_t(trans.truncation());
        ASSERT(T > 0);

        auto N = repres::sh::SphericalHarmonics::number_of_complex_coefficients(T);
        ASSERT(N > 0);

        input_vo.resize(F * N * 2);
        input_d.resize(F * N * 2);

        for (size_t f = 0, which = 0; f < F; ++f, which += 2) {
            repres::sh::SphericalHarmonics::interlace_spectra(input_vo, field.values(which + 0), T, N, f, F);
            repres::sh::SphericalHarmonics::interlace_spectra(input_d, field.values(which + 1), T, N, f, F);

            field.direct(which + 0).clear();
            field.direct(which + 1).clear();
        }
    }
    else {
        input_vo.swap(field.direct(0));
        input_d.swap(field.direct(1));
    }
    ASSERT(input_vo.size() == input_d.size());


    // set output working area
    auto Ngp = size_t(trans.grid().size());
    ASSERT(Ngp > 0);

    MIRValuesVector output(F * Ngp * 2);


    // inverse transform
    {
        trace::Timer timer("InvtransVodTouv: invtrans");
        trans.invtrans(int(F), input_vo.data(), input_d.data(), output.data(), config);

        input_vo.clear();
        input_d.clear();
    }


    // set field values (again, avoid copies for one field pair only)
    {
        trace::Timer timer("InvtransVodTouv: copying grid-point values");

        // u/v are contiguous, they are saved in alternate order
        long id_u = 0;
        long id_v = 0;
        util::Wind::paramIds(parametrisation, id_u, id_v);

        MIRValuesVector values;
        auto here = output.cbegin();
        auto ngp  = MIRValuesVector::difference_type(Ngp);

        for (size_t f = 0, which = 0; f < F; ++f, which += 2, here += ngp) {
            values.assign(here, here + ngp);

            field.update(values, which);
            field.metadata(which, "paramId", id_u);
        }

        for (size_t f = 0, which = 1; f < F; ++f, which += 2, here += ngp) {
            values.assign(here, here + ngp);

            field.update(values, which);
            field.metadata(which, "paramId", id_v);
        }
    }
}


}  // namespace transform
}  // namespace action
}  // namespace mir
