/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   FormulaParser.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016


#include "mir/util/FormulaParser.h"
#include "mir/util/FormulaNumber.h"
#include "eckit/utils/Translator.h"
#include "mir/util/FormulaIdent.h"
#include "mir/util/FormulaString.h"
#include "mir/util/FormulaFunction.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/util/FormulaBinop.h"


namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------

FormulaParser::FormulaParser(std::istream &in) : StreamParser(in, true) {
}

std::string FormulaParser::parseIdent() {
    std::string s;
    char c = peek();
    while (isalnum(c)) {
        s += next();
        c = peek();
    }
    return s;
}


Formula* FormulaParser::parseNumber()
{
    std::string s;

    while (isdigit(peek())) {
        s += next();
    }

    if (peek() == '.') {
        s += next();
        char c = next();
        if (!isdigit(c)) {
            throw StreamParser::Error(std::string("FormulaParser::parseNumber invalid char '") + c + "'");
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
        if (c == '-' || c == '+')
        {
            s += c;
            c = next();
        }

        if (!isdigit(c)) {
            throw StreamParser::Error(std::string("FormulaParser::parseNumber invalid char '") + c + "'");
        }
        s += c;
        while (isdigit(peek())) {
            s += next();
        }

    }

    eckit::Translator<std::string, double> s2d;
    return new FormulaNumber(s2d(s));
}

Formula* FormulaParser::parseString()
{
    char quote = peek();
    consume(quote);
    std::string s;
    for (;;)
    {
        char c = next(true);
        if (c == quote) {
            break;
        }
        s += c;
    }

    return new FormulaString(s);
}

Formula* FormulaParser::parseAtom()
{
    eckit::ScopedPtr<Formula> f;

    char c = peek();
    switch (c)
    {
    case '(':
        consume('(');
        f.reset(parseTest());
        consume(')');
        return f.release();
        break;

    case '-':
        consume('-');
        return new FormulaFunction("neg", parseAtom());
        break;

    case '\'':
        return parseString();
        break;

    case '"':
        return parseString();
        break;

    default:
        if (isalpha(c) || c == '_') {
            std::string name = parseIdent();
            if (peek() == '(') {
                std::vector<Formula*> args = parseList();
                return new FormulaFunction(name, args);
            }
            else {
                return new FormulaIdent(name);
            }
        } else if (isdigit(c)) {
            return parseNumber();
        }
        else {
            throw StreamParser::Error(std::string("FormulaParser::parseAtom invalid  char '") + c + "'");
        }
        break;

    }
}

Formula* FormulaParser::parsePower()
{
    char name[2] = {0,};

    Formula* result = parseAtom();
    char c = peek();
    while (c == '^' /*|| c == '*' */ )
    {
        consume(c);
        name[0] = c;
        result = new FormulaBinop(name,
            result,
            parseAtom());
        c = peek();
    }

    return result;
}

std::vector<Formula*> FormulaParser::parseList() {
    std::vector<Formula*> v;
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


Formula* FormulaParser::parseFactor()
{
    char name[2] = {0,};

    Formula* result = parsePower();
    char c = peek();
    while (c == '*' || c == '/')
    {
        consume(c);
        name[0] = c;
        result = new FormulaBinop(name,
                                     result,
                                     parsePower());
        c = peek();
    }
    return result;
}

Formula* FormulaParser::parseTerm()
{   char name[2] = {0,};

    Formula* result = parseFactor();
    char c = peek();
    while (c == '+' || c == '-')
    {
        consume(c);
        name[0] = c;
        result = new FormulaBinop(name,
                                     result,
                                     parseFactor());
        c = peek();
    }
    return result;
}

Formula* FormulaParser::parseTest()
{
    char name[3] = {0,};

    Formula* result = parseTerm();
    char c = peek();
    while (c == '<' || c == '>' || c == '=')
    {
        consume(c);
        name[0] = c;
        name[1] = 0;

        c = peek();
        if (c == '=' || c == '>')
        {
            consume(c);
            name[1] = c;
        }

        result = new FormulaFunction(name,
                                     result,
                                     parseTerm());
        c = peek();
    }
    return result;
}

Formula* FormulaParser::parse() {
    eckit::ScopedPtr<Formula> f(parseTest());
    char c;
    if ((c = peek())) {
        throw StreamParser::Error(std::string("Error parsing rules: remaining char: ") + c);
    }
    return f.release();
}


//----------------------------------------------------------------------------------------------------------------------

} // namespace util
} // namespace mir
