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


#ifndef FieldParametrisation_H
#define FieldParametrisation_H

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace param {


class FieldParametrisation : public MIRParametrisation {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    FieldParametrisation();

    // -- Destructor

    virtual ~FieldParametrisation(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members
    // None

    // -- Methods

    // -- Overridden methods
    // From MIRParametrisation
    virtual bool get(const std::string &name, std::string &value) const;
    virtual bool get(const std::string &name, bool &value) const;
    virtual bool get(const std::string &name, long &value) const;
    virtual bool get(const std::string &name, double &value) const;
    virtual bool get(const std::string &name, std::vector<long> &value) const;
    virtual bool get(const std::string &name, std::vector<double> &value) const;

    virtual bool has(const std::string &name) const;

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    FieldParametrisation(const FieldParametrisation &);
    FieldParametrisation &operator=(const FieldParametrisation &);

    // -- Members

    mutable bool check_;
    mutable const MIRParametrisation *style_;

    // -- Methods

    template<class T>
    bool _get(const std::string &, T &) const;


    virtual void latitudes(std::vector<double> &) const;
    virtual void longitudes(std::vector<double> &) const;

    // -- Overridden methods

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends


};


}  // namespace param
}  // namespace mir
#endif

