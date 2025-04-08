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


#include <memory>

#include "mir/util/FormulaBinop.h"
#include "mir/util/FormulaFunction.h"
#include "mir/util/FormulaIdent.h"
#include "mir/util/FormulaNumber.h"
#include "mir/util/FormulaParser.h"
#include "mir/util/FormulaString.h"
#include "mir/util/Translator.h"


namespace mir::util {


FormulaParser::FormulaParser(std::istream& in) : StreamParser(in, true) {}


std::string FormulaParser::parseIdent(const param::MIRParametrisation& /*parametrisation*/) {
    std::string s;
    char c = peek();
    while (isalnum(c) != 0) {
        s += next();
        c = peek();
    }
    return s;
}


Formula* FormulaParser::parseNumber(const param::MIRParametrisation& parametrisation) {
    std::string s;

    while (isdigit(peek()) != 0) {
        s += next();
    }

    if (peek() == '.') {
        s += next();
        char c = next();
        if (isdigit(c) == 0) {
            throw StreamParser::Error(std::string("FormulaParser::parseNumber invalid char '") + c + "'");
        }
        s += c;
        while (isdigit(peek()) != 0) {
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

        if (isdigit(c) == 0) {
            throw StreamParser::Error(std::string("FormulaParser::parseNumber invalid char '") + c + "'");
        }
        s += c;
        while (isdigit(peek()) != 0) {
            s += next();
        }
    }

    return new FormulaNumber(parametrisation, from_string<double>(s));
}


Formula* FormulaParser::parseString(const param::MIRParametrisation& parametrisation) {
    char quote = peek();
    consume(quote);
    std::string s;
    for (;;) {
        char c = next(true);
        if (c == quote) {
            break;
        }
        s += c;
    }

    return new FormulaString(parametrisation, s);
}


Formula* FormulaParser::parseAtom(const param::MIRParametrisation& parametrisation) {
    std::unique_ptr<Formula> f;

    char c = peek();
    switch (c) {
        case '(':
            consume('(');
            f.reset(parseTest(parametrisation));
            consume(')');
            return f.release();
            // break;

        case '-':
            consume('-');
            return new FormulaFunction(parametrisation, "neg", parseAtom(parametrisation));
            // break;

        case '\'':
            return parseString(parametrisation);
            // break;

        case '"':
            return parseString(parametrisation);
            // break;

        default:
            if (isalpha(c) != 0 || c == '_') {
                std::string name = parseIdent(parametrisation);
                if (peek() == '(') {
                    std::vector<Formula*> args = parseList(parametrisation);
                    return new FormulaFunction(parametrisation, name, args);
                }
                return new FormulaIdent(parametrisation, name);
            }

            if (isdigit(c) != 0) {
                return parseNumber(parametrisation);
            }
            break;
    }

    throw StreamParser::Error(std::string("FormulaParser::parseAtom invalid char '") + c + "'");
}


Formula* FormulaParser::parsePower(const param::MIRParametrisation& parametrisation) {
    char name[2] = {
        0,
    };

    Formula* result = parseAtom(parametrisation);
    char c          = peek();
    while (c == '^' /*|| c == '*' */) {
        consume(c);
        name[0] = c;
        result  = new FormulaBinop(parametrisation, name, result, parseAtom(parametrisation));
        c       = peek();
    }

    return result;
}


std::vector<Formula*> FormulaParser::parseList(const param::MIRParametrisation& parametrisation) {
    std::vector<Formula*> v;
    consume('(');
    while (peek() != ')') {
        v.push_back(parseTest(parametrisation));
        if (peek() == ')') {
            break;
        }
        consume(',');
    }
    consume(')');
    return v;
}


Formula* FormulaParser::parseFactor(const param::MIRParametrisation& parametrisation) {
    char name[2] = {
        0,
    };

    Formula* result = parsePower(parametrisation);
    char c          = peek();
    while (c == '*' || c == '/') {
        consume(c);
        name[0] = c;
        result  = new FormulaBinop(parametrisation, name, result, parsePower(parametrisation));
        c       = peek();
    }
    return result;
}


Formula* FormulaParser::parseTerm(const param::MIRParametrisation& parametrisation) {
    char name[2] = {
        0,
    };

    Formula* result = parseFactor(parametrisation);
    char c          = peek();
    while (c == '+' || c == '-') {
        consume(c);
        name[0] = c;
        result  = new FormulaBinop(parametrisation, name, result, parseFactor(parametrisation));
        c       = peek();
    }
    return result;
}


Formula* FormulaParser::parseTest(const param::MIRParametrisation& parametrisation) {
    char name[3] = {
        0,
    };

    Formula* result = parseTerm(parametrisation);
    char c          = peek();
    while (c == '<' || c == '>' || c == '=') {
        consume(c);
        name[0] = c;
        name[1] = 0;

        c = peek();
        if (c == '=' || c == '>') {
            consume(c);
            name[1] = c;
        }

        result = new FormulaFunction(parametrisation, name, result, parseTerm(parametrisation));
        c      = peek();
    }
    return result;
}


Formula* FormulaParser::parse(const param::MIRParametrisation& parametrisation) {
    std::unique_ptr<Formula> f(parseTest(parametrisation));
    char c = peek();
    if (c != 0) {
        throw StreamParser::Error(std::string("Error parsing rules: remaining char: ") + c);
    }
    return f.release();
}


}  // namespace mir::util
