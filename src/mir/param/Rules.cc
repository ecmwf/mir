/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/param/Rules.h"
#include "mir/param/SimpleParametrisation.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/parser/JSON.h"
#include "eckit/parser/YAMLParser.h"


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

    auto p = rules_.find(paramId);
    if (p == rules_.end()) {
        SimpleParametrisation* s = new SimpleParametrisation();
        rules_[paramId] = s;
        return *s;
    }

    return *(p->second);
}


const MIRParametrisation& Rules::lookup(const std::string& ruleName, long ruleValue) {
    ASSERT(ruleName == PARAM_ID);
    return lookup(ruleValue);
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
            std::cout << what << " " << paramId << " " << s << std::endl;

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

}


}  // namespace param
}  // namespace mir

