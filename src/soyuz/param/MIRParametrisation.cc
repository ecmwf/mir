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


#include "soyuz/param/MIRParametrisation.h"

#include "eckit/exception/Exceptions.h"

#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"

namespace mir {
namespace param {


MIRParametrisation::MIRParametrisation() {
}


MIRParametrisation::~MIRParametrisation() {
}


bool MIRParametrisation::has(const std::string& name) const {
    std::string ignore;
    return get(name, ignore);
}

static inline const char* name_of(bool) {
    return "bool";
}

static inline const char* name_of(long) {
    return "long";
}

static inline const char* name_of(double) {
    return "double";
}

template<class T>
bool MIRParametrisation::_sget(const std::string& name, T& value) const {
    eckit::Translator<std::string, T> translate;
    std::string s;

    if (!get(name, s)) {
        return false;
    }

    eckit::Log::warning() << " ++++ WARNING MIRParametrisation::get(" << name << "), want " << name_of(value) << " from string" << std::endl;
    value = translate(s);

    return true;
}

template<class T>
bool MIRParametrisation::_vget(const std::string& name, std::vector<T>& value) const {
    eckit::Translator<std::string, T> translate;
    std::string s;

    if (!get(name, s)) {
        return false;
    }

    eckit::Log::warning() << " ++++ WARNING MIRParametrisation::get(" << name << "), want std::vector<" << name_of(T()) << "> from string" << std::endl;

    eckit::Tokenizer parse("/");

    std::vector<std::string> p;

    parse(s, p);

    value.clear();
    value.reserve(p.size());

    for (std::vector<std::string>::const_iterator j = p.begin(); j != p.end(); ++j) {
        value.push_back(translate(*j));
    }

    return true;
}


bool MIRParametrisation::get(const std::string& name, bool& value) const {
    return _sget(name, value);

}

bool MIRParametrisation::get(const std::string& name, long& value) const {
    return _sget(name, value);

}

bool MIRParametrisation::get(const std::string& name, double& value) const {
    return _sget(name, value);

}

bool MIRParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _vget(name, value);
}

bool MIRParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _vget(name, value);
}


}  // namespace param
}  // namespace mir

