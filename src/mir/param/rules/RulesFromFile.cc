/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/param/rules/RulesFromFile.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/parser/YAMLParser.h"


namespace mir {
namespace param {
namespace rules {


const RulesFromFile& RulesFromFile::instance() {
    static RulesFromFile ego;
    return ego;
}


RulesFromFile::RulesFromFile() {

    std::map< std::string, eckit::SharedPtr<CountedParametrisation> > allClasses;


    eckit::ValueMap classes = eckit::YAMLParser::decodeFile("~mir/etc/mir/classes.yaml");
    for (const auto& i : classes) {
        const std::string& klass = i.first;
        eckit::ValueMap values = i.second;

        if (values.find("dimension") != values.end()) {
            throw eckit::UserError("Class cannot use reserved key 'dimension' ('" + klass + "')");
        }

        CountedParametrisation* s = new CountedParametrisation();

        for (const auto& j : values) {
            std::string name = j.first;
            eckit::Value value = j.second;
            s->set(name, std::string(value));
        }

        allClasses[klass].reset(s);
    }


    eckit::ValueMap parameterClass = eckit::YAMLParser::decodeFile("~mir/etc/mir/parameter-class.yaml");
    for (const auto& i : parameterClass) {
        const std::string& klass = i.first;

        auto j = allClasses.find(klass);
        if (j == allClasses.end()) {
            throw eckit::UserError("Class unknown '" + klass + "'");
        }

        eckit::ValueList list = i.second;
        // std::cout << list << std::endl;

        for (long paramId : list) {
            if (find(paramId) != end()) {
                throw eckit::UserError("Parameter class is set more than once, for paramId=" + std::to_string(paramId));
            }
            container_t::operator[](paramId).reset(j->second);
        }
    }


    eckit::ValueMap parameterDimension = eckit::YAMLParser::decodeFile("~mir/etc/mir/parameter-dimension.yaml");
    for (const auto& i : parameterDimension) {
        const std::string& dimension = i.first;
        eckit::ValueList list = i.second;

        for (long paramId : list) {

            // paramId-specific 'dimension' value
            CountedParametrisation* s = new CountedParametrisation();
            s->set("dimension", dimension);

            auto p = find(paramId);
            if (p != end()) {

                // known parameter: copy class settings to new entry
                std::string d;
                if (p->second->get("dimension", d)) {
                    throw eckit::UserError("Parameter dimension is set more than once, for paramId=" + std::to_string(paramId));
                }
                p->second->copyValuesTo(*s);
                p->second.reset(s);

            } else {

                // unknown parameter: set dimension only
                container_t::operator[](paramId).reset(s);

            }
        }
    }
}


}  // namespace rules
}  // namespace param
}  // namespace mir

