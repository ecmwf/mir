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


#include "mir/api/MIRJob.h"

#include <iostream>
#include "eckit/log/Plural.h"
#include "mir/action/plan/Job.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace api {


MIRJob::MIRJob() {
}


MIRJob::~MIRJob() {
}


void MIRJob::execute(input::MIRInput &input, output::MIROutput &output, util::MIRStatistics& statistics) const {
    action::Job(*this, input, output).execute(statistics);
}

void MIRJob::execute(input::MIRInput &input, output::MIROutput &output) const {
    util::MIRStatistics statistics;
    execute(input, output, statistics);
}

void MIRJob::print(std::ostream &out) const {
    if (eckit::format(out) == eckit::Log::applicationFormat) {
        out << "mir-tool ";
        SimpleParametrisation::print(out);
        out << " in.grib out.grib";
    } else {
        out << "MIRJob[";
        SimpleParametrisation::print(out);
        out << "]";
    }
}

MIRJob &MIRJob::reset() {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::reset()" << std::endl;
    SimpleParametrisation::reset();
    return *this;
}

MIRJob &MIRJob::clear(const std::string &name) {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::clear [" << name << "]" << std::endl;
    SimpleParametrisation::clear(name);
    return *this;
}
MIRJob &MIRJob::set(const std::string &name, const std::string &value) {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::set [" << name << "] = [" << value << "] (string)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, const char *value) {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::set [" << name << "] = [" << value << "] (char)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, bool value) {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::set [" << name << "] = [" << value << "] (bool)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}


MIRJob &MIRJob::set(const std::string &name, long value) {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::set [" << name << "] = [" << value << "] (long)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}


MIRJob &MIRJob::set(const std::string &name, size_t value) {
    ASSERT(size_t(long(value)) == value);
    SimpleParametrisation::set(name, long(value));
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, double value) {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::set [" << name << "] = [" << value << "] (double)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, param::DelayedParametrisation *value) {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::set [" << name << "] = [" << value << "] (delayed)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, const std::vector<long>& v) {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::set [" << name << "] = [" << eckit::Plural(v.size(), "value") << "] (vector<long>)" << std::endl;
    SimpleParametrisation::set(name, v);
    return *this;
}


MIRJob &MIRJob::set(const std::string &name, const std::vector<double>& v) {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::set [" << name << "] = [" << eckit::Plural(v.size(), "value") << "] (vector<double>)" << std::endl;
    SimpleParametrisation::set(name, v);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, double v1, double v2) {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::set [" << name << "] = [" << v1 << ", "  << v2 << "] (double)" << std::endl;
    std::vector<double> v(2);
    v[0] = v1;
    v[1] = v2;
    SimpleParametrisation::set(name, v);
    return *this;
}

MIRJob &MIRJob::set(const std::string &name, double v1, double v2, double v3, double v4) {
    // eckit::Log::debug<LibMir>() << "************* MIRJob::set [" << name << "] =  [" << v1
                             // << ", "  << v2 << ", "  << v3 << ", "  << v4 << "] (double)" << std::endl;
    std::vector<double> v(4);
    v[0] = v1;
    v[1] = v2;
    v[2] = v3;
    v[3] = v4;
    SimpleParametrisation::set(name, v);
    return *this;
}

MIRJob& MIRJob::representationFrom(input::MIRInput& input) {

    const repres::Representation* repres = input.field().representation();

    // eckit::Log::debug<LibMir>() << "Copy from " << *repres << std::endl;
    repres->fill(*this);

    return *this;
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

