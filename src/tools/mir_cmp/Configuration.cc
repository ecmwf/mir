/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#include "tools/mir_cmp/Configuration.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include "eckit/filesystem/PathName.h"
#include "eckit/utils/Translator.h"
#include "mir/config/LibMir.h"
#include "mir/param/ParamClass.h"
#include "mir/util/Parser.h"


using namespace mir;


namespace mir_cmp {


Configuration::Configuration() :
    current_(0),
    scope_(0),
    flattenSetting_("key"),
    flattenDepth_(3) {

    eckit::PathName path("~mir/etc/mir/mir_cmp.cfg");
    eckit::Log::debug<mir::LibMir>() << "Loading configuration from " << path << std::endl;
    if (!path.exists()) {
        eckit::Log::warning() << "Cannot load " << path << std::endl;
        throw eckit::CantOpenFile(path, Here());
    }

    util::Parser parser(path);
    parser.fill(*this);
}


Configuration::~Configuration() {
    for (map_t::iterator j = settings_.begin(); j != settings_.end(); ++j) {
        delete (*j).second;
    }
}


const Configuration& Configuration::instance() {
    static Configuration instance_;
    return instance_;
}


void Configuration::print(std::ostream& out) const {
    out << "Configuration[...]";
}


void Configuration::store(const std::string& name, const char* value) {
    store(name, std::string(value));
}


void Configuration::store(const std::string& name, const std::string& value) {
    ASSERT(scope_);
    scope_->set(name, value);
}


void Configuration::store(const std::string& name, bool value) {
    ASSERT(scope_);
    scope_->set(name, value);
}


void Configuration::store(const std::string& name, long value) {
    ASSERT(scope_);
    scope_->set(name, value);
}


void Configuration::store(const std::string& name, double value) {
    ASSERT(scope_);
    scope_->set(name, value);
}


void Configuration::scope(const std::string& name) {
    long scope = eckit::Translator<std::string, long>()(name);

    map_t::iterator j = settings_.find(scope);
    if (j == settings_.end()) {
        settings_[scope] = new param_t();
    }
    current_ = scope;
    scope_ = settings_[scope];

#if 0
    const std::string scope = name;
    eckit::Log::debug<LibMir>() << "ParamClass::scope(paramClass=" << scope << ")" << std::endl;

    map_t::iterator j = settings_.find(scope);
    if (j == settings_.end()) {
        settings_[scope] = new param_t();
    }
    current_ = scope;
    scope_ = settings_[scope];
#endif
}


const Configuration::param_t* Configuration::lookup(long paramId) const {
    map_t::const_iterator j = settings_.find(paramId);
    if (j == settings_.end()) {
        return 0;
    }

#if 0
    if (!flatten_params_on_setting(ParamClass::instance(), flattenSetting_, flattenDepth_, *(j->second))) {
        throw eckit::UserError("Cannot flatten paramId=" + eckit::Translator< long, std::string >()(j->first) + " on \"class\"", Here());
    }

    // fill params (recursively) with settings from settingsParams[setting]
    bool flatten_params_on_setting(const ParamClass& settingsParams, const std::string& setting, const size_t& recurseLevelMax, param_t& params) {
        ASSERT(setting.length());

        // track recursivity depth and values (to detect loops)
        std::vector< std::string > recurse;

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

            // copy the non-parameter-specific parametrization
            param_t paramsModif;
            settingsParams.lookup(value)->copyValuesTo(paramsModif);

            // overwrite with parameter-specific settings (exclude the "flattening" setting)
            params.clear(setting);
            params.copyValuesTo(paramsModif);
            paramsModif.copyValuesTo(params);

        }

        return true;
    }
#endif
    eckit::Log::debug<LibMir>() << "Configuration paramId=" << j->first << ": ["  << *(j->second) << "]" << std::endl;

    return (*j).second;
}


}  // namespace mir_cmp
