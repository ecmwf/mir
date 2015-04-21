/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LatLon_H
#define LatLon_H

#ifndef Grid_H
#include "Grid.h"
#endif

// Forward declarations

class LatLon : public Grid {
  public:

// -- Contructors
    LatLon(double ns, double we);
    LatLon(double north, double west, double south, double east, double ns, double we);
    LatLon(const Area& other, double ns, double we);

// for reduced grid
    LatLon(double ns);
    LatLon(double north, double west, double south, double east, double ns);
    LatLon(const Area& other, double ns);

// with Number of Points
    LatLon(double north, double west, double south, double east, double ns, long nsNumber);
    LatLon(const Area& other, double ns, long nsNumber);


// -- Destructor
    virtual ~LatLon(); // Change to virtual if base class


// Methods
    double calculateNortSouthIncrement(double north, double south, long npts) const;
    virtual double calculateWestEastIncrement(double east, double west, double inc, long npts) const;
    virtual double calculateWestEastIncrement(long npts) const;
// Overriden methods
    virtual int westEastNumberOfPoints()           const;
    virtual int northSouthNumberOfPoints()         const;
    virtual Area  setGlobalArea()          const;
    virtual Area  setGlobalArea(double west)  const;
    Area  fitToGlobalArea(double west)        const;

  protected:

// Overriden methods
    virtual void latitudes(vector<double>& lats) const;
    virtual double  westEastIncrement()       const {
        return we_;
    };
    void print(ostream&) const;

// -- Methods
    double we() const;
    double ns() const;
    virtual bool  areValidIncrements() const;
    virtual bool  isGlobalWestEast()         const ;

    // creates LsmFilename for LatLon grids
    string constructLsmFilename(const string& stub) const;
// Members
    double ns_;
    double we_;
    vector<double> latitudes_;
    long           latitudesSize_;

  private:

// No copy allowed
    LatLon(const LatLon&);
    LatLon& operator=(const LatLon&);


// Overriden methods

    void latitudesLocal(vector<double>& lats) const;
    void northHemisphereLatitudes(vector<double>& latitudes) const;

    double  northSouthIncrement()     const {
        return ns_;
    };
    int     poleEquatorNumberOfPoints() const;
    virtual string   gridType()         const {
        return "latlon";
    }

    string   coeffInfo()  const;

    Area  adjustArea(const Area& area, bool globalWE, bool globalNS) const;
    void  adjustAreaWestEastMars( double& west, double& east, double increment) const;
    void  setGlobalNorthSouth(double& north, double& south) const;

    virtual bool  isGlobalNorthSouth()       const ;

    int   northIndex(double north)   const;
    int   southIndex(double south)   const;
    size_t loadGridSpec(vector<long>& rgSpec) const;

    int    truncate(int truncation)      const;
    int    matchGaussian()               const;

    void  weightsY(vector<double>& w) const;

// -- Methods

    void  dump()                   const ;

};

#endif
