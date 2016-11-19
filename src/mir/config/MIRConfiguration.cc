/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Nov 2016


#include "mir/config/MIRConfiguration.h"

#include <iostream>
#include "eckit/filesystem/PathName.h"
#include "eckit/parser/JSONParser.h"
#include "eckit/parser/StringTools.h"
#include "eckit/utils/Translator.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace config {


namespace {


bool string_contains_paramIds(const std::string& str, std::vector<long>& ids) {
    ids.clear();
    if (str.find_first_not_of("0123456789/") != std::string::npos) {
        return false;
    }

    std::vector<std::string> v = eckit::StringTools::split("/", str);
    ids.reserve(v.size());

    for (std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i) {
        long id = 0;
        if (i->length() && (id = eckit::Translator<std::string, long>()(*i))) {
            ids.push_back(id);
        }
    }

    return ids.size();
}


bool string_contains_metadata(const std::string& str, std::string& key, std::string& value) {
    using eckit::StringTools;
    key.clear();
    value.clear();

    std::vector<std::string> v = StringTools::split("=", str);
    if (v.size() == 2) {
        v[0] = StringTools::trim(v[0]);
        v[1] = StringTools::trim(v[1]);
        if (v[0].length() && v[1].length()) {
            key   = v[0];
            value = v[1];
        }
    }
    return key.length() && value.length();
}


}  // (anonymous namespace)


const MIRConfiguration& MIRConfiguration::instance() {
    static MIRConfiguration instance_;
    return instance_;
}


MIRConfiguration::MIRConfiguration() {

    fillKey_ = "class";
    configFile_ = "configuration.json";
    configDir_  = "~mir/etc/mir";


    eckit::PathName path(configFile_);
    if (!path.exists()){
        path = eckit::PathName(configDir_) / path;
    }

    eckit::Log::debug<LibMir>() << "Loading configuration from '" << path << "'" << std::endl;
    std::ifstream in(path.asString().c_str());
    if (!in) {
        throw eckit::CantOpenFile(path);
    }


    eckit::JSONParser parser(in);
    const eckit::ValueMap j = parser.parse();

    root_.reset(new InheritParametrisation());
    parseInheritMap(root_.get(), j);
    eckit::Log::debug<LibMir>() << "root: " << *root_ << std::endl;


//    fill_.clear();
//    parseFillingMap(*fill_, j, configFill_e);
//    eckit::Log::debug<LibMir>() << "fill: " << fill_ << std::endl;


#if 0
    eckit::Log::debug<LibMir>() << "Configure: parse filling..." << std::endl;
    root_.reset(new Inherit());
    parseFillingMap(*root_, j);
    eckit::Log::debug<LibMir>() << root_ << std::endl;
    eckit::Log::debug<LibMir>() << "Configure: parse filling." << std::endl;
#endif


    eckit::Log::info() << "done" << std::endl;
}


bool MIRConfiguration::parseInheritMap(InheritParametrisation* who, const eckit::ValueMap& map) const {

    for (eckit::ValueMap::const_iterator i = map.begin(); i != map.end(); ++i) {
        std::vector<long> ids;
        std::string key;
        std::string value;

        eckit::Log::info() << "+++ key=val: '" << i->first << "'='" << i->second << "'" << std::endl;

        if (i->first == fillKey_) {
            // skip
        }
        else if (string_contains_paramIds(i->first, ids)) {
            eckit::Log::info() << "+++ key=val: '" << i->first << "'='" << i->second << "'" << std::endl;
            ASSERT(i->second.isMap());

            InheritParametrisation* me = new InheritParametrisation(who, ids);
            parseInheritMap(me, i->second);
            who->child(me);

        }
        else if (string_contains_metadata(i->first, key, value)) {
            eckit::Log::info() << "+++ key=val: '" << i->first << "'='" << i->second << "'" << std::endl;
            ASSERT(i->second.isMap());

            InheritParametrisation* me = new InheritParametrisation(who, key, value);
            parseInheritMap(me, i->second);
            who->child(me);

        }
        else {
            eckit::Log::info() << "+++ key=val: '" << i->first << "'='" << i->second << "'" << std::endl;
            ASSERT(!i->second.isMap());

            if (!(who->has(i->first))) {
                who->set(i->first, std::string(i->second));
            }

        }
    }

    return false;
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration[...]";
}


