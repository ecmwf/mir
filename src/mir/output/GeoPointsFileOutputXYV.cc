/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/output/GeoPointsFileOutputXYV.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/HandleBuf.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace output {


// See https://software.ecmwf.int/wiki/display/METV/Geopoints


GeoPointsFileOutputXYV::GeoPointsFileOutputXYV(
        const std::string& path,
        const param::MIRParametrisation&) :
    GeoPointsFileOutput(path) {
}


static const char* keys[] = {"class", "type", "stream", "expver",
                             "date", "time", "step", "number", "levtype",
                             "levelist", "param", 0
                            };


size_t GeoPointsFileOutputXYV::copy(const param::MIRParametrisation&, context::Context&) {
    NOTIMP;
}


size_t GeoPointsFileOutputXYV::save(const param::MIRParametrisation& param, context::Context& ctx) {
    ASSERT(once());

    data::MIRField& field = ctx.field();

    eckit::DataHandle& handle = dataHandle();
    eckit::Offset position = handle.position();

    std::ostream out(new eckit::HandleBuf(handle));

    for (size_t j = 0; j < field.dimensions(); ++j) {

        // ASSERT(field.dimensions() == 1);

        param::RuntimeParametrisation runtime(param);
        auto md = field.metadata(j);
        if (md.find("paramId") != md.end()) {
            std::ostringstream oss;
            oss << md["paramId"];
            runtime.set("param", oss.str());
        }

        const std::vector<double>& values = field.values(j);


        // eckit::Log::info() << "GeoPointsFileOutputXYV::save => " << handle << std::endl;


        out << "#GEO"
               "\n#FORMAT XYV";

        // FIXME: what's going on??
//        for (auto& key : keys) {
//            std::string v;
//            if (runtime.get(key, v)) {
//                out << "\n# " << key << "=" << v;
//            }
//        }

        out << "\n#DATA";


        std::vector<double>::const_iterator v = values.cbegin();

        eckit::ScopedPtr<repres::Iterator> it(field.representation()->iterator());
        while (it->next()) {
            const repres::Iterator::point_ll_t& p = it->pointUnrotated();
            ASSERT(v != values.cend());
            out << "\n" << p.lon.value() << ' ' << p.lat.value() << ' ' << *v;
            ++v;
        }

        out << std::endl;
    }

    // eckit::Log::info() << "GeoPointsFileOutputXYV::save <= " << handle.position() - position << std::endl;

    return handle.position() - position;
}


static MIROutputBuilder<GeoPointsFileOutputXYV> output("geopoints-xyv", {".gp", ".gpt", ".geo"});


}  // namespace output
}  // namespace mir

