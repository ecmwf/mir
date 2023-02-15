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


#include "mir/key/style/CustomParametrisation.h"

#include <ostream>

#include "eckit/utils/StringTools.h"
#include "eckit/utils/Translator.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::key::style {


CustomParametrisation::CustomParametrisation(const std::string& name,
                                             const std::map<std::string, std::vector<std::string> >& params,
                                             const param::MIRParametrisation& parametrisation) :
    name_(name), params_(params), parametrisation_(parametrisation) {}


CustomParametrisation::~CustomParametrisation() = default;


const param::MIRParametrisation& CustomParametrisation::userParametrisation() const {
    return *this;
}


const param::MIRParametrisation& CustomParametrisation::fieldParametrisation() const {
    return *this;
}


template <class T>
static void fill(T& value, const std::vector<std::string>& params) {
    eckit::Translator<std::string, T> t;

    ASSERT(params.size() == 1);
    value = t(params[0]);
}


template <>
void fill(std::string& value, const std::vector<std::string>& params) {
    value = eckit::StringTools::join("/", params);
}


template <class T>
static void fill(std::vector<T>& value, const std::vector<std::string>& params) {
    eckit::Translator<std::string, T> t;

    value.clear();
    for (const auto& j : params) {
        value.push_back(t(j));
    }
}


template <class T>
bool CustomParametrisation::_get(const std::string& name, T& value) const {
    Log::debug() << *this << " get('" << name << "')" << std::endl;

    auto j = params_.find(name);
    if (j != params_.end()) {
        fill(value, j->second);
        return true;
    }

    return parametrisation_.get(name, value);
}


bool CustomParametrisation::has(const std::string& name) const {
    if (params_.find(name) != params_.end()) {
        return true;
    }
    return parametrisation_.has(name);
}


bool CustomParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}


bool CustomParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}


bool CustomParametrisation::get(const std::string& name, int& value) const {
    return _get(name, value);
}


bool CustomParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}


bool CustomParametrisation::get(const std::string& name, float& value) const {
    return _get(name, value);
}


bool CustomParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}


bool CustomParametrisation::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}


bool CustomParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}


bool CustomParametrisation::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}


bool CustomParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}


bool CustomParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


void CustomParametrisation::print(std::ostream& out) const {
    out << "CustomParametrisation[name=" << name_;

    out << ",params=[";
    const char* sep = "";
    for (const auto& p : params_) {
        out << sep << p.first << "=[";
        const char* sepv = "";
        for (const auto& v : p.second) {
            out << sepv << v;
            sepv = ",";
        }
        out << "]";
        sep = ",";
    }

    out << "]";
}


}  // namespace mir::key::style
