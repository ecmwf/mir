/*
 * (C) Copyright 1996-2015 ECMWF.
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



#include "mir/output/GeoPointsOutput.h"
#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Iterator.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/repres/Representation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "eckit/io/HandleBuf.h"
#include "mir/action/context/Context.h"


namespace mir {
namespace output {

// See https://software.ecmwf.int/wiki/display/METV/Geopoints


GeoPointsOutput::GeoPointsOutput():
    once_(true) {
}


GeoPointsOutput::~GeoPointsOutput() {
}


size_t GeoPointsOutput::copy(const param::MIRParametrisation &param, context::Context &ctx) {
    NOTIMP;
    return 0;
}

static const char* keys[] = {"class", "type", "stream", "expver",
                             "date", "time", "step", "number", "levtype",
                             "levelist", "param", 0
                            };

size_t GeoPointsOutput::save(const param::MIRParametrisation &param, context::Context &ctx) {

    if(!once_) {
        std::strstringstream oss;
        oss << "Attempt to write more than once to " << *this;
        throw eckit::SeriousBug(oss.str());
    }
    ASSERT(once_);

    data::MIRField& field = ctx.field();

    eckit::DataHandle& handle = dataHandle();
    eckit::Offset position = handle.position();

    std::ostream out(new eckit::HandleBuf(handle));

    for (size_t j = 0; j < field.dimensions(); ++j) {

        // ASSERT(field.dimensions() == 1);

        param::RuntimeParametrisation runtime(param);
        if (field.paramId(j)) {
            std::ostringstream oss;
            oss << field.paramId(j);
            runtime.set("param", oss.str());
        }

        const std::vector<double>& values = field.values(j);


        // std::cout << "GeoPointsOutput::save => " << handle << std::endl;


        out << "#GEO" << std::endl;

        size_t i = 0;
        while (keys[i]) {
            std::string v;
            if (runtime.get(keys[i], v)) {
                out << "# " << keys[i] << "=" << v << std::endl;
            }
            i++;
        }

        out << "#DATA" << std::endl;


        eckit::ScopedPtr<repres::Iterator> it(field.representation()->rotatedIterator());
        double lat;
        double lon;

        std::vector<double>::const_iterator v = values.begin();

        while (it->next(lat, lon)) {
            ASSERT(v != values.end());
            out << lat << ' ' << lon << ' ' << *v << std::endl;
            ++v;
        }

    }

    once_ = false;

    // std::cout << "GeoPointsOutput::save <= " << handle.position() - position << std::endl;

    return handle.position() - position;
}




}  // namespace output
}  // namespace mir

