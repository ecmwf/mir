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
    trace::Timer timer("InvtransVodTouv::sh2grid", Log::debug());

    size_t number_of_fields = field.dimensions();
    ASSERT(number_of_fields == 2);

    const int number_of_grid_points = int(trans.grid().size());
    ASSERT(number_of_grid_points > 0);


    // set invtrans options
    atlas::util::Config config;
    config.set(atlas::option::global());


    // get vo/d
    const MIRValuesVector& field_vo = field.values(0);
    const MIRValuesVector& field_d  = field.values(1);

    if (field_vo.size() != field_d.size()) {
        Log::error() << "ShVodToUV: input fields have different truncation: " << field_vo.size() << "/"
                     << field_d.size() << std::endl;
        ASSERT(field_vo.size() == field_d.size());
    }


    // do inverse transform and set gridded values
    MIRValuesVector output(size_t(number_of_grid_points) * 2);
    trans.invtrans(1, field_vo.data(), field_d.data(), output.data(), config);

    MIRValuesVector output_field;


    // configure paramIds for u/v
    long id_u = 0;
    long id_v = 0;
    util::Wind::paramIds(parametrisation, id_u, id_v);


    // u/v are contiguous, they are saved as separate vectors
    auto here = output.cbegin();
    output_field.assign(here, here + number_of_grid_points);
    field.update(output_field, 0);
    field.metadata(0, "paramId", id_u);

    here += number_of_grid_points;
    output_field.assign(here, here + number_of_grid_points);
    field.update(output_field, 1);
    field.metadata(1, "paramId", id_v);
}

}  // namespace transform
}  // namespace action
}  // namespace mir
