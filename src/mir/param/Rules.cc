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
#include "mir/param/SimpleParametrisation.h"

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/parser/JSON.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/utils/Translator.h"


namespace mir {
namespace param {


static std::string PARAM_ID("paramId");


Rules::Rules() {
}


Rules::~Rules() {
    for ( auto& rule : rules_) {
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

            static bool abortIfUnknownParameterClass = eckit::Resource<bool>("$MIR_ABORT_IF_UNKNOWN_PARAMETER_CLASS", false);


            std::ostringstream oss;
            oss << "No class defined for " << ruleName << "=" << ruleValue;

            if (abortIfUnknownParameterClass) {
                throw eckit::SeriousBug(oss.str());
            }
            else {
                eckit::Log::warning() << oss.str() << std::endl;
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


void Rules::load(const std::string& kind, const std::string& path) {


    eckit::ValueMap config = eckit::YAMLParser::decodeFile(path);
    for (const auto& i : config) {
        const std::string& what = i.first;
        eckit::ValueList list = i.second;

        // std::cout << what << " " << list << std::endl;

        for (long paramId : list) {

            SimpleParametrisation& s = lookup(paramId);

            std::string d;
            if (s.get(kind, d)) {

                std::ostringstream oss;
                oss << "Duplicate [" << kind << "] for parameter " << paramId << " " << d << " and " << what;
                throw eckit::SeriousBug(oss.str());
            }

            s.set(kind, what);
        }
    }
}


void Rules::readConfigurationFiles() {


    eckit::ValueMap classes = eckit::YAMLParser::decodeFile("~mir/etc/mir/classes.yaml");


    load("class", "~mir/etc/mir/parameter-class.yaml");
    load("dimension", "~mir/etc/mir/parameter-dimension.yaml");

    for (auto& rule : rules_) {

        std::string klass;
        if (rule.second->get("class", klass)) {

            const auto& config = classes.find(klass);
            if (config == classes.end()) {
                std::ostringstream oss;
                oss << "Unknown parameter class [" << klass << "] for parameter " << rule.first;
                throw eckit::SeriousBug(oss.str());
            }

            eckit::ValueMap values = config->second;

            for (const auto& j : values) {
                std::string name = j.first;
                std::string value = j.second;
                rule.second->set(name, value); // TODO: implement set() with a value
            }
        }

    }



    eckit::ValueMap parameters = eckit::YAMLParser::decodeFile("~mir/etc/mir/parameters.yaml");
    for (const auto& i : parameters) {
        eckit::ValueList options = i.second;

        long paramId = eckit::Translator<std::string, long>()(i.first);
        SimpleParametrisation& config = Rules::lookup(paramId);

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

