/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Nov 2016


#include "mir/config/MIRConfiguration.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/parser/YAMLParser.h"
#include "mir/config/LibMir.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace config {


namespace  {
static struct Defaults : param::SimpleParametrisation {
    Defaults() {
        // these options are (can be) overridden by the configuration file
        set("paramId.u", 131);
        set("paramId.v", 132);

        set("parameter-configuration", "~mir/etc/mir/parameter.yaml");

        set("mir-cache-path", std::vector<std::string>({"mir-cache-path;$MIR_CACHE_PATH", "/tmp/cache"}));
    }
} defaults;
}  // (anonymous namespace)


MIRConfiguration& MIRConfiguration::instance() {
    static MIRConfiguration instance_;
    return instance_;
}


MIRConfiguration::MIRConfiguration() {

    // Always start with internal defaults, not from file
    configure("");
}


const param::MIRParametrisation& MIRConfiguration::lookup(const param::MIRParametrisation& metadata) const {
    long id = 0;
    metadata.get("paramId", id);
    return lookup(id, metadata);
}


const param::MIRParametrisation& MIRConfiguration::lookup(const long& paramId, const param::MIRParametrisation& metadata) const {
    return parameterConfiguration_->lookup(paramId, metadata);
}


void MIRConfiguration::configure(const eckit::PathName& path) {

    // configure from file, skip if this was already done
    if (!path.asString().empty() && path.asString() != configPath_) {
        eckit::Log::debug<LibMir>() << "MIRConfiguration: loading configuration from '" << path << "'" << std::endl;
        std::ifstream in(path.asString().c_str());
        if (!in) {
            throw eckit::CantOpenFile(path);
        }

        // Create hierarchy (using non-overwriting filling keys)
        eckit::YAMLParser parser(in);
        const eckit::ValueMap j = parser.parse();
        root_.fill(j);
    }


    configPath_ = path;
    //    eckit::Log::debug<LibMir>() << "MIRConfiguration: " << root_ << std::endl;

    // Use defaults (non-overwriting)
    defaults.copyValuesTo(root_, false);
}


bool MIRConfiguration::has(const std::string& name) const {
    //FIXME
    return true;
}


bool MIRConfiguration::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}


bool MIRConfiguration::get(const std::string& name, bool& value) const {
    return _get(name, value);
}


bool MIRConfiguration::get(const std::string& name, int& value) const {
    return _get(name, value);
}


bool MIRConfiguration::get(const std::string& name, long& value) const {
    return _get(name, value);
}


bool MIRConfiguration::get(const std::string& name, float& value) const {
    return _get(name, value);
}


bool MIRConfiguration::get(const std::string& name, double& value) const {
    return _get(name, value);
}


bool MIRConfiguration::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}


bool MIRConfiguration::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}


bool MIRConfiguration::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}


bool MIRConfiguration::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}


bool MIRConfiguration::get(const std::string&, std::vector<std::string>&) const {
    NOTIMP;
}


template<class T>
bool MIRConfiguration::_get(const std::string& name, T& value) const {
//    SettingsMap::const_iterator j = settings_.find(name);
//    if (j == settings_.end()) {
//        return false;
//    }
//    (*j).second->get(name, value);
    // eckit::Log::debug<LibMir>() << "SimpleParametrisation::get(" << name << ") => " << value << std::endl;
    return true;
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration["
        <<  "configPath=" << configPath_
        << ",root=" << root_
        << "]";
}


}  // namespace config
}  // namespace mir

