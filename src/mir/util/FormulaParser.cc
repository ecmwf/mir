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


namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------


FormulaParser::FormulaParser(std::istream &in) : StreamParser(in, true) {
}

void FormulaParser::parseIdent() {
    std::string s;
    char c = peek();
    while (isalnum(c)) {
        s += next();
        c = peek();
    }
}


void FormulaParser::parseNumber()
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

}

void FormulaParser::parseString()
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
}

void FormulaParser::parseAtom()
{
    char c = peek();
    switch (c)
    {
    case '(':
        consume('(');
        parseTest();
        consume(')');
        break;

    case '-':
        consume('-');
        parseAtom();
        break;

    case '\'':
        parseString();
        break;

    case '"':
        parseString();
        break;

    default:
        if (isalpha(c) || c == '_') {
            parseIdent();
            if (peek() == '(') {
                parseList();
            }
        } else if (isdigit(c)) {
            parseNumber();
        }
        else {
            throw StreamParser::Error(std::string("FormulaParser::parseAtom invalid  char '") + c + "'");
        }
        break;

    }
}

void FormulaParser::parsePower()
{
    parseAtom();
    char c = peek();
    while (c == '^' /*|| c == '*' */ )
    {

        consume(c);
        parseAtom();
        c = peek();
    }
}

void FormulaParser::parseList() {
    consume('(');
    while (peek() != ')') {
        parseTest();
        if (peek() == ')') {
            break;
        }
        consume(',');
    }
    consume(')');
}


void FormulaParser::parseFactor()
{
    parsePower();
    char c = peek();
    while (c == '*' || c == '/')
    {
        consume(c);
        parsePower();
        c = peek();
    }
}

void FormulaParser::parseTerm()
{   parseFactor();
    char c = peek();
    while (c == '+' || c == '-')
    {
        consume(c);
        parseFactor();
        c = peek();
    }
}

void FormulaParser::parseTest()
{
    parseTerm();
    char c = peek();
    while (c == '<' || c == '>' || c == '=')
    {
        consume(c);
        c = peek();
        if (c == '=' || c == '>')
        {
            consume(c);
        }
        parseTerm();
        c = peek();
    }
}

void FormulaParser::parse() {
    parseTest();
    char c;
    if ((c = peek())) {
        throw StreamParser::Error(std::string("Error parsing rules: remaining char: ") + c);
    }
}


//----------------------------------------------------------------------------------------------------------------------

} // namespace util
} // namespace mir
