/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#ifndef mir_netcdf_RegularLL
#define mir_netcdf_RegularLL

#include "mir/netcdf/GridSpec.h"
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace netcdf {


class IrregularLatlon : public GridSpec {
public:

  IrregularLatlon(const Variable &,
                  double north,
                  double south,
                  const std::vector<double>& latitudes,
                  double west,
                  double east,
                  const std::vector<double>& longitudes);



  virtual ~IrregularLatlon();

  // -- Methods


  static GridSpec* guess(const Variable &variable,
                         const Variable &latitudes,
                         const Variable &longitudes);


protected:

  // -- Members

  bool jScansPositively_;

  double north_;
  double south_;
  std::vector<double> latitudes_;

  double west_;
  double east_;
  std::vector<double> longitudes_;

private:

  IrregularLatlon(const IrregularLatlon &);
  IrregularLatlon &operator=(const IrregularLatlon &);


  // - Methods

  virtual void print(std::ostream &s) const;

  // For MIR
  virtual bool has(const std::string& name) const;
  virtual bool get(const std::string&, long&) const;
  virtual bool get(const std::string&, std::string&) const;
  virtual bool get(const std::string &name, double &value) const;
  virtual bool get(const std::string &name, std::vector<double> &value) const;

  virtual void reorder(std::vector<double>& values) const;

};

}
}
#endif
