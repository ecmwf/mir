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


#include "mir/output/GmshOutput.h"

#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

#include "eckit/filesystem/PathName.h"
#include "eckit/parser/YAMLParser.h"

#include "mir/action/context/Context.h"
#include "mir/caching/InMemoryMeshCache.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Trace.h"
#include "mir/util/ValueMap.h"


namespace mir {
namespace output {


GmshOutput::GmshOutput(std::string path) : path_(std::move(path)) {}


size_t GmshOutput::save(const param::MIRParametrisation& param, context::Context& ctx) {
    trace::ResourceUsage usage("GmshOutput::save");
    auto timing(ctx.statistics().saveTimer());


    // Options
    atlas::util::Config config;
    config.set("coordinates", "xyz");  // Atlas option
    config.set("ghost", false);        // ...
    config.set("write_mesh", true);    // non-Atlas option
    config.set("write_values", true);  // ...

    std::string yaml;
    if (param.get("output", yaml)) {
        util::ValueMap map(eckit::YAMLParser::decodeString(yaml));
        map.set(config);
    }

    auto writeMesh   = config.getBool("write_mesh", true);
    auto writeValues = config.getBool("write_values", true);
    auto overwrite   = config.getBool("overwrite", true);


    // Grid
    const auto& field = ctx.field();
    repres::RepresentationHandle rep(field.representation());
    auto grid = rep->atlasGrid();


    // Mesh
    atlas::Mesh mesh;
    if (writeMesh) {
        util::MeshGeneratorParameters meshGenParams(param);
        rep->fill(meshGenParams);
        meshGenParams.set("3d", config.getString("coordinates") == "xyz");

        trace::Timer time("Generating mesh");
        mesh = caching::InMemoryMeshCache::atlasMesh(ctx.statistics(), grid, meshGenParams);
    }


    // Path
    auto get_path = [&](const std::string& path, bool overwrite = true) {
        eckit::PathName p(path);
        if (overwrite) {
            return p;
        }

        auto ext  = p.extension();
        auto base = p.dirName() / p.baseName(false);
        for (size_t counter = 1; p.exists(); ++counter) {
            std::ostringstream s;
            s << base << "." << std::setw(4) << std::setfill('0') << counter << ext;
            p = s.str();
        }
        return p;
    };


    for (size_t d = 0; d < field.dimensions(); d++) {
        auto path = get_path(path_, overwrite);
        trace::Timer time("GmshOutput: writing to '" + path + "'");

        atlas::output::Gmsh out(path, config);

        if (writeMesh) {
            out.write(mesh);
        }

        if (writeValues) {
            for (size_t which = 0; which < field.dimensions(); ++which) {
                auto& v = field.values(which);
                atlas::Field f("values", const_cast<double*>(v.data()), atlas::array::make_shape(v.size()));
                if (field.hasMissing()) {
                    f.metadata().set("missing_value_type", "equals").set("missing_value", field.missingValue());
                }
                out.write(f);
            }
        }
    }

    return 1;
}


bool GmshOutput::sameAs(const MIROutput& other) const {
    const auto* o = dynamic_cast<const GmshOutput*>(&other);
    return (o != nullptr) && eckit::PathName(path_) == eckit::PathName(o->path_);
}


bool GmshOutput::sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const {
    return false;
}


bool GmshOutput::printParametrisation(std::ostream&, const param::MIRParametrisation&) const {
    return false;
}


void GmshOutput::print(std::ostream& out) const {
    out << "GmshOutput[path=" << path_ << "]";
}


static const MIROutputBuilder<GmshOutput> _output("gmsh", {".msh"});


}  // namespace output
}  // namespace mir
