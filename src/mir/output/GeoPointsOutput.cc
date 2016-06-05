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


#include <iostream>
#include <fstream>

#include "mir/output/GeoPointsOutput.h"
#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Iterator.h"
#include "mir/param/MIRParametrisation.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/repres/Representation.h"
#include "mir/param/RuntimeParametrisation.h"


namespace mir {
namespace output {

// See https://software.ecmwf.int/wiki/display/METV/Geopoints


GeoPointsOutput::GeoPointsOutput(const std::string& path):
    path_(path) {
}


GeoPointsOutput::~GeoPointsOutput() {
}


bool GeoPointsOutput::sameAs(const MIROutput& other) const {
    const GeoPointsOutput* o = dynamic_cast<const GeoPointsOutput*>(&other);
    return o && (path_ == o->path_);
}


size_t GeoPointsOutput::copy(const param::MIRParametrisation &param, input::MIRInput &input) {
    NOTIMP;
    return 0;
}

static const char* keys[] = {"class", "type", "stream", "expver",  "date", "time", "step", "number", "levtype", "levelist", "param", 0};

size_t GeoPointsOutput::save(const param::MIRParametrisation &param, input::MIRInput &input, data::MIRField &field) {

    std::cout << "Save " << *this << std::endl;

    ASSERT(field.dimensions() == 1);

    param::RuntimeParametrisation runtime(param);
    if (field.paramId(0)) {
        runtime.set("param", long(field.paramId(0)));
    }

    const std::vector<double> values = field.values(0);

    std::ofstream out(path_);
    if (!out) {
        throw eckit::CantOpenFile(path_);
    }

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

    out.close();
    if (out.bad()) {
        throw eckit::WriteError(path_);
    }
    return 0;
}


void GeoPointsOutput::print(std::ostream &out) const {
    out << "GeoPointsOutput[path=" << path_ << "]";
}



}  // namespace output
}  // namespace mir

