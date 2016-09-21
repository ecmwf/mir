/*
* (C) Copyright 1996-2016 ECMWF.
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
#include "eckit/parser/Tokenizer.h"

#include "mir/input/GribFileInput.h"


class MIRInspect : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);

  public:
    MIRInspect(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void MIRInspect::usage(const std::string &tool) {
    eckit::Log::info()
            << "\n" "Usage: " << tool << " [key1 key2 ...] file.grib"
               "\n" "Examples:"
               "\n" "% " << tool << " grid area file.grib"
               "\n" "% " << tool << " reduced file.grib"
               "\n" "% " << tool << " regular file.grib"
               "\n" "% " << tool << " truncation file.grib"
               "\n" "% " << tool << " octahedral file.grib"
            << std::endl;
}

void MIRInspect::run() {
    if (argc() <= 2) {
        usage(name());
    }

    mir::input::GribFileInput file(argv(argc() - 1));

    while (file.next()) {
        mir::input::MIRInput &input = file;

        const mir::param::MIRParametrisation &parametrisation = input.parametrisation();

        const char *sep = "";
        std::string value;

        for (int i = 1; i < argc() - 1; i++) {
            std::cout << sep << argv(i) << "=";
            if (parametrisation.get(argv(i), value)) {
                std::cout << value;
            } else {
                std::cout << "<not found>";
            }
            sep = ", ";
        }

        std::cout << std::endl;
    }
}


int main( int argc, char **argv ) {
    MIRInspect tool(argc, argv);
    return tool.start();
}

