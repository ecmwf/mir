/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#ifndef mir_cmp_Configuration_h
#define mir_cmp_Configuration_h

#include <iosfwd>
#include <map>
#include <string>
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/ParserConsumer.h"
//#include <iosfwd>
//#include <map>
//#include <string>
//#include "mir/param/SimpleParametrisation.h"
//#include "mir/util/ParserConsumer.h"


namespace mir_cmp {


class Configuration : public mir::util::ParserConsumer {

    // -- Types
    typedef mir::param::SimpleParametrisation param_t;
    typedef std::map< std::string, param_t* > map_t;

public:

    // -- Methods

    const param_t* lookup(std::string key) const;

    static const Configuration& instance();

private:

    // No copy allowed

    Configuration(const Configuration&);
    Configuration& operator=(const Configuration&);

    // -- Contructors

    Configuration();

    // -- Destructor

    ~Configuration();

    // -- Members

    long current_;
    param_t* scope_;
    map_t settings_;
    const std::string flattenSetting_;
    const size_t flattenDepth_;

    // -- Methods

    void print(std::ostream&) const;

    // -- Overridden methods

    // From ParserConsumer
    virtual void store(const std::string& name, const char* value);
    virtual void store(const std::string& name, const std::string& value);
    virtual void store(const std::string& name, bool value);
    virtual void store(const std::string& name, long value);
    virtual void store(const std::string& name, double value);

    virtual void scope(const std::string& name);

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Configuration& p) {
        p.print(s);
        return s;
    }

};


}  // namespace mir_cmp


#endif
