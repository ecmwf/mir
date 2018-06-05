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


static std::string PARAM_ID("paramId");


Rules::Rules() {
}


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

    if (!s.has("class")) {
        if (noted_.find(ruleValue) == noted_.end()) {

            const std::string msg = "No class defined for " + ruleName + "=" + std::to_string(ruleValue);

            static bool abortIfUnknownParameterClass = eckit::Resource<bool>("$MIR_ABORT_IF_UNKNOWN_PARAMETER_CLASS", false);
            if (abortIfUnknownParameterClass) {
                throw eckit::SeriousBug(msg);
            } else {
                eckit::Log::warning() << msg << std::endl;
            }
        }

        noted_.insert(ruleValue);
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

    eckit::ValueMap classes = eckit::YAMLParser::decodeFile("~mir/etc/mir/classes.yaml");
    eckit::ValueMap parameterClass = eckit::YAMLParser::decodeFile("~mir/etc/mir/parameter-class.yaml");
    for (const auto& i : parameterClass) {

        // class
        const std::string& klassName = i.first;
        const auto& config = classes.find(klassName);
        if (config == classes.end()) {
            throw eckit::SeriousBug("Rules: unkown class '" + klassName + "'");
        }
        const eckit::ValueMap klassConfig = config->second;

        // paramId(s)
        eckit::ValueList paramIds = i.second;
        for (long paramId : paramIds) {
            SimpleParametrisation& pidConfig = lookup(paramId);

            for (const auto& j : klassConfig) {
                const std::string& keyName = j.first;
                const std::string& keyValue = j.second;

                if (static_cast<MIRParametrisation&>(pidConfig).has(keyName)) {
                    throw eckit::SeriousBug("Rules: parameter " + std::to_string(paramId)
                                            + " already has key '" + keyName
                                            + "' when setting class '"
                                            + klassName + "'");
                }
                pidConfig.set(keyName, keyValue);
            }
        }
    }


    eckit::ValueMap parameters = eckit::YAMLParser::decodeFile("~mir/etc/mir/parameters.yaml");
    for (const auto& i : parameters) {

        long paramId = eckit::Translator<std::string, long>()(i.first);
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

