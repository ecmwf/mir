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
/// @author Pedro Maciel
/// @date Apr 2015


#include "eckit/runtime/Tool.h"
#include "mir/param/JSONConfiguration.h"


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
    std::string json = "{\"a\":2,\"b\":[1,2,3], \"c\":{\"b\":1.2}}";
    in.str(json);

    mir::param::JSONConfiguration config(in);

    std::cout << config << std::endl;

    size_t n;
    config.get("a", n);
    std::cout << "a = " << n << std::endl;

    double x;
    config.get("c.b", x);
    std::cout << "c.b = " << x << std::endl;

    x = -9;
    config.get("c.d", x);
    std::cout << "c.d = " << x << std::endl;

    std::vector<double> v;
    config.get("b", v);
    for (size_t i = 0; i < v.size(); i++) {
        std::cout << "b = " << v[i] << std::endl;
    }

    mir::param::JSONConfiguration sub(config, "c");
    std::cout << sub << std::endl;
    sub.set("d.b", 2L);
}


int main( int argc, char **argv ) {
    JSONTool tool(argc, argv);
    tool.start();
    return 0;
}

