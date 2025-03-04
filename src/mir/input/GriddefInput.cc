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


#include "mir/input/GriddefInput.h"

#include <cctype>
#include <fstream>
#include <ostream>
#include "eckit/filesystem/PathName.h"
#include "eckit/serialisation/IfstreamStream.h"

#include "mir/data/MIRField.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::input {


GriddefInput::GriddefInput(const eckit::PathName& path) : calls_(0) {
    load(path, latitudes_, longitudes_);
    ASSERT(latitudes_.size() == longitudes_.size());
    ASSERT(!latitudes_.empty());

    parametrisation_.set("gridded", true);
}


void GriddefInput::load(const eckit::PathName& path, std::vector<double>& latitudes, std::vector<double>& longitudes) {
    Log::info() << "GriddefInput::load '" << path << "'" << std::endl;

    std::ifstream in(path.asString().c_str());
    if (!in) {
        throw exception::CantOpenFile(path);
    }

    if (std::isprint(in.peek()) == 0) {
        // binary
        eckit::IfstreamStream s(in);

        size_t version = 0;
        s >> version;
        ASSERT(version == 1);

        size_t count = 0;
        s >> count;

        latitudes.resize(count);
        longitudes.resize(count);

        for (size_t i = 0; i < count; ++i) {
            s >> latitudes[i];
            s >> longitudes[i];
        }
    }
    else {
        double lat = 0;
        double lon = 0;
        while (in >> lat >> lon) {
            latitudes.push_back(lat);
            longitudes.push_back(lon);
        }
    }
}


bool GriddefInput::next() {
    return calls_++ == 0;
}


size_t GriddefInput::dimensions() const {
    return 1;
}


const param::MIRParametrisation& GriddefInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return parametrisation_;
}


data::MIRField GriddefInput::field() const {
    return {new repres::other::UnstructuredGrid{latitudes_, longitudes_}};
}


bool GriddefInput::sameAs(const MIRInput&) const {
    return false;
}


void GriddefInput::print(std::ostream& out) const {
    out << "GriddefInput[size=" << latitudes_.size() << "]";
}


}  // namespace mir::input
