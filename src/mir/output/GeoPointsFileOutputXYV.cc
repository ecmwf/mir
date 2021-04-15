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


#include "mir/output/GeoPointsFileOutputXYV.h"

#include <limits>
#include <memory>
#include <sstream>

#include "eckit/io/HandleBuf.h"
#include "eckit/serialisation/HandleStream.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace output {


// See https://software.ecmwf.int/wiki/display/METV/Geopoints


GeoPointsFileOutputXYV::GeoPointsFileOutputXYV(const std::string& path, bool binary) :
    GeoPointsFileOutput(path, binary) {}


static const char* keys[] = {"class", "type",   "stream",  "expver",   "date", "time",
                             "step",  "number", "levtype", "levelist", "param"};


size_t GeoPointsFileOutputXYV::copy(const param::MIRParametrisation&, context::Context&) {
    NOTIMP;
}


size_t GeoPointsFileOutputXYV::save(const param::MIRParametrisation& param, context::Context& ctx) {
    ASSERT(once());
    return binary_ ? saveBinary(param, ctx) : saveText(param, ctx);
}


size_t GeoPointsFileOutputXYV::saveText(const param::MIRParametrisation& param, context::Context& ctx) {
    const auto& field = ctx.field();
    auto mv           = field.hasMissing() ? field.missingValue() : std::numeric_limits<double>::quiet_NaN();

    eckit::DataHandle& handle = dataHandle();
    eckit::Offset position    = handle.position();

    std::ostream out(new eckit::HandleBuf(handle));
    std::vector<double> latitudes;
    std::vector<double> longitudes;

    for (size_t j = 0; j < field.dimensions(); ++j) {

        // ASSERT(field.dimensions() == 1);

        param::RuntimeParametrisation runtime(param);
        auto md = field.metadata(j);
        if (md.find("paramId") != md.end()) {
            std::ostringstream oss;
            oss << md["paramId"];
            runtime.set("param", oss.str());
        }

        const MIRValuesVector& values = field.values(j);


        // Log::info() << "GeoPointsFileOutputXYV::save => " << handle << std::endl;


        out << "#GEO"
               "\n#FORMAT XYV";

        for (auto& key : keys) {
            std::string v;
            if (runtime.get(key, v)) {
                out << "\n# " << key << "=" << v;
            }
        }

        out << "\n#DATA";


        auto v = values.cbegin();

        latitudes.reserve(values.size());
        longitudes.reserve(values.size());

        std::unique_ptr<repres::Iterator> it(field.representation()->iterator());
        while (it->next()) {
            const auto& p = it->pointUnrotated();
            ASSERT(v != values.cend());
            if (*v != mv) {
                out << "\n" << p.lon().value() << ' ' << p.lat().value() << ' ' << *v;
            }
            ++v;

            latitudes.push_back(p.lat().value());
            longitudes.push_back(p.lon().value());
        }
        ASSERT(v == values.cend());

        out << std::endl;
    }

    std::ostringstream oss;
    oss << "GeoPointsFileOutputXYV save " << handle;

    repres::other::UnstructuredGrid::check(oss.str(), latitudes, longitudes);

    delete out.rdbuf();

    // Log::info() << "GeoPointsFileOutputXYV::save <= " << handle.position() - position << std::endl;

    return size_t(handle.position() - position);
}


size_t GeoPointsFileOutputXYV::saveBinary(const param::MIRParametrisation& param, context::Context& ctx) {

    const data::MIRField& field = ctx.field();

    eckit::DataHandle& handle = dataHandle();
    eckit::Offset position    = handle.position();

    eckit::HandleStream out(handle);


    std::vector<double> latitudes;
    std::vector<double> longitudes;

    for (size_t j = 0; j < field.dimensions(); ++j) {

        // ASSERT(field.dimensions() == 1);

        param::RuntimeParametrisation runtime(param);
        auto md = field.metadata(j);
        if (md.find("paramId") != md.end()) {
            std::ostringstream oss;
            oss << md["paramId"];
            runtime.set("param", oss.str());
        }

        const MIRValuesVector& values = field.values(j);

        // Log::info() << "GeoPointsFileOutputXYV::save => " << handle << std::endl;

        out << "GEO";
        out << "XYV";

        for (auto& key : keys) {
            std::string v;
            if (runtime.get(key, v)) {
                out << key << v;
            }
        }

        out << "-";

        auto v = values.cbegin();
        out << values.size();

        latitudes.reserve(values.size());
        longitudes.reserve(values.size());

        std::unique_ptr<repres::Iterator> it(field.representation()->iterator());
        size_t i = 0;
        while (it->next()) {
            const auto& p = it->pointUnrotated();
            ASSERT(v != values.cend());
            out << double(p.lon().value()) << double(p.lat().value()) << double(*v);

            latitudes.push_back(p.lat().value());
            longitudes.push_back(p.lon().value());


            ++v;
            ++i;
        }
        ASSERT(v == values.cend());
        ASSERT(i == values.size());
    }

    out << "END";


    std::ostringstream oss;
    oss << "GeoPointsFileOutputXYV save " << handle;

    repres::other::UnstructuredGrid::check(oss.str(), latitudes, longitudes);


    // Log::info() << "GeoPointsFileOutputXYV::save <= " << handle.position() - position << std::endl;

    return size_t(handle.position() - position);
}


static MIROutputBuilder<GeoPointsFileOutputXYV> output1("geopoints-xyv", {".gp", ".gpt", ".geo"});
static MIROutputBuilder<GeoPointsFileOutputXYV> output2("geopoints");


}  // namespace output
}  // namespace mir
