/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/param/Rules.h"

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/parser/JSON.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/utils/Translator.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace param {


static const std::string PARAM_ID("paramId");
static const std::string KLASS("@class");
static const std::string WARNING("warning");


Rules::Rules() = default;


Rules::~Rules() {
    for (auto& rule : rules_) {
        delete rule.second;
    }
}


SimpleParametrisation& Rules::lookup(long paramId) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    auto p = rules_.find(paramId);
    if (p == rules_.end()) {
        SimpleParametrisation* s = new SimpleParametrisation();
        rules_[paramId] = s;
        return *s;
    }

    return *(p->second);
}


const MIRParametrisation& Rules::lookup(const std::string& ruleName, long ruleValue) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(ruleName == PARAM_ID);

    MIRParametrisation& s = lookup(ruleValue);

    if (!s.has(KLASS)) {
        if (noted_.insert(ruleValue).second) {

            const std::string msg = "No class defined for " + ruleName + "=" + std::to_string(ruleValue);
            eckit::Log::warning() << msg << std::endl;

            static bool abortIfUnknownParameterClass = eckit::Resource<bool>("$MIR_ABORT_IF_UNKNOWN_PARAMETER_CLASS", false);
            if (abortIfUnknownParameterClass) {
                throw eckit::SeriousBug(msg);
            }

        }
    }

    if (warning_.find(ruleValue) != warning_.end()) {
        const std::string msg = "Warning: " + ruleName + "=" + std::to_string(ruleValue);
        eckit::Log::warning() << msg << std::endl;
    }

    return s;
}


void Rules::print(std::ostream& s) const {
    s << "Rules=";
    eckit::JSON json(s);

    json.startObject();
    for (const auto& rule : rules_) {
        json << rule.first << rule.second;
    }
    json.endObject();
}


void Rules::readConfigurationFiles() {

    eckit::Translator<std::string, long> translate_to_long;
    eckit::Translator<std::string, bool> translate_to_bool;

    warning_.clear();

    eckit::ValueMap classes = eckit::YAMLParser::decodeFile("~mir/etc/mir/classes.yaml");
    eckit::ValueMap parameterClass = eckit::YAMLParser::decodeFile("~mir/etc/mir/parameter-class.yaml");
    for (const auto& i : parameterClass) {

        // class
        const std::string& klass = i.first;
        const auto& config = classes.find(klass);
        if (config == classes.end()) {
            throw eckit::SeriousBug("Rules: unkown class '" + klass + "'");
        }
        const eckit::ValueMap klassConfig = config->second;

        // paramId(s)
        eckit::ValueList paramIds = i.second;
        for (long paramId : paramIds) {
            SimpleParametrisation& pidConfig = lookup(paramId);

            std::string klasses;
            klasses = klass + (pidConfig.get(KLASS, klasses) ? ", " + klasses : "");
            pidConfig.set(KLASS, klasses);

            for (const auto& j : klassConfig) {
                const std::string& keyName = j.first;
                const std::string& keyValue = j.second;

                ASSERT(keyName != KLASS);
                if (keyName == WARNING) {
                    if (translate_to_bool(keyValue)) {
                        warning_.insert(paramId);
                    }
                    continue;
                }

                if (static_cast<MIRParametrisation&>(pidConfig).has(keyName)) {
                    throw eckit::UserError("Rules: parameter " + std::to_string(paramId)
                                           + " has ambigous key '" + keyName + "'"
                                             " from classes " + klasses);
                }

                pidConfig.set(keyName, keyValue);
            }
        }
    }


    eckit::ValueMap parameters = eckit::YAMLParser::decodeFile("~mir/etc/mir/parameters.yaml");
    for (const auto& i : parameters) {

        long paramId = translate_to_long(i.first);
        SimpleParametrisation& config = Rules::lookup(paramId);

        eckit::ValueList options = i.second;
        for (const eckit::ValueMap j : options) {
            for (auto k : j) {
                std::string name = k.first;
                std::string value = k.second;
                config.set(name, value);
            }
        }
    }
}


}  // namespace param
}  // namespace mir

