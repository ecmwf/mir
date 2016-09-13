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


#include "mir/param/MIRConfiguration.h"

#include <iostream>
#include "eckit/filesystem/PathName.h"
#include "eckit/utils/Translator.h"
#include "mir/config/LibMir.h"
#include "mir/util/Parser.h"


namespace mir {
namespace param {


MIRConfiguration::MIRConfiguration():
    scope_(0) {

    eckit::PathName path("~mir/etc/mir/interpolation-methods.cfg");
    eckit::Log::debug<LibMir>() << "Loading MIR configuration from " << path << std::endl;
    if (!path.exists()) {
        eckit::Log::warning() << "Cannot load " << path << std::endl;
        return;
        // throw eckit::CantOpenFile(path, Here());
    }

    util::Parser parser(path);
    parser.fill(*this);
}


MIRConfiguration::~MIRConfiguration() {
    for (map_t::iterator j = settings_.begin(); j != settings_.end(); ++j) {
        delete (*j).second;
    }
}


const MIRConfiguration& MIRConfiguration::instance() {
    static MIRConfiguration instance_;
    return instance_;
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration[...]";
}


void MIRConfiguration::store(const std::string& name, const char* value) {
    store(name, std::string(value));
}


void MIRConfiguration::store(const std::string& name, const std::string& value) {
    ASSERT(scope_);
    eckit::Log::debug<LibMir>() << "From configuration file " << name << "=[" << value << "] (string) paramId=" << current_ << std::endl;
    scope_->set(name, value);
}


void MIRConfiguration::store(const std::string& name, bool value) {
    ASSERT(scope_);
    eckit::Log::debug<LibMir>() << "From configuration file " << name << "=[" << value << "] (bool) paramId=" << current_ << std::endl;
    scope_->set(name, value);
}


void MIRConfiguration::store(const std::string& name, long value) {
    ASSERT(scope_);
    eckit::Log::debug<LibMir>() << "From configuration file " << name << "=[" << value << "] (long) paramId=" << current_ << std::endl;
    scope_->set(name, value);
}


void MIRConfiguration::store(const std::string& name, double value) {
    ASSERT(scope_);
    eckit::Log::debug<LibMir>() << "From configuration file " << name << "=[" << value << "] (double) paramId=" << current_ << std::endl;
    scope_->set(name, value);
}


void MIRConfiguration::scope(const std::string& name) {
    long scope = eckit::Translator<std::string, long>()(name);
    eckit::Log::debug<LibMir>() << "MIRConfiguration::scope(paramId=" << scope << ")" << std::endl;

    map_t::iterator j = settings_.find(scope);
    if (j == settings_.end()) {
        settings_[scope] = new SimpleParametrisation();
    }
    current_ = scope;
    scope_ = settings_[scope];
}


const SimpleParametrisation* MIRConfiguration::lookup(long paramId) const {
    map_t::const_iterator j = settings_.find(paramId);
    if(j == settings_.end()) {
        return 0;
    } else {
        return (*j).second;
    }
}


}  // namespace param
}  // namespace mir

