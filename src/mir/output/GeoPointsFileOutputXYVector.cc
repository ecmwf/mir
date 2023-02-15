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


#include "mir/output/GeoPointsFileOutputXYVector.h"

#include <limits>
#include <memory>

#include "eckit/io/HandleBuf.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::output {


// See https://software.ecmwf.int/wiki/display/METV/Geopoints


GeoPointsFileOutputXYVector::GeoPointsFileOutputXYVector(const std::string& path, bool binary) :
    GeoPointsFileOutput(path, binary) {}


size_t GeoPointsFileOutputXYVector::save(const param::MIRParametrisation& param, context::Context& ctx) {
    ASSERT(once());
    ASSERT(!binary_);

    const auto& field = ctx.field();
    auto mv           = field.hasMissing() ? field.missingValue() : std::numeric_limits<double>::quiet_NaN();

    eckit::DataHandle& handle = dataHandle();
    eckit::Offset position    = handle.position();

    std::ostream out(new eckit::HandleBuf(handle));

    ASSERT(field.dimensions() % 2 == 0);
    for (size_t j = 0; j < field.dimensions(); j += 2) {
        const auto& values_u = field.values(j);
        const auto& values_v = field.values(j + 1);
        ASSERT(values_u.size() == values_v.size());

        out << "#GEO"
               "\n#FORMAT XY_VECTOR"
               "\n# lat  lon  height  date  time  u  v"
               "\n#DATA";

        std::string extra;
        for (const auto& key : {"height", "date", "time"}) {
            std::string v("0");
            param.get(key, v);
            extra += ' ' + v;
        }

        for (const std::unique_ptr<repres::Iterator> it(field.representation()->iterator()); it->next();) {
            const auto& p = it->pointUnrotated();
            auto u        = values_u.at(it->index());
            auto v        = values_v.at(it->index());

            if (u != mv && v != mv) {
                out << "\n" << p.lat().value() << ' ' << p.lon().value() << extra << ' ' << u << ' ' << v;
            }
        }

        out << std::endl;
    }

    delete out.rdbuf();

    // Log::info() << "GeoPointsFileOutputXYVector::save <= " << handle.position() - position << std::endl;

    return size_t(handle.position() - position);
}


static const MIROutputBuilder<GeoPointsFileOutputXYVector> output("geopoints-xy-vector");


}  // namespace mir::output
