/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#include "mir/action/transform/InvtransVodTouv.h"

#include <iostream>
#include <vector>

#include "eckit/exception/Exceptions.h"

#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {
namespace transform {


void InvtransVodTouv::print(std::ostream& out) const {
    out << "invtrans=<vod2uv>";
}


void InvtransVodTouv::sh2grid(data::MIRField& field,
                              const ShToGridded::atlas_trans_t& trans,
                              const param::MIRParametrisation& parametrisation) const {
    eckit::Timer timer("InvtransVodTouv::sh2grid", eckit::Log::debug<LibMir>());

    size_t number_of_fields = field.dimensions();
    ASSERT(number_of_fields == 2);

    const int number_of_grid_points = int(trans.grid().size());
    ASSERT(number_of_grid_points > 0);

    // set invtrans options
    atlas::util::Config config;
    config.set(atlas::option::global());



    // do inverse transform and set gridded values
    MIRValuesVector output(size_t(number_of_grid_points) * 2);
    trans.invtrans(1, field.values(0).data(), field.values(1).data(),  output.data(), config);

    // set u/v field values and paramId (u/v are contiguous, they are saved as separate vectors)
    MIRValuesVector output_field;


    size_t id_vo = 0;
    ASSERT(parametrisation.fieldParametrisation().get("paramId", id_vo));
    ASSERT(id_vo > 0);

    // Assumes the same table for the defaults

    size_t table = id_vo / 1000;

    eckit::Log::debug<LibMir>() << "U/V table = " << table << std::endl;

    size_t id_u = 131 + table * 1000;
    size_t id_v = 132 + table * 1000;


    // User input if given
    parametrisation.userParametrisation().get("paramId.u", id_u);
    parametrisation.userParametrisation().get("paramId.v", id_v);

    auto here = output.cbegin();
    output_field.assign(here, here + number_of_grid_points);
    field.update(output_field, 0);
    field.metadata(0, "paramId", id_u);

    here += number_of_grid_points;
    output_field.assign(here, here + number_of_grid_points);
    field.update(output_field, 1);
    field.metadata(1, "paramId", id_v);

     eckit::Log::debug<LibMir>() << "Using paramId U/V = " << id_u << " / " << id_v << std::endl;
}


}  // namespace transform
}  // namespace action
}  // namespace mir

