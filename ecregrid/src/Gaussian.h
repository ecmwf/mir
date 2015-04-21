/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Gaussian_H
#define Gaussian_H

#ifndef Grid_H
#include "Grid.h"
#endif

// Forward declarations

class Gaussian : public Grid {
  public:
    Gaussian(int n);
    Gaussian(double north, double west, double south, double east, int n);
    Gaussian(const Area &other, int n);
    // -- Destructor
    virtual ~Gaussian();  // Change to virtual if base class

    int gaussianNumber() const;
    int    truncate(int truncation)      const;
    virtual int northSouthNumberOfPoints()   const;
    virtual int westEastNumberOfPoints()     const;
    int   northIndex(double north)      const;
    int   southIndex(double south)      const;
    int   northIndexMars(double north)  const;
    int   southIndexMars(double south)  const;

    virtual Area  setGlobalArea()                     const ;
    virtual Area  setGlobalArea(double west)          const;
    Area  fitToGlobalArea(double west)        const;
    void  setGlobalNorthSouth(double &north, double &south) const;

  protected:
    int gaussianNumber_;
    vector<double> latitudes_;
    void print(ostream &) const; // Change to virtual if base class

  private:
    void latitudes(vector<double> &lats) const;
    void northHemisphereLatitudes(vector<double> &latitudes) const;

    virtual double westEastIncrement()   const {
        return 90.0 / gaussianNumber_;
    };
    double northSouthIncrement() const {
        return 90.0 / gaussianNumber_;
    };
    int     poleEquatorNumberOfPoints()  const {
        return gaussianNumber_;
    }
    virtual string   gridType()          const {
        return "gaussian";
    }
    string   coeffInfo()         const;

    virtual Area  adjustArea(const Area &area, bool globalWE, bool globalNS) const;
    virtual void  adjustAreaWestEastMars( double &west, double &east, double increment) const;

    virtual bool  isGlobalWestEast()                  const ;
    bool  isGlobalNorthSouth()                        const ;

    int    matchGaussian()               const;

    // -- Methods
    void  gaussianLatitudesFirstGuess(vector<double> &values) const;
    void  gaussianLatitudes(vector<double> &latitudes) const;
    void  weightsY(vector<double> &w) const;

    size_t  loadGridSpec(vector<long> &rgSpec) const;

    void  printLatitudes() const;

    void dump()                                       const ;

    // Members

};

#endif
