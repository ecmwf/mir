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

#include <algorithm>
#include <iostream>
#include <vector>
#include "eckit/filesystem/PathName.h"
#include "eckit/parser/StringTools.h"
#include "eckit/utils/Translator.h"
#include "mir/config/LibMir.h"
#include "mir/param/ParamClass.h"
#include "mir/util/Parser.h"


namespace mir {
namespace param {


namespace  {


// fill params (recursively) with settings from settingsParams[setting]
bool flatten_params_on_setting(const ParamClass& settingsParams, const std::string& setting, const std::string& defaultSetting, const std::string& trackSetting, const size_t& recurseLevelMax, SimpleParametrisation& params) {
    ASSERT(setting.length());

    // track recursivity depth and values (to detect loops)
    std::vector< std::string > recurse;

    if (settingsParams.has(defaultSetting)) {
        settingsParams.lookup(defaultSetting)->copyValuesTo(params, false);
    }

    while (static_cast< const MIRParametrisation& >(params).has(setting)) {

        // get the setting value
        std::string value;
        static_cast< const MIRParametrisation& >(params).get(setting, value);
        ASSERT(value.length());

        // increase recursivity depth
        if (std::find(recurse.begin(), recurse.end(), value) != recurse.end()) {
            // all settings copied already, an "elegant" way to not loop...
            eckit::Log::warning() << "flatten_on_setting: recursion loop on \"" << setting << " = " << value << "\"";
            return true;
        }
        if (recurse.size() >= recurseLevelMax) {
            eckit::Log::error() << "flatten_on_setting: recursion exceeds maximum depth (" << recurseLevelMax << ")";
            return false;
        }
        recurse.push_back(value);

        // remove recursed parameter
        params.clear(setting);

        // copy the parameter-specific settings (exclude the "flattening" setting, and don't overwrite)
        settingsParams.lookup(value)->copyValuesTo(params, false);

    }

    // track the "flattened" setting
    if (recurse.size() && trackSetting.length()) {
        ASSERT(setting != trackSetting);
        const std::string track = eckit::StringTools::join(", ", recurse);
        params.set(trackSetting, track);
        eckit::Log::debug<LibMir>() << "flatten_on_setting: recurse: \"" << track << "\"" << std::endl;
    }

    return true;
}


}  // (anonymous namespace)


MIRConfiguration::MIRConfiguration() :
    scope_(0),
    flattenSetting_("class"),
    flattenDefaultSetting_("class.default"),
    flattenTrackSetting_("class.debug"),
    flattenDepth_(3) {

    eckit::PathName path("~mir/etc/mir/param-info.cfg");
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
    if (j == settings_.end()) {
        return 0;
    }

    if (!flatten_params_on_setting(ParamClass::instance(), flattenSetting_, flattenDefaultSetting_, flattenTrackSetting_, flattenDepth_, *(j->second))) {
        throw eckit::UserError("Cannot flatten paramId=" + eckit::Translator< long, std::string >()(j->first) + " on \"class\"", Here());
    }
    eckit::Log::debug<LibMir>() << "MIRConfiguration paramId=" << j->first << ": ["  << *(j->second) << "]" << std::endl;

    return (*j).second;
}


}  // namespace param
}  // namespace mir

