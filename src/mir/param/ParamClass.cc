/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#include "mir/param/ParamClass.h"

#include <iostream>
#include "eckit/filesystem/PathName.h"
//include "eckit/utils/Translator.h"
#include "mir/config/LibMir.h"
#include "mir/util/Parser.h"


namespace mir {
namespace param {


ParamClass::ParamClass():
    scope_(0) {

    eckit::PathName path("~mir/etc/mir/param-class.cfg");
    eckit::Log::debug<LibMir>() << "Loading parameter classes from " << path << std::endl;
    if (!path.exists()) {
        eckit::Log::warning() << "Cannot load " << path << std::endl;
        return;
        // throw eckit::CantOpenFile(path, Here());
    }
    util::Parser parser(path);
    parser.fill(*this);

}


ParamClass::~ParamClass() {
    for (map_t::iterator j = settings_.begin(); j != settings_.end(); ++j) {
        delete (*j).second;
    }
}


const ParamClass& ParamClass::instance() {
    static ParamClass instance_;
    return instance_;
}


void ParamClass::print(std::ostream& out) const {
    out << "ParamClass[...]";
}


void ParamClass::store(const std::string& name, const char* value) {
    store(name, std::string(value));
}


void ParamClass::store(const std::string& name, const std::string& value) {
    ASSERT(scope_);
    eckit::Log::debug<LibMir>() << "From configuration file " << name << "=[" << value << "] (string) paramClass=" << current_ << std::endl;
    scope_->set(name, value);
}


void ParamClass::store(const std::string& name, bool value) {
    ASSERT(scope_);
    eckit::Log::debug<LibMir>() << "From configuration file " << name << "=[" << value << "] (bool) paramClass=" << current_ << std::endl;
    scope_->set(name, value);
}


void ParamClass::store(const std::string& name, long value) {
    ASSERT(scope_);
    eckit::Log::debug<LibMir>() << "From configuration file " << name << "=[" << value << "] (long) paramClass=" << current_ << std::endl;
    scope_->set(name, value);

}


void ParamClass::store(const std::string& name, double value) {
    ASSERT(scope_);
    eckit::Log::debug<LibMir>() << "From configuration file " << name << "=[" << value << "] (double) paramClass=" << current_ << std::endl;
    scope_->set(name, value);
}


void ParamClass::scope(const std::string& name) {
    const std::string scope = name;
    eckit::Log::debug<LibMir>() << "ParamClass::scope(paramClass=" << scope << ")" << std::endl;

    map_t::iterator j = settings_.find(scope);
    if (j == settings_.end()) {
        settings_[scope] = new SimpleParametrisation();
    }
    current_ = scope;
    scope_ = settings_[scope];
}


const SimpleParametrisation* ParamClass::lookup(const std::string& paramClass) const {
    map_t::const_iterator j = settings_.find(paramClass);
    if (j != settings_.end()) {
        return (*j).second;
    }
    throw eckit::UserError("Cannot find a definition for parameter class \"" + paramClass + "\"", Here());
}


bool ParamClass::has(const std::string& paramClass) const {
    return (settings_.find(paramClass) != settings_.end());
}


}  // namespace param
}  // namespace mir

