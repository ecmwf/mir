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


#ifndef CachedParametrisation_H
#define CachedParametrisation_H

#include <map>
#include <set>
#include <string>
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"



namespace mir {
namespace param {


class CachedParametrisation : public MIRParametrisation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    CachedParametrisation(MIRParametrisation& parametrisation);

    // -- Destructor

    virtual ~CachedParametrisation();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void reset();
    //
    void set(const std::string& name, int value);
    void set(const std::string& name, long value);
    void set(const std::string& name, double value);
    void set(const std::string& name, const std::string& value);
    void set(const std::string& name, const char* value);


    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members


    // -- Methods

    virtual void print(std::ostream&) const;


    // -- Overridden methods

    // From MIRParametrisation
    virtual bool has(const std::string& name) const;

    virtual bool get(const std::string& name, std::string& value) const;
    virtual bool get(const std::string& name, bool& value) const;
    virtual bool get(const std::string& name, int& value) const;
    virtual bool get(const std::string& name, long& value) const;
    virtual bool get(const std::string& name, float& value) const;
    virtual bool get(const std::string& name, double& value) const;

    virtual bool get(const std::string& name, std::vector<int>& value) const;
    virtual bool get(const std::string& name, std::vector<long>& value) const;
    virtual bool get(const std::string& name, std::vector<float>& value) const;
    virtual bool get(const std::string& name, std::vector<double>& value) const;
    virtual bool get(const std::string& name, std::vector<std::string>& value) const;


    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Types

    // No copy allowed

    CachedParametrisation(const CachedParametrisation&);
    CachedParametrisation& operator=(const CachedParametrisation&);

    // -- Members

    MIRParametrisation& parametrisation_;
    mutable SimpleParametrisation cache_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    template<class T>
    bool _get(const std::string& name, T& value) const;

    template<class T>
    void _set(const std::string& name, const T& value) const;
    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends


};


}  // namespace param
}  // namespace mir


#endif

