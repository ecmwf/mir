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


#ifndef mir_util_Arguments_H
#define mir_util_Arguments_H


#include <map>
#include <string>
#include <vector>

#include "eckit/memory/Builder.h"

//#include "mir/Params.h"


namespace mir {
namespace util {


/**
 * @brief Arguments provides an interpretation to command-line arguments,
 * combining consistent behaviours of:
 * - modes of operation based on name (./???_mode) or "git-style" (./??? [^-]mode)
 * - short (-?) and long (--??) options
 * - positional (-? opt1 opt2) and named parameters (--?? key2=opt2 key1=opt1) to the options
 * - default options parameters, and
 * - standard base set of options (-h, --help, -v, --verbose)
 */
struct Arguments {
  public:

    /// Constructor performing all the parsing
    Arguments(int argc, char** argv);


  public:

    /**
     * @brief Command-line option (long-style) and parameters interpreter.
     * @note Works on named, non-positional parameters with default values.
     */
    struct argument_option_t {
      public:

        /// Option positional, non-named parameters type
        typedef std::map< std::string, std::string > params_t;

        /// Option processing warnings type
        typedef std::vector< std::string > warnings_t;

        /**
         * @brief Command-line option (long-style) and parameters interpreter.
         * @note Works on named, non-positional parameters with default values.
         * @param _option      option longname (excluding starting "--")
         * @param _help        option description
         * @param _nparams     number of (_param_name,_param_value) pairs
         * @param _param_name  option parameter name
         * @param _param_value option parameter (default) value
         */
        argument_option_t(
            const std::string& _option,
            const std::string& _help        = "",
            const unsigned     _nparams     = 0,
            const char*        _param_name  = "",
            const char*        _param_value = "",
            ... );

        /// @brief Process named parameters
        void process(const std::string& _param, const std::string& _value);

        /// @brief Process a named parameter container type
        void process(const params_t& param, const warnings_t& warn);


      public:

        /// Option name (can work as key)
        const std::string option;

        /// Option descrition
        const std::string help;

        /// Access option parameters, after processing (check processing warnings)
        const params_t params() const {
            return params_;
        }

        /// Access processing warnings
        /// @note ensure it is empty after processing arguments
        const warnings_t warnings() {
            return warnings_;
        }


      protected:

        /// Named (non-positional) option parameters (internal)
        params_t params_;

        /// Collect processing warnings (ensure it is empty after processing arguments)
        std::vector< std::string > warnings_;

    };


    /**
     * @brief Command-line option (short-style) and parameters interpreter.
     * @note Works on positional, non-named parameters.
     */
    struct argument_shortoption_t {
      public:

        /**
         * @brief Command-line option (short-style) and parameters interpreter.
         * @note Works on positional, non-named parameters.
         * @param _option      option longname (excluding starting "--")
         * @param _nparams     number of (_param_name,_param_value) pairs
         * @param _param_name  option parameter name
         * @param _param_value option parameter (default) value
         */
        argument_shortoption_t(
            const std::string& _shortoption,
            const std::string& _option,
            const std::string& _help        = "",
            const unsigned     _nparams     = 0,
            const char*        _param_name  = "",
            const char*        _param_value = "",
            ... );

        /**
         * @brief Map a positional parameter to a name-value map, according to a
         * naming vector (to return the map later).
         * @param _value parameter value to associate with parameter name
         */
        void process(const std::string& _value);

        /**
         * @brief Access option parameters, after processing (check processing warnings)
         */
        const argument_option_t::params_t params();

        /**
         * @brief Access option warnings (ensure it is empty after processing)
         */
        const argument_option_t::warnings_t warnings() {
            return warnings_;
        }


      public:

        /// Option short name (can work as key)
        std::string shortoption;

        /// Option name (can work as key)
        std::string option;

        /// Option descrition
        std::string help;


      protected:

        /// Named (non-positional) option parameters
        argument_option_t::params_t params_;

        /// Collect processing warnings (ensure it is empty after processing arguments)
        argument_option_t::warnings_t warnings_;

        /// Positional (non-named) option parameters
        std::vector< std::string > positional_params_;

        /// Positional (non-named) option parameters
        size_t positional_counter_;

    };


    /// @brief Argument mode interpreting short into long options
    struct argmode_t : private std::vector< argument_shortoption_t > {
      protected:

        /// Base argument mode container (convenience)
        typedef std::vector< argument_shortoption_t > container_t;


      public:

        /// Base argument mode builder defintion
        typedef eckit::BuilderT0< argmode_t > builder_t;

        /// Argument mode identifier
        static std::string className() {
            return "";
        }

        /// Argument mode default, always present options
        argmode_t();

        /// Element access: access specified element with bounds checking
        using container_t::at;

        /// Element access: access specified element
        using container_t::operator[];

        /// Element access: access the first element
        using container_t::front;

        /// Element access: access the last element
        using container_t::back;

        /// Iterators: returns an iterator to the beginning
        using container_t::begin;

        /// Iterators: returns an iterator to the end
        using container_t::end;

        /// Iterators: returns a reverse iterator to the beginning
        using container_t::rbegin;

        /// Iterators: returns a reverse iterator to the end
        using container_t::rend;

        /// Capacity: checks whether the container is empty
        using container_t::empty;

        /// Capacity: returns the number of elements
        using container_t::size;

        /// Modifiers: inserts elements
        using container_t::insert;

        /// Modifiers: adds elements to the end
        using container_t::push_back;

    };


  protected:

    /**
     * @return short-to-long options interpreter
     * @param[in] _basename command-line option with calling executable basename
     * @param[in] _arg1 first argument following executable
     * @param[out] _mode_argposition return 1 if _basename or 2 if _arg1 was used
     * to determine interpreter, 0 if neither
     */
    argmode_t& get_argument_mode(
        const std::string& _basename,
        const std::string& _arg1,
        int& _modearg ) const;

    /**
     * @return interpreted short options (as long options)
     * @param[in] _shortoptions short options interpretation
     * @param[in] _args user-provided arguments to interpret
     */
    std::vector< argument_option_t > get_long_options(
        const std::vector< argument_shortoption_t >& _shortoptions,
        const std::vector< std::string >& _args ) const;


  public:

    const std::vector< std::string >& argv() const {
        return argv_;
    }


  protected:

    std::vector< std::string > argv_;

};


}  // namespace util
}  // namespace mir
#endif
