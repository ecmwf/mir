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


#include "mir/output/GriddefOutput.h"

#include <fstream>
#include <ios>
#include <memory>
#include <ostream>

#include "eckit/filesystem/PathName.h"
#include "eckit/serialisation/FileStream.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::output {


GriddefOutput::GriddefOutput(std::string path, bool binary) : path_(std::move(path)), binary_(binary) {}


void GriddefOutput::save(const eckit::PathName& path, const std::vector<double>& latitudes,
                         const std::vector<double>& longitudes, bool binary) {
    ASSERT(latitudes.size() == longitudes.size());
    const auto count = latitudes.size();

    if (binary) {
        eckit::FileStream s(path, "w");
        s << static_cast<size_t>(1);  // version
        s << static_cast<size_t>(count);

        for (size_t i = 0; i < count; ++i) {
            s << latitudes[i];
            s << longitudes[i];
        }

        s.close();
    }
    else {
        std::ofstream out(path.asString().c_str());
        std::fixed(out).precision(6);  // dot + 6 digits

        for (size_t i = 0; i < count; ++i) {
            out << latitudes[i] << ' ' << longitudes[i] << '\n';
        }
    }
}


size_t GriddefOutput::save(const param::MIRParametrisation& param, context::Context& ctx) {
    const auto& field = ctx.field();
    repres::RepresentationHandle repres{field.representation()};

    if (binary_) {
        eckit::FileStream s(path_, "w");
        s << static_cast<size_t>(1);  // version
        s << static_cast<size_t>(repres->numberOfPoints());

        for (std::unique_ptr<repres::Iterator> it{repres->iterator()}; it->next();) {
            const auto& p = **it;
            s << p[0];  // latitude
            s << p[1];  // longitude
        }

        s.close();
    }
    else {
        std::ofstream out(path_.c_str());
        std::fixed(out).precision(6);  // dot + 6 digits

        for (std::unique_ptr<repres::Iterator> it{repres->iterator()}; it->next();) {
            const auto& p = **it;
            out << p[0] << ' ' << p[1] << '\n';
        }
    }

    return 1;
}


bool GriddefOutput::sameAs(const MIROutput& other) const {
    const auto* o = dynamic_cast<const GriddefOutput*>(&other);
    return (o != nullptr) && eckit::PathName(path_) == eckit::PathName(o->path_);
}


bool GriddefOutput::sameParametrisation(const param::MIRParametrisation& /*unused*/,
                                        const param::MIRParametrisation& /*unused*/) const {
    return false;
}


bool GriddefOutput::printParametrisation(std::ostream& /*unused*/, const param::MIRParametrisation& /*unused*/) const {
    return false;
}


void GriddefOutput::print(std::ostream& out) const {
    out << "GriddefOutput[path=" << path_ << ",binary=" << binary_ << "]";
}


static const MIROutputBuilder<GriddefBinaryOutput> _output1("griddef");
static const MIROutputBuilder<GriddefTextOutput> _output2("griddef-text");


}  // namespace mir::output