const param::MIRParametrisation* MIRConfiguration::lookup(const long& paramId, const param::MIRParametrisation& metadata) const {
    param::SimpleParametrisation* param = new param::SimpleParametrisation();
    ASSERT(param);

    // inherit from most-specific paramId/metadata individual and its parents
    {
        const InheritParametrisation* who = NULL;
        root_->pick(who, paramId, metadata);
        ASSERT(who);

        eckit::Log::debug<LibMir>() << "MIRConfiguration::lookup: inheriting from " << (*who) << std::endl;
        who->inherit(*param);
    }

    // inherit recursively from a "filling" key
    std::string fillValue;
    size_t check = 0;
    while (param->get(fillKey_, fillValue)) {
        ASSERT(check++ < 50);
        param->clear(fillKey_);

        const InheritParametrisation* who = NULL;
        fill_->pick(who, fillKey_, fillValue);
        ASSERT(who);

        eckit::Log::debug<LibMir>() << "MIRConfiguration::lookup: inheriting from '" << fillKey_ << "=" << fillValue << "': " << (*who) << std::endl;
        who->inherit(*param);
    }

    return param;
}


}  // namespace config
}  // namespace mir


#if 0
#include "eckit/io/Buffer.h"
#include "eckit/parser/JSON.h"
#include "eckit/parser/JSONParser.h"
#include "eckit/testing/Setup.h"


void test_eckit_parser_parse_to_value() {
    std::istringstream in("{ \"a\" : [true, false, 3], \"b\" : 42.3 , \"c\" : null, \"d\" : \"y\n\tr\rh\", \"e\" : \"867017db84f4bc2b5078ca56ffd3b9b9\"}");
    eckit::JSONParser p(in);

    eckit::Value v = p.parse();

    BOOST_TEST_MESSAGE( v );
    BOOST_TEST_MESSAGE( v["a"] );
    BOOST_TEST_MESSAGE( v["a"][2] );

    eckit::JSON j(std::cout);
    j << v;

    BOOST_CHECK( v.isMap() );
    BOOST_CHECK_EQUAL( v.as<eckit::ValueMap>().size(), 5 );

    BOOST_CHECK( v["a"].isList() );
    BOOST_CHECK_EQUAL( v["a"].as<eckit::ValueList>().size(), 3 );


    BOOST_CHECK( v["a"][0].isBool() );
    BOOST_CHECK_EQUAL( v["a"][0].as<bool>(), true );

    BOOST_CHECK( v["a"][1].isBool() );
    BOOST_CHECK_EQUAL( v["a"][1].as<bool>(), false );

    BOOST_CHECK( v["a"][2].isNumber() );
    BOOST_CHECK_EQUAL( (int) v["a"][2], 3 );

    BOOST_CHECK( v["b"].isDouble() );
    BOOST_CHECK_LT( v["b"].as<double>() - 42.3, 1E-12 );

    BOOST_CHECK( v["c"].isNil() );

    BOOST_CHECK( v["d"].isString() );

    BOOST_CHECK( v["e"].isString() );
    BOOST_CHECK_EQUAL( v["e"].as<std::string>(), "867017db84f4bc2b5078ca56ffd3b9b9" );
}

void test_eckit_parser_parse_to_set() {
    std::istringstream in("[ \"a\" , \"b\", \"c\" ]" );
    eckit::JSONParser p(in);
    eckit::Value v = p.parse();

    BOOST_TEST_MESSAGE( v );

    BOOST_CHECK( v.isList() );
    BOOST_CHECK_EQUAL( v.as<eckit::ValueList>().size(), 3 );

    BOOST_CHECK( v[0].isString() );
    BOOST_CHECK_EQUAL( v[0].as<std::string>(), "a" );

    BOOST_CHECK( v[1].isString() );
    BOOST_CHECK_EQUAL( v[1].as<std::string>(), "b" );

    BOOST_CHECK( v[2].isString() );
    BOOST_CHECK_EQUAL( v[2].as<std::string>(), "c" );
}

void test_eckit_parser_parse_to_map() {
    std::istringstream in("{ \"a\" : \"AAA\", \"b\" : 0.0 , \"c\" : \"null\", \"d\" : \"\"}" );
    eckit::JSONParser p(in);
    eckit::Value v = p.parse();

    BOOST_TEST_MESSAGE( v );

    BOOST_CHECK( v.isMap() );
    BOOST_CHECK_EQUAL( v.as<eckit::ValueMap>().size(), 4 );

    BOOST_CHECK( v["a"].isString() );
    BOOST_CHECK_EQUAL( v["a"].as<std::string>(), "AAA" );

    BOOST_CHECK( v["b"].isDouble() );
    BOOST_CHECK_EQUAL( v["b"].as<double>(), 0.0 );

    BOOST_CHECK( v["c"].isString() );
    BOOST_CHECK_EQUAL( v["c"].as<std::string>(), "null" );

    BOOST_CHECK( v["d"].isString() );
    BOOST_CHECK_EQUAL( v["d"].as<std::string>(), "" );
}
#endif
