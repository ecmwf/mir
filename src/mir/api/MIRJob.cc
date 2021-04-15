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


#include "mir/api/MIRJob.h"

#include <ostream>

#include "eckit/utils/Tokenizer.h"

#include "mir/action/plan/Job.h"
#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace api {


MIRJob::MIRJob() = default;


MIRJob::~MIRJob() = default;


void MIRJob::execute(input::MIRInput& input, output::MIROutput& output, util::MIRStatistics& statistics) const {

    bool dont_compress = false;
    get("dont-compress-plan", dont_compress);

    action::Job(*this, input, output, !dont_compress).execute(statistics);
}


void MIRJob::execute(input::MIRInput& input, output::MIROutput& output) const {
    util::MIRStatistics statistics;
    execute(input, output, statistics);
}

void MIRJob::estimate(input::MIRInput& input, output::MIROutput& output, MIREstimation& estimation) const {
    action::Job(*this, input, output, true).estimate(estimation);
}


void MIRJob::print(std::ostream& out) const {
    if (eckit::format(out) == Log::applicationFormat) {
        out << "mir";
        SimpleParametrisation::print(out);
        out << " in.grib out.grib";
    }
    else {
        out << "MIRJob[";
        SimpleParametrisation::print(out);
        out << "]";
    }
}


MIRJob& MIRJob::set(const std::string& args) {
    eckit::Tokenizer parseSpace(" ");
    eckit::Tokenizer parseEquals("=");

    std::vector<std::string> argv;
    parseSpace(args, argv);
    for (const std::string& a : argv) {

        std::vector<std::string> nameValue;
        parseEquals(a, nameValue);

        if (nameValue.size() == 1) {
            nameValue.emplace_back("true");
        }

        if (nameValue[0].find("--") != 0) {
            throw exception::UserError("MIRJob::set: invalid parameter '" + a + "'");
        }

        set(nameValue[0].substr(2), nameValue[1]);
    }
    return *this;
}


static const std::map<std::string, std::string> aliases{
    //    {"resol", "truncation"},
    {"gridname", "grid"},
};


static const std::string& resolveAliases(const std::string& name) {
    auto j = aliases.find(name);
    if (j != aliases.end()) {
        Log::debug() << "MIRJob: changing [" << name << "] to [" << j->second << "]" << std::endl;
        return j->second;
    }
    return name;
}


template <class T>
static const T& resolveAliases(const std::string& /*name*/, const T& value) {
    return value;
}


MIRJob& MIRJob::clear(const std::string& name) {
    auto& rName = resolveAliases(name);
    Log::debug() << "MIRJob: clear '" << rName << "'" << std::endl;
    SimpleParametrisation::clear(rName);
    return *this;
}


template <class T>
MIRJob& MIRJob::_setScalar(const std::string& name, const T& value) {
    Log::debug() << "MIRJob: set '" << name << "'='" << value << "'" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}


template <class T>
MIRJob& MIRJob::_setVector(const std::string& name, const T& value, size_t outputCount) {
    auto& out = Log::debug();

    out << "MIRJob: set '" << name << "'='";
    const char* sep = "";
    size_t n        = 0;
    for (; n < outputCount && n < value.size(); ++n) {
        out << sep << value[n];
        sep = "/";
    }
    if (n < value.size()) {
        out << sep << "...";
    }
    out << "'" << std::endl;

    SimpleParametrisation::set(name, value);
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, const std::string& value) {
    _setScalar(resolveAliases(name), resolveAliases(name, value));
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, const char* value) {
    _setScalar(resolveAliases(name), resolveAliases(name, value));
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, float value) {
    _setScalar(resolveAliases(name), resolveAliases(name, value));
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, bool value) {
    _setScalar(resolveAliases(name), resolveAliases(name, value));
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, long value) {
    _setScalar(resolveAliases(name), resolveAliases(name, value));
    return *this;
}

MIRJob& MIRJob::set(const std::string& name, long long value) {
    _setScalar(resolveAliases(name), resolveAliases(name, value));
    return *this;
}

MIRJob& MIRJob::set(const std::string& name, size_t value) {
    ASSERT(size_t(long(value)) == value);
    _setScalar(resolveAliases(name), long(value));
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, double value) {
    _setScalar(resolveAliases(name), resolveAliases(name, value));
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, int value) {
    _setScalar(resolveAliases(name), resolveAliases(name, value));
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, const std::vector<int>& value) {
    _setVector(resolveAliases(name), resolveAliases(name, value));
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, const std::vector<long>& v) {
    _setVector(resolveAliases(name), v);
    return *this;
}

MIRJob& MIRJob::set(const std::string& name, const std::vector<long long>& v) {
    _setVector(resolveAliases(name), v);
    return *this;
}

MIRJob& MIRJob::set(const std::string& name, const std::vector<size_t>& value) {
    _setVector(resolveAliases(name), resolveAliases(name, value));
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, const std::vector<float>& value) {
    _setVector(resolveAliases(name), resolveAliases(name, value));
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, const std::vector<double>& v) {
    _setVector(resolveAliases(name), v);
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, const std::vector<std::string>& value) {
    _setVector(resolveAliases(name), resolveAliases(name, value));
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, double v1, double v2) {
    std::vector<double> v(2);
    v[0] = v1;
    v[1] = v2;
    _setVector(resolveAliases(name), v, 2);
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, double v1, double v2, double v3, double v4) {
    std::vector<double> v(4);
    v[0] = v1;
    v[1] = v2;
    v[2] = v3;
    v[3] = v4;
    _setVector(resolveAliases(name), v, 4);
    return *this;
}


MIRJob& MIRJob::representationFrom(const input::MIRInput& input) {

    const data::MIRField field           = input.field();
    const repres::Representation* repres = field.representation();
    ASSERT(repres);

    // Log::debug() << "Copy from " << *repres << std::endl;
    repres->fill(*this);

    return *this;
}


void MIRJob::mirToolCall(std::ostream& out) const {
    auto fmt = eckit::format(out);
    out << eckit::setformat(Log::applicationFormat) << *this << eckit::setformat(fmt);
}


void MIRJob::json(eckit::JSON& json) const {
    SimpleParametrisation::json(json);
}

}  // namespace api
}  // namespace mir
