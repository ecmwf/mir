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


#include "mir/param/Rules.h"

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/JSON.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/utils/Translator.h"

#include "mir/config/LibMir.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace param {


static const std::string PARAM_ID("paramId");
static const std::string KLASS("_class");
static const std::string WARNING("_warning");
static const std::string DEFAULT("_default");


Rules::Rules() {
    readConfigurationFiles();
}


Rules::~Rules() {
    for (auto& rule : rules_) {
        delete rule.second;
    }
}


const MIRParametrisation* Rules::find(const MIRParametrisation& param) const {
    long paramId = 0;
    if (param.get(PARAM_ID, paramId) && 0 < paramId) {
        auto p = rules_.find(paramId);
        if (p != rules_.end()) {
            return p->second;
        }
    }

    return nullptr;
}


SimpleParametrisation& Rules::lookup(long paramId) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    auto p = rules_.find(paramId);
    if (p != rules_.end()) {
        return *(p->second);
    }

    return *(rules_[paramId] = new SimpleParametrisation());
}


const MIRParametrisation& Rules::lookup(const std::string& ruleName, long ruleValue) {
    ASSERT(ruleName == PARAM_ID);

    MIRParametrisation& s = lookup(ruleValue);

    auto msg = [&]() -> std::string { return ruleName + "=" + std::to_string(ruleValue) + ": "; };

    auto w = warning_.find(ruleValue);
    if (w != warning_.end()) {
        warning_.erase(w);
        Log::warning() << "Warning: " << msg() << "post-processing defaults might not be appropriate" << std::endl;
        return s;
    }

    if (!s.has(KLASS) && noted_.insert(ruleValue).second) {
        std::string m = msg() + "no class defined";

        static bool abortIfUnknownParameterClass =
            eckit::Resource<bool>("$MIR_ABORT_IF_UNKNOWN_PARAMETER_CLASS", false);
        if (abortIfUnknownParameterClass) {
            Log::error() << m << std::endl;
            throw exception::UserError(m);
        }

        Log::warning() << "Warning: " << m << std::endl;
    }

    return s;
}


void Rules::print(std::ostream& s) const {
    s << "Rules=";
    eckit::JSON json(s);

    json.startObject();
    for (const auto& rule : rules_) {
        json << rule.first << (*rule.second);
    }
    json.endObject();
}


void Rules::readConfigurationFiles() {
    eckit::Translator<std::string, long> translate_to_long;
    eckit::Translator<std::string, bool> translate_to_bool;

    warning_.clear();

    struct ConfigFile : private eckit::PathName {
        eckit::ValueMap map() { return eckit::YAMLParser::decodeFile(static_cast<PathName>(*this)); }
        using PathName::exists;
        using PathName::PathName;
    };

    ConfigFile classes(LibMir::configFile(LibMir::config_file::CLASSES));
    ConfigFile parameterClass(LibMir::configFile(LibMir::config_file::PARAMETER_CLASS));
    ConfigFile parameters(LibMir::configFile(LibMir::config_file::PARAMETERS));

    if (!classes.exists() || !parameterClass.exists() || !parameters.exists()) {

        const std::string msg =
            "Configuration files not found,"
            " post-processing defaults might not be appropriate";

        static bool abortIfConfigurationFilesNotFound =
            eckit::Resource<bool>("$MIR_ABORT_IF_CONFIGURATION_NOT_FOUND", false);
        if (abortIfConfigurationFilesNotFound) {
            Log::error() << msg << std::endl;
            throw exception::UserError(msg);
        }

        Log::warning() << msg << std::endl;
        return;
    }


    const auto classesMap = classes.map();
    for (const auto& i : parameterClass.map()) {

        // class
        const std::string& klass = i.first;
        ASSERT(klass != DEFAULT);

        const auto& config = classesMap.find(klass);
        if (config == classesMap.end()) {
            throw exception::UserError("Rules: unkown class '" + klass + "'");
        }
        const eckit::ValueMap& klassConfig = config->second;

        // paramId(s)
        const eckit::ValueList& paramIds = i.second;
        for (long paramId : paramIds) {
            SimpleParametrisation& pidConfig = lookup(paramId);

            std::string klasses;
            klasses = klass + (pidConfig.get(KLASS, klasses) ? ", " + klasses : "");
            pidConfig.set(KLASS, klasses);

            for (const auto& j : klassConfig) {
                const std::string& keyName  = j.first;
                const std::string& keyValue = j.second;

                ASSERT(keyName != KLASS);
                if (keyName == WARNING) {
                    if (translate_to_bool(keyValue)) {
                        warning_.insert(paramId);
                    }
                    continue;
                }

                if (pidConfig.has(keyName)) {
                    throw exception::UserError("Rules: parameter " + std::to_string(paramId) + " has ambigous key '" +
                                               keyName +
                                               "'"
                                               " from classes " +
                                               klasses);
                }

                pidConfig.set(keyName, keyValue);
            }
        }
    }


    const auto defaults = classesMap.find(DEFAULT);
    if (defaults != classesMap.end()) {
        const eckit::ValueMap& defaultConfig = defaults->second;
        for (auto p : rules_) {
            ASSERT(p.second);
            SimpleParametrisation& pidConfig = *(p.second);

            for (const auto& j : defaultConfig) {
                const std::string& name  = j.first;
                const std::string& value = j.second;

                if (!pidConfig.has(name)) {
                    pidConfig.set(name, value);
                }
            }
        }
    }


    for (const auto& i : parameters.map()) {
        long paramId                  = translate_to_long(i.first);
        SimpleParametrisation& config = lookup(paramId);

        const eckit::ValueList& options = i.second;
        for (const auto& j : options) {
            for (const auto& k : eckit::ValueMap(j)) {
                const std::string& name  = k.first;
                const std::string& value = k.second;
                config.set(name, value);
            }
        }
    }
}


}  // namespace param
}  // namespace mir
