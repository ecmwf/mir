/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   PlanParser.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016


#include "mir/util/PlanParser.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/style/CustomParametrisation.h"


namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------

PlanParser::PlanParser(std::istream &in) : StreamParser(in, true) {
}

void PlanParser::parse(action::ActionPlan &plan, const
                       param::MIRParametrisation &parametrisation) {

    char c = peek();
    while (c != 0) {
        parseAction(plan, parametrisation);
        c = peek();
        if (c == ';') {
            consume(c);
        }

    }

}

void PlanParser::parseAction(action::ActionPlan &plan, const
                             param::MIRParametrisation &parametrisation) {

    std::string name = parseToken();
    plan.add(name, new style::CustomParametrisation(name,
             parseArguments(name),
             parametrisation));

}

std::string PlanParser::parseToken() {
    std::string s;
    char c = peek();
    while (c != '(' && c != ')' && c != '=' && c != ',' && c != '[' && c != ']') {
        s += next();
        c = peek();
    }
    return s;
}

std::map<std::string, std::vector<std::string> > PlanParser::parseArguments(const std::string& action) {
    std::map<std::string, std::vector<std::string> > result;

    char c = peek();
    if (c == '(') {
        consume(c);
        c = peek();
        while (c != ')') {


            std::string name = parseToken();
            c = peek();
            if (c == '=') {
                consume('=');

                c = peek();
                if (c == '[') {
                    result[name] = parseValues();
                }
                else {
                    result[name].push_back(parseToken());
                }
            } else {
                // Implicty name is action
                // e.g. frame(1) same as frame(frame=1)
                result[action].push_back(name);
            }
            c = peek();
            if (c == ',') {
                consume(c);
            }
        }
        consume(')');
    }


    return result;
}


std::vector<std::string> PlanParser::parseValues() {
    std::vector<std::string> result;

    consume('[');

    char c = peek();
    while (c != ']') {


        result.push_back(parseToken());


        c = peek();
        if (c == ',') {
            consume(c);
        }
    }

    consume(']');


    return result;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace util
} // namespace mir
