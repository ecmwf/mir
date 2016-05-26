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

#include "eckit/log/Plural.h"
#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/log/MIR.h"
#include "mir/action/Job.h"

#include "mir/repres/Representation.h"

#include "mir/api/MIRJob.h"


namespace mir {
namespace api {


MIRJob::MIRJob() {
}


MIRJob::~MIRJob() {
}


void MIRJob::execute(input::MIRInput &input, output::MIROutput &output) const {
    action::Job(*this, input, output).execute();
}

void MIRJob::print(std::ostream &out) const {
    if (eckit::format(out) == eckit::Log::applicationFormat) {
        out << "mir_tool ";
        SimpleParametrisation::print(out);
        out << " in.grib out.grib";
    } else {
        out << "MIRJob[";
        SimpleParametrisation::print(out);
        out << "]";
    }
}

bool MIRJob::empty() const {
    return size() == 0;
}

MIRJob &MIRJob::clear(const std::string &name) {
    eckit::Log::trace<MIR>() << "************* MIRJob::clear [" << name << "]" << std::endl;
    SimpleParametrisation::clear(name);
    return *this;
}
MIRJob &MIRJob::set(const std::string &name, const std::string &value) {
    eckit::Log::trace<MIR>() << "************* MIRJob::set [" << name << "] = [" << value << "] (string)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, const char *value) {
    eckit::Log::trace<MIR>() << "************* MIRJob::set [" << name << "] = [" << value << "] (char)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, bool value) {
    eckit::Log::trace<MIR>() << "************* MIRJob::set [" << name << "] = [" << value << "] (bool)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}


MIRJob &MIRJob::set(const std::string &name, long value) {
    eckit::Log::trace<MIR>() << "************* MIRJob::set [" << name << "] = [" << value << "] (long)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}


MIRJob &MIRJob::set(const std::string &name, size_t value) {
    ASSERT(long(value) == value);
    SimpleParametrisation::set(name, long(value));
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, double value) {
    eckit::Log::trace<MIR>() << "************* MIRJob::set [" << name << "] = [" << value << "] (double)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, param::DelayedParametrisation *value) {
    eckit::Log::trace<MIR>() << "************* MIRJob::set [" << name << "] = [" << value << "] (delayed)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, const std::vector<long>& v) {
    eckit::Log::trace<MIR>() << "************* MIRJob::set [" << name << "] = [" << eckit::Plural(v.size(), "value") << "] (vector<long>)" << std::endl;
    SimpleParametrisation::set(name, v);
    return *this;
}

MIRJob&MIRJob::set(const eckit::Value& map) {
    eckit::Log::trace<MIR>() << "************* MIRJob::set from ValueMap" << std::endl;
    SimpleParametrisation::set(map);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, double v1, double v2) {
    eckit::Log::trace<MIR>() << "************* MIRJob::set [" << name << "] = [" << v1 << ", "  << v2 << "] (double)" << std::endl;
    std::vector<double> v(2);
    v[0] = v1;
    v[1] = v2;
    SimpleParametrisation::set(name, v);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, double v1, double v2, double v3, double v4) {
    eckit::Log::trace<MIR>() << "************* MIRJob::set [" << name << "] =  [" << v1
                       << ", "  << v2 << ", "  << v3 << ", "  << v4 << "] (double)" << std::endl;
    std::vector<double> v(4);
    v[0] = v1;
    v[1] = v2;
    v[2] = v3;
    v[3] = v4;
    SimpleParametrisation::set(name, v);
    return *this;
}

MIRJob& MIRJob::representationFrom(input::MIRInput& input) {
    eckit::ScopedPtr< data::MIRField > field(input.field());

    const repres::Representation* repres = field->representation();

    eckit::Log::trace<MIR>() << "Copy from " << *repres << std::endl;
    repres->fill(*this);

    return *this;
}


bool MIRJob::matches(const param::MIRParametrisation &metadata) const {
    static const char *force[] = { "vod2uv", "bitmap",
                                   "frame", "packing",
                                   "accuracy", "checkerboard",
                                   "pattern",
                                   0
                                 }; // Move to MIRLogic
    size_t i = 0;
    while (force[i]) {
        if (has(force[i])) {
            eckit::Log::trace<MIR>() << "MIRJob will perform transformation/interpolation ('"
                               << force[i] << "' specified)" << std::endl;
            return false;
        }
        i++;
    }

    bool ok = SimpleParametrisation::matches(metadata);
    if (!ok) {
        eckit::Log::trace<MIR>() << "MIRJob will perform transformation/interpolation" << std::endl;
    }
    return ok;
}

void MIRJob::mirToolCall(std::ostream &out) const {
    int fmt = eckit::format(out);
    eckit::setformat(out, eckit::Log::applicationFormat);
    out << *this;
    eckit::setformat(out, fmt);
}

void MIRJob::json(eckit::JSON& json) const {
    SimpleParametrisation::json(json);
}

}  // namespace api
}  // namespace mir

