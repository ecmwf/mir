/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef PolarStereographic_H
#define PolarStereographic_H

#ifndef Projection_H
#include "Projection.h"
#endif

// Forward declarations

class PolarStereographic : public Projection {
  public:

// -- Contructors
//	PolarStereographic(double standardLatitude = 60.0 , double centralLongitude = 0, double dxInMetres = 150000.0, double dxInMetres = 150000.0,int xCoordinateOfPole = 81, int yCoordinateOfPole = 81, int xDirectionNumberOfPoints = 161, int yDirectionNumberOfPoints = 161, bool northPole = true);
    PolarStereographic( const vector<Point>& llgrid, double north, double west, double south, double east, double standardLatitude, double centralLongitude, double dxInMetres, double dyInMetres, int xCoordinateOfPole, int yCoordinateOfPole, int xDirectionNumberOfPoints, int yDirectionNumberOfPoints, bool northPole);

    PolarStereographic(int areaNumber = 100);

// -- Destructor
    ~PolarStereographic(); // Change to virtual if base class

// Methods
    void generateGrid(vector<Point>& llgrid) const;
    void generateGridFirstLatLon(vector<Point>& llgrid) const;

// Overriden methods
    int westEastNumberOfPoints()      const {
        return xDirectionNumberOfPoints_;
    }
    int northSouthNumberOfPoints()    const {
        return yDirectionNumberOfPoints_;
    }
    double  northSouthIncrement()     const {
        return float(dyInMetres_);
    }
    double  westEastIncrement()       const {
        return float(dxInMetres_);
    }

  protected:

// Overriden methods
    void print(ostream&) const;

// -- Methods

  private:

// No copy allowed
    PolarStereographic(const PolarStereographic&);
    PolarStereographic& operator=(const PolarStereographic&);

// Members
    double standardLatitude_;
//	double incrementInMetresStandardLatitude_;

    double dxInMetres_;
    double dyInMetres_;

    double centralLongitude_;

    int    xCoordinateOfPole_;
    int    yCoordinateOfPole_;

    int    xTopLeft_;
    int    yTopLeft_;

    int    xDirectionNumberOfPoints_;
    int    yDirectionNumberOfPoints_;

    bool   northPole_;

    vector<Point> points_;

// Overriden methods

    Grid*    newGrid(const Grid& other)        const;
    Grid*    newGrid(const Area& other)        const;



    string   composedName()  const {
        return "polar_stereographic";
    }

    virtual bool sameAs(const RegularLatLon& other)   const  {
        return Projection::sameAs(other);
    }
    virtual bool sameAs(const RegularLatLonCellCentered& other)   const  {
        return Projection::sameAs(other);
    }
    virtual bool sameAs(const ReducedLatLon& other)   const  {
        return Projection::sameAs(other);
    }
    virtual bool sameAs(const RotatedRegularLatLon& other)   const  {
        return Projection::sameAs(other);
    }
    virtual bool sameAs(const ReducedGaussian& other) const  {
        return Projection::sameAs(other);
    }
    virtual bool sameAs(const RegularGaussian& other) const  {
        return Projection::sameAs(other);
    }
    virtual bool sameAs(const PseudoRegularGaussian& other) const  {
        return Projection::sameAs(other);
    }
    virtual bool sameAs(const  PolarStereographic& other) const  ;
    virtual bool sameAs(const ListOfPoints& other)    const  {
        return Projection::sameAs(other);
    }
    bool equals(const Grid& other) const {
        return other.sameAs(*this);
    }

    string   predefinedLsmFileName() const;

// -- Methods
    void standardAreaDefinitions(int areaNumber);
    void dump()                   const ;

};

#endif
