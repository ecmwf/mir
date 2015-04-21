#ifndef BiLinearPrecipitation_H
#define BiLinearPrecipitation_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Interpolator.h"

class BiLinearPrecipitation : public Interpolator {
  public:
    static const double TRIGGER;
// -- Contructors
    BiLinearPrecipitation();
    BiLinearPrecipitation(bool w, bool a, double nPole, double sPole);

// -- Destructor
    virtual ~BiLinearPrecipitation();

// -- Overridden methods
    virtual double interpolatedValue(const Point& point, const vector<FieldPoint>& nearests) const;

  protected:
    void print(ostream&) const;

  private:

// Methods
    virtual double unnormalisedWeight(const Point& p, const FieldPoint& oposite) const {
        return fabs(oposite.latitude() - p.latitude()) * fabs(oposite.longitude() - p.longitude());
    }
};
#endif
