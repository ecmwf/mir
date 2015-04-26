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


#include "eckit/runtime/Context.h"

#include "mir/logic/ToolLogic.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Arguments.h"


namespace mir {
namespace logic {


ToolLogic::ToolLogic(const param::MIRParametrisation &parametrisation):
    MIRLogic(parametrisation) {
}


ToolLogic::~ToolLogic() {
}


void ToolLogic::print(std::ostream &out) const {
    out << "ToolLogic[]";
}


void ToolLogic::prepare(param::RuntimeParametrisation& runtime, std::vector<std::auto_ptr< action::Action > > &actions) const {
    // All the nasty logic goes there

    size_t argc = eckit::Context::instance().argc();
    for(size_t i = 0; i < argc; i++) {
        std::cout << eckit::Context::instance().argv(i) << std::endl;
    }

}


/**
 * @brief Command-line option (long-style) and parameters interpreter.
 * It interprets command-line options suitable for interpolation.
 * @note Works on named, non-positional parameters with default values.
 */
struct argmode_interpolate_t : public util::Arguments::argmode_t {

    /// Argument mode identifier
    static std::string className() {
        return "interpolate";
    }

    /// Argument mode options
    argmode_interpolate_t() {
        push_back(util::Arguments::argument_shortoption_t( "i", "input",    "input filename",            1, "filename",      "" ));
        push_back(util::Arguments::argument_shortoption_t( "o", "output",   "output filename",           2, "filename",      "", "format", "grib" ));
        push_back(util::Arguments::argument_shortoption_t( "I", "method",   "interpolation method",      1, "method",        "" ));
        push_back(util::Arguments::argument_shortoption_t( "g", "template", "maximum truncation length", 1, "MaxTruncation", "" ));
    }

};


/**
 * @brief Command-line option (long-style) and parameters interpreter.
 * It interprets command-line options suitable for transforms (truncations).
 * @note Works on named, non-positional parameters with default values.
 */
struct argmode_transform_t : public util::Arguments::argmode_t {

    /// Argument mode identifier
    static std::string className() {
        return "transform";
    }

    /// Argument mode options
    argmode_transform_t() {
        push_back(util::Arguments::argument_shortoption_t( "i", "input",     "input filename",            1, "filename",      "" ));
        push_back(util::Arguments::argument_shortoption_t( "o", "output",    "output filename",           2, "filename",      "", "format", "grib" ));
        push_back(util::Arguments::argument_shortoption_t( "t", "transform", "maximum truncation length", 1, "MaxTruncation", "" ));
    }

};


// register command-line tool specialized logic (and associated parsing modes)
namespace {
static MIRLogicBuilder< ToolLogic > __tool("tool");
eckit::ConcreteBuilderT0< util::Arguments::argmode_t, argmode_transform_t   > __Arguments_argmode_transform;
eckit::ConcreteBuilderT0< util::Arguments::argmode_t, argmode_interpolate_t > __Arguments_argmode_interpolate;
}


}  // namespace logic
}  // namespace mir

