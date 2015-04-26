/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Pedro Maciel
/// @date Apr 2015


#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/memory/Factory.h"
#include "eckit/parser/StringTools.h"
#include "eckit/parser/Tokenizer.h"

#include "mir/util/Arguments.h"

#include <cstdarg>  // last because this is "ugly" code


namespace mir {
namespace util {


Arguments::argument_option_t::argument_option_t(
    const std::string& _option,
    const std::string& _help,
    const unsigned _nparams,
    const char* _param_name,
    const char* _param_value,
    ... )
    : option(_option),
      help(_help) {
    typedef std::pair< std::string, std::string > value_t;
    if (!_nparams)
        return;

    // append (non-variadic) option/default value pair
    if (_nparams > 0) {
        ASSERT(_param_name != NULL);
        ASSERT(_param_value != NULL);
        if (std::string(_param_name).length())
            params_.insert(value_t(
                               _param_name,
                               _param_value ));
    }

    // append (variadic) subsequent options/default values pairs
    va_list ap;
    va_start(ap, _param_value);
    for (unsigned i = 1; i < _nparams; ++i) {
        _param_name  = va_arg(ap, const char*);
        _param_value = va_arg(ap, const char*);
        ASSERT(_param_name != NULL);
        ASSERT(_param_value != NULL);
        if (std::string(_param_name).length())
            params_.insert(value_t(
                               _param_name,
                               _param_value ));
    }
    va_end(ap);
}


void Arguments::argument_option_t::process(
    const std::string& _param,
    const std::string& _value ) {
    if (params_.find(_param) == params_.end()) {
        eckit::StrStream os;
        os << "option '" << option << "' does not have parameter '" << _param << "'" << eckit::StrStream::ends;
        warnings_.push_back(std::string(os));
        return;
    }
    params_[_param] = _value;
}


void Arguments::argument_option_t::process(
    const argument_option_t::params_t& param,
    const argument_option_t::warnings_t& warn ) {
    // append all "inherited" warnings
    for (warnings_t::const_iterator w = warn.begin(); w != warn.end(); ++w) {
        warnings_.push_back(*w);
    }

    // overwrite keys as passed in, but do not recognize keys not in map
    for (params_t::const_iterator p = param.begin(); p != param.end(); ++p) {
        params_t::iterator q;
        if ((q = params_.find(p->first)) != params_.end()) {
            q->second = p->second;
        } else {
            eckit::StrStream os;
            os << "Option '" << option << "' does not have parameter '" << p->first + "'" << eckit::StrStream::ends;
            warnings_.push_back(std::string(os));
        }
    }
}


Arguments::argument_shortoption_t::argument_shortoption_t(
    const std::string& _shortoption,
    const std::string& _option,
    const std::string& _help,
    const unsigned _nparams,
    const char* _param_name,
    const char* _param_value,
    ... )
    : shortoption(_shortoption),
      option(_option),
      help(_help),
      positional_counter_(0) {
    if (!_nparams)
        return;
    std::string p;  // parameter name
    std::string v;  // parameter value

    // append (non-variadic) option/default value pair
    if (_nparams > 0) {
        ASSERT(_param_name != NULL);
        ASSERT(_param_value != NULL);
        p = eckit::StringTools::trim(_param_name);
        v = eckit::StringTools::trim(_param_value);
        if (p.length())
            positional_params_.push_back(v.length() ? p + '=' + v : p);
    }

    // append (variadic) subsequent options/default values pairs
    va_list ap;
    va_start(ap, _param_value);
    for (unsigned i = 1; i < _nparams; ++i) {
        _param_name  = va_arg(ap, const char*);
        _param_value = va_arg(ap, const char*);
        ASSERT(_param_name != NULL);
        ASSERT(_param_value != NULL);
        p = eckit::StringTools::trim(_param_name);
        v = eckit::StringTools::trim(_param_value);
        if (p.length())
            positional_params_.push_back(v.length() ? p + '=' + v : p);
    }
    va_end(ap);
}


void Arguments::argument_shortoption_t::process(const std::string& _value) {
    // string parser (splits "a=b" into ["a","b"])
    eckit::Tokenizer parse_equal('=');

    // check if position is within bounds
    if (positional_counter_ >= positional_params_.size()) {
        eckit::StrStream os;
        os << "Option '" << shortoption << "': '" << _value
           << " exceeds the number of expected parameters ("
           << positional_params_.size() << ")" << eckit::StrStream::ends;

        warnings_.push_back(std::string(os));
        return;
    }

    // look for the parameter name (increment position), checking if a default
    // parameter is set (param=value)
    std::vector< std::string > pv;
    parse_equal(positional_params_[positional_counter_++], pv);
    if (pv.size() != 1 && pv.size() != 2) {
        eckit::StrStream os;
        os << "Option '" << shortoption << "': could not find parameter to assign value '" << _value + "' to" << eckit::StrStream::ends;
        warnings_.push_back(std::string(os));
        return;
    }
    const std::string param = eckit::StringTools::trim(pv[0]);
    if (!param.length() || !_value.length()) {
        eckit::StrStream os;
        os << "Option '" << shortoption << "': could not assign '" << param << "'='" << _value << "'" << eckit::StrStream::ends;
        warnings_.push_back(std::string(os));
        return;
    }

    // all ok, assign
    params_[param] = _value;
}


const Arguments::argument_option_t::params_t Arguments::argument_shortoption_t::params() {
    // string parser (splits "a=b" into ["a","b"])
    eckit::Tokenizer parse_equal('=');

    // check for additional (optional) parameters which have defaults
    // if a default value hasn't been (positionally) assigned, do it
    std::vector< std::string > pv;
    std::vector< std::string >::const_iterator p;
    for (p = positional_params_.begin(); p != positional_params_.end(); ++p) {
        pv.clear();
        parse_equal(*p, pv);
        if (pv.size() == 2) {
            const std::string
            param = eckit::StringTools::trim(pv[0]),
            value = eckit::StringTools::trim(pv[1]);
            if (value.length() && params_.find(param) == params_.end())
                params_[param] = value;
        }
    }

    return params_;
}


Arguments::argmode_t::argmode_t() {
    push_back(argument_shortoption_t("h", "help", "this help"));
    push_back(argument_shortoption_t("v", "verbose", "produce verbose messages"));
}


Arguments::Arguments(int argc, char** argv) {
    // general assumptions on shell-provided options
    ASSERT(argc > 0);
    ASSERT(argv != NULL);


    // 0. store original options (for future reference)
    for (int c = 0; c < argc; c++)
        argv_.push_back(argv[c]);


    // 1. detect argument interpreter mode
    int mode_argposition = 0;
    argmode_t& argmode = get_argument_mode(
                             /* $0 */ eckit::PathName(argv_[0]).baseName().asString(),
                             /* $1 */ argv_.size() > 1 ? argv_[1] : "",
                             mode_argposition );


    // 2. parse short options (be tolerant to warnings)
    std::vector< argument_option_t > opt = get_long_options(
            // interpreter
            std::vector< argument_shortoption_t >(
                argmode.begin(),
                argmode.end() ),
            // user arguments
            std::vector< std::string >(
                argv_.begin() + mode_argposition,
                argv_.end() ));


    // 3. fill long options parameters (be strict on warnings)


    std::cout << eckit::Factory< argmode_t >::instance() << std::endl;




#if 0
    argument_option_t la("help1", "this help1", 1, "xs");
    argument_option_t lo("help2", "this help2", 1, "option", "optionparam");
    argument_option_t lc("help3", "this help3");
    /*
         General procedure:
         - build short options mapping based on mode
         - check for help/verbose
         - map short options to long options
       */

    std::vector< argument_option_t > options;
#endif
}


Arguments::argmode_t& Arguments::get_argument_mode(
    const std::string& _basename,
    const std::string& _arg1,
    int& _modearg ) const {
    eckit::Factory< argmode_t >& fmodes(eckit::Factory< argmode_t >::instance());

    // 1. check 1st argument (busybox-style)
    const std::string::size_type uscore = _basename.find_first_of("_");
    if (uscore != std::string::npos && fmodes.exists(_basename.substr(uscore + 1))) {
        std::string a(_basename.substr(uscore + 1));
        _modearg = 1;
        DEBUG_VAR(a);
        return *(fmodes.get( _basename.substr(uscore + 1) ).create());
    }

    // 2. check 2nd argument ($1) (git-style)
    if (fmodes.exists(_arg1)) {
        _modearg = 2;
        DEBUG_VAR(_modearg);
        return *(fmodes.get( _arg1 ).create());
    }

    // 3. return default argument mode
    _modearg = 0;
    DEBUG_VAR(_modearg);
    return *(fmodes.get("").create());
}


std::vector< Arguments::argument_option_t > Arguments::get_long_options(
    const std::vector< argument_shortoption_t >& _shortoptions,
    const std::vector< std::string >& _args ) const {
    std::vector< argument_option_t > opt;
    std::vector< std::string >::const_iterator a;
    for (a = _args.begin(); a != _args.end(); ++a) {



    }
    return opt;
}


// register default mode
namespace {
eckit::ConcreteBuilderT0< Arguments::argmode_t, Arguments::argmode_t > __Arguments_argmode;
}


}  // namespace util
}  // namespace mir

