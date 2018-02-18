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



bool GeoPointsOutput::sameParametrisation(const param::MIRParametrisation &param1,
        const param::MIRParametrisation & param2) const {
    return true;
}

bool GeoPointsOutput::printParametrisation(std::ostream& out, const param::MIRParametrisation &param) const {
    return false;
}

size_t GeoPointsOutput::save(const param::MIRParametrisation &param, context::Context &ctx) {

    if (!once_) {
        std::ostringstream oss;
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
        auto md = field.metadata(j);
        if (md.find("paramId") != md.end()) {
            std::ostringstream oss;
            oss << md["paramId"];
            runtime.set("param", oss.str());
        }

        const std::vector<double>& values = field.values(j);


        // eckit::Log::info() << "GeoPointsOutput::save => " << handle << std::endl;


        out << "#GEO"
            << "\n#FORMAT XYV";

        size_t i = 0;
        while (keys[i]) {
            std::string v;
            if (runtime.get(keys[i], v)) {
                out << "\n# " << keys[i] << "=" << v;
            }
            i++;
        }

        out << "\n#DATA";


        std::vector<double>::const_iterator v = values.begin();

        eckit::ScopedPtr<repres::Iterator> it(field.representation()->iterator());
        while (it->next()) {
            const repres::Iterator::point_ll_t& p = it->pointUnrotated();
            ASSERT(v != values.end());
            out << "\n" << p.lon.value() << ' ' << p.lat.value() << ' ' << *v;
            ++v;
        }

        out << std::endl;
    }

    once_ = false;

    // eckit::Log::info() << "GeoPointsOutput::save <= " << handle.position() - position << std::endl;

    return handle.position() - position;
}




}  // namespace output
}  // namespace mir

