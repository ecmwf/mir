/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/function/FunctionParser.h"

#include "eckit/memory/ScopedPtr.h"
#include "eckit/types/Types.h"
#include "eckit/utils/Translator.h"
#include "mir/util/function/Function.h"


namespace mir {
namespace util {
namespace function {


FunctionParser::FunctionParser(std::istream& in) : StreamParser(in) {
}


std::string FunctionParser::parseIdent() {
    std::string s;
    char c = peek();
    while (isalnum(c)) {
        s += next();
        c = peek();
    }
    return s;
}


Function* FunctionParser::parseNumber() {
    std::string s;

    while (isdigit(peek())) {
        s += next();
    }

    if (peek() == '.') {
        s += next();
        char c = next();
        if (!isdigit(c)) {
            throw StreamParser::Error(std::string("FunctionParser::parseNumber invalid char '") + c + "'");
        }
        s += c;
        while (isdigit(peek())) {
            s += next();
        }
    }

    char c = peek();
    if (c == 'e' || c == 'E') {
        s += next();

        c = next();
        if (c == '-' || c == '+') {
            s += c;
            c = next();
        }

        if (!isdigit(c)) {
            throw StreamParser::Error(std::string("FunctionParser::parseNumber invalid char '") + c + "'");
        }
        s += c;
        while (isdigit(peek())) {
            s += next();
        }
    }

    eckit::Translator<std::string, double> s2d;
    return new Constant(s2d(s));
}


Function* FunctionParser::parseAtom() {
    eckit::ScopedPtr<Function> f;

    char c = peek();
    switch (c) {
    case '(': {
        consume('(');
        f.reset(parseTest());
        consume(')');
        return f.release();
    }

    case '-': {
        consume('-');
        Function* arg = parseAtom();
        return FunctionFactory::build("neg", {arg});
    }

    default:
        if (isalpha(c) || c == '_') {
            std::string name = parseIdent();
            if (peek() == '(') {
                std::vector<Function*> args = parseList();
                return FunctionFactory::build(name, args);
            } else {
                return new Variable(name);
            }
        } else if (isdigit(c)) {
            return parseNumber();
        }
    }

    throw StreamParser::Error(std::string("FunctionParser::parseAtom invalid char '") + c + "'");
}


Function* FunctionParser::parsePower() {
    char name[2] = {
        0,
    };

    Function* result = parseAtom();
    char c = peek();
    while (c == '^' /*|| c == '*' */) {
        consume(c);
        name[0] = c;
        Function* arg = parseAtom();
        result = FunctionFactory::build(name, {result, arg});
        c = peek();
    }

    return result;
}


std::vector<Function*> FunctionParser::parseList() {
    std::vector<Function*> v;
    consume('(');
    while (peek() != ')') {
        v.push_back(parseTest());
        if (peek() == ')') {
            break;
        }
        consume(',');
    }
    consume(')');
    return v;
}


Function* FunctionParser::parseFactor() {
    char name[2] = {
        0,
    };

    Function* result = parsePower();
    char c = peek();
    while (c == '*' || c == '/') {
        consume(c);
        name[0] = c;
        Function* arg = parsePower();
        result = FunctionFactory::build(name, {result, arg});
        c = peek();
    }
    return result;
}


Function* FunctionParser::parseTerm() {
    char name[2] = {
        0,
    };

    Function* result = parseFactor();
    char c = peek();
    while (c == '+' || c == '-') {
        consume(c);
        name[0] = c;
        Function* arg = parseFactor();
        result = FunctionFactory::build(name, {result, arg});
        c = peek();
    }
    return result;
}


Function* FunctionParser::parseTest() {
    char name[3] = {
        0,
    };

    Function* result = parseTerm();
    char c = peek();
    while (c == '<' || c == '>' || c == '=') {
        consume(c);
        name[0] = c;
        name[1] = 0;

        c = peek();
        if (c == '=' || c == '>') {
            consume(c);
            name[1] = c;
        }

        Function* arg = parseTerm();
        result = FunctionFactory::build(name, {result, arg});
        c = peek();
    }
    return result;
}


Function* FunctionParser::parse() {
    eckit::ScopedPtr<Function> f(parseTest());
    char c;
    if ((c = peek())) {
        throw StreamParser::Error(std::string("Error parsing rules: remaining char: ") + c);
    }
    return f.release();
}


}  // namespace function
}  // namespace util
}  // namespace mir
