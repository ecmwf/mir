/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef InterpolatorLsm_H
#define InterpolatorLsm_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#ifndef Interpolator_H
#include "Interpolator.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#include "ref_counted_ptr.h"

// Forward declarations

class Point;
class Grid;

// global function not ideal
inline bool isLand(double lsm) 
{ 
    if(lsm > 0.5 )
        return true; 
#if ECREGRID_EMOS_SIMULATION
    if (same(lsm,0.5))
        return true; 
#endif
    return false; 
}
// 

class InterpolatorLsm : public Interpolator {
public:

// -- Contructors
	InterpolatorLsm(const Grid& input, const Grid& output, const string& lsmMethod);
	InterpolatorLsm(int n, const Grid& input, const Grid& output, const string& lsmMethod);
	InterpolatorLsm(bool w, bool a, double nPole, double sPole, int n, const Grid& input, const Grid& output, const string& lsmMethod);

// -- Destructor
	virtual ~InterpolatorLsm();

// -- Methods

	void lsm(const Grid& input, const Grid& output, const string& lsmMethod);

	double useNearestNeigbour(const Point& where, const vector<FieldPoint>& nearests, int size, double missingValue) const;
	double useNearestNeigbour(const Point& where, const vector<FieldPoint>& nearests, int size) const;

protected:
// -- Methods
	virtual void print(ostream&) const;

	double adjustWeight(bool where, bool neighbour, double weight) const
	{
		if(where != neighbour){
			return weight * LSM_FACTOR;
		}
		return weight;
	}

    ref_counted_ptr<const vector<double> > inLsmData_;
    string inLsmFilePreserve_;
    bool firstInput_;
    
    ref_counted_ptr<const vector<double> > outLsmData_;
    string outLsmFilePreserve_;
    bool firstOutput_;

	bool emosTrigger_;



private:

// -- Friends
    friend ostream& operator<<(ostream& s,const InterpolatorLsm& p)
	        { p.print(s); return s; }
};


#endif

