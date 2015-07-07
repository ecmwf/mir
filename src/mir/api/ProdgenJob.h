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


#ifndef ProdgenJob_H
#define ProdgenJob_H

#include <string>

#include "mir/util/BoundingBox.h"
#include "mir/input/RawMetadata.h"


namespace mir {
namespace api {


class ProdgenJob : public input::RawMetadata {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    ProdgenJob();

    // -- Destructor

    virtual ~ProdgenJob();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void usewind(bool);
    void uselsm(bool);
    void useprecip(bool);
    void missingvalue(bool);

    void lsm_param(bool);
    void parameter(size_t);
    void table(size_t);
    void reduced(size_t);
    void truncation(size_t);

    void auto_pl();
    void g_pnts(int*);

    // ============


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


    // -- Class members
    // None

    // -- Class methods
    // None

  private:


    // -- Members

    std::string gridType_;
    util::BoundingBox bbox_;

    size_t N_;
    std::vector<long> pl_;

    size_t truncation_;

    bool gridded_;
    bool spectral_;

    bool usewind_;
    bool uselsm_;
    bool useprecip_;
    bool missingvalue_;
    bool lsm_param_;

    size_t parameter_;
    size_t table_;

    // -- Methods


    // -- Overridden methods


    virtual const std::vector<long>& pl() const ;

    virtual size_t N() const;
    virtual size_t truncation() const;

    virtual const util::BoundingBox &bbox() const;
    virtual const std::string &gridType() const ;
    virtual void print(std::ostream &) const;
    virtual bool gridded() const;
    virtual bool spectral() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

};


}  // namespace api
}  // namespace mir
#endif

