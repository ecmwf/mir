/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date Jul 2015


#include "eckit/runtime/Tool.h"
#include "eckit/config/JSONConfiguration.h"
#include "eckit/config/LocalConfiguration.h"


class JSONTool : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);

  public:
    JSONTool(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};


void JSONTool::run() {

    std::istringstream in;
    std::string json = R"(
        {
            "a" : 2,
            "b" : [1,2,3],
            "c" : {"b": 1.2},
            "fields":[{ "datatype" : "real64", "name" : "pressure" },
                      { "datatype" : "real64", "name" : "temperature" },
                      { "datatype" : "int32",  "name" : "soiltype" }]

      }
      )";

    in.str(json);

    eckit::JSONConfiguration config(in);

    std::cout << config << eckit::newl;

    size_t n;
    if (config.get("a", n)) {
        std::cout << "a = " << n << eckit::newl;
    } else {
        std::cout << "a not found" << eckit::newl;
    }

    double x;
    if (config.get("c.b", x)) {
        std::cout << "c.b = " << x << eckit::newl;
    } else {
        std::cout << "c.b not found" << eckit::newl;
    }

    if (config.get("c.d", x)) {
        std::cout << "c.d = " << x << eckit::newl;
    } else {
        std::cout << "c.d not found" << eckit::newl;
    }

    std::vector<double> v;
    if (config.get("b", v)) {
        for (size_t i = 0; i < v.size(); i++) {
            std::cout << "b = " << v[i] << eckit::newl;
        }
    } else {
        std::cout << "b not found" << eckit::newl;
    }

    eckit::LocalConfiguration sub(config, "c");
    std::cout << "++++++++ " << sub << eckit::newl;
    sub.set("a.s", 2L);
    sub.set("a.b.c.d", 2.6);
    std::cout << "++++++++ " << sub << eckit::newl;
    sub.set("a.b.c.r", false);
    std::cout << "++++++++ " << sub << eckit::newl;
    std::cout << config << eckit::newl;

    eckit::LocalConfiguration sub2(config);
    sub2.set("p", "p");
    std::cout << "++++++++ " << sub2 << eckit::newl;

    eckit::LocalConfiguration sub3;
    sub3.set("p", "p");
    std::cout << "++++++++ " << sub3 << eckit::newl;


    std::vector<eckit::LocalConfiguration> fields;
    config.get("fields", fields);

    for (size_t i = 0; i < fields.size(); i++) {
        std::cout << fields[i] << eckit::newl;
    }
}


int main( int argc, char **argv ) {
    JSONTool tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    tool.start();
    return 0;
#else
    return tool.start();
#endif
}

