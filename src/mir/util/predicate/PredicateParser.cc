/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/predicate/PredicateParser.h"

#include "eckit/memory/ScopedPtr.h"
#include "eckit/utils/Translator.h"
#include "mir/util/predicate/PredicateBinop.h"
#include "mir/util/predicate/PredicateFunction.h"
#include "mir/util/predicate/PredicateIdent.h"
#include "mir/util/predicate/PredicateNumber.h"
#include "mir/util/predicate/PredicateString.h"


namespace mir {
namespace util {
namespace predicate {


PredicateParser::PredicateParser(std::istream &in) : StreamParser(in, true) {
}


std::string PredicateParser::parseIdent() {
    std::string s;
    char c = peek();
    while (isalnum(c)) {
        s += next();
        c = peek();
    }
    return s;
}


Predicate* PredicateParser::parseNumber()
{
    std::string s;

    while (isdigit(peek())) {
        s += next();
    }

    if (peek() == '.') {
        s += next();
        char c = next();
        if (!isdigit(c)) {
            throw StreamParser::Error(std::string("PredicateParser::parseNumber invalid char '") + c + "'");
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
            throw StreamParser::Error(std::string("PredicateParser::parseNumber invalid char '") + c + "'");
        }
        s += c;
        while (isdigit(peek())) {
            s += next();
        }

    }

    eckit::Translator<std::string, double> s2d;
    return new PredicateNumber(s2d(s));
}

Predicate* PredicateParser::parseString()
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

    return new PredicateString(s);
}

Predicate* PredicateParser::parseAtom()
{
    eckit::ScopedPtr<Predicate> f;

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
        return new PredicateFunction("neg", parseAtom());
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
                std::vector<Predicate*> args = parseList();
                return new PredicateFunction(name, args);
            }
            else {
                return new PredicateIdent(name);
            }
        } else if (isdigit(c)) {
            return parseNumber();
        }
        break;

    }

    throw StreamParser::Error(std::string("PredicateParser::parseAtom invalid char '") + c + "'");
    return NULL;
}

Predicate* PredicateParser::parsePower()
{
    char name[2] = {0,};

    Predicate* result = parseAtom();
    char c = peek();
    while (c == '^' /*|| c == '*' */ )
    {
        consume(c);
        name[0] = c;
        result = new PredicateBinop(name, result, parseAtom());
        c = peek();
    }

    return result;
}

std::vector<Predicate*> PredicateParser::parseList() {
    std::vector<Predicate*> v;
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


Predicate* PredicateParser::parseFactor()
{
    char name[2] = {0,};

    Predicate* result = parsePower();
    char c = peek();
    while (c == '*' || c == '/')
    {
        consume(c);
        name[0] = c;
        result = new PredicateBinop(name, result, parsePower());
        c = peek();
    }
    return result;
}

Predicate* PredicateParser::parseTerm()
{   char name[2] = {0,};

    Predicate* result = parseFactor();
    char c = peek();
    while (c == '+' || c == '-')
    {
        consume(c);
        name[0] = c;
        result = new PredicateBinop(name, result, parseFactor());
        c = peek();
    }
    return result;
}

Predicate* PredicateParser::parseTest()
{
    char name[3] = {0,};

    Predicate* result = parseTerm();
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

        result = new PredicateFunction(name, result, parseTerm());
        c = peek();
    }
    return result;
}

Predicate* PredicateParser::parse() {
    eckit::ScopedPtr<Predicate> f(parseTest());
    char c;
    if ((c = peek())) {
        throw StreamParser::Error(std::string("Error parsing rules: remaining char: ") + c);
    }
    return f.release();
}


}  // namespace predicate
}  // namespace util
}  // namespace mir
