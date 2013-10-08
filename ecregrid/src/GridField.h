/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef GridField_H
#define GridField_H

// namespace src;

// Headers
#ifndef   Field_H
#include "Field.h"
#endif

#ifndef   Grid_H
#include "Grid.h"
#endif


// Forward declarations
class Point;
class FieldPoint;
class Area;
class FieldDescription;

// 

class GridField : public Field {
public:


// -- Contructors

	GridField( Grid* grid, int scan, int frame, bool bitmap, const string& bitmapFile, int bitsPerValue, int editionNumber, double missingValue);

	GridField( Grid* grid, const Parameter& param, int scan, int frame = 0, bool bitmap = false, const string& bitmapFile = "none", double missingValue = MISSING_VALUE);

// Input
	GridField( Grid* grid, const Parameter& param, const string& units, int editionNumber, int centre, const string& levType, int level,int date, int time, const string& stepUnits, int startStep, int endStep, int bitsPerValue, int scan, bool bitmap, const vector<double>& values, double missingValue);

// Output
	GridField(Grid* grid, const Field& f, int bitsPerValue, int editionNumber, int scan, bool bitmap, const vector<double>& values, double missingValue); 
	GridField(const Parameter& param, Grid* grid, const Field& f, int bitsPerValue, int editionNumber, int scan, bool bitmap, const vector<double>& values, double missingValue) ;

// -- Destructor

	virtual ~GridField(); // Change to virtual if base class

// -- Convertors
	// None

// -- Operators
	bool operator==(const GridField& other) const;

// -- Methods

	Grid&          grid() const; 

	string         nameOfField()   const { return grid_->composedName(); }
	int            scanningMode()  const { return scanningMode_; }

	bool           isRotated()     const { return grid_->rotated(); }
	long           frame()         const { return frame_; }
	bool           bitmap()        const { return bitmap_; }
	string         bitmapFile()    const { return bitmapFile_; }
	double         missingValue()  const { return missingValue_; }

// pole proccesssing
	bool   extrapolateLinearOnPole() const;
	bool   extrapolateAverageOnPole(double& northValue, double& southValue) const;
	double average(long offset, long end) const;
	double averageForPole(const string& pole) const;
// --------------------------------------------------------
	GridField*     gridFieldForOtherScanningMode() const;

	Grid*          releaseGrid()         { return grid_.release(); }

	void dump2file(const string& name) const;
	void dump() const;

// -- Overridden methods
	void              resetOutput(FieldDescription* out) const;
	FieldDescription* makeOutput(const FieldDescription& out)  const;

    size_t dataLength()    const { return data_.size(); }

    const vector<double>& data() const { return data_; }

	double  dataAverage() const;
	bool isMissingValue(double value) const {return same(value,missingValue_); }
	void  outputArea(double* a)      const;
	void  setOutputAreaAndBasics(FieldDescription& out) const;
	unsigned long  calculatedLength() const  { return grid_->calculatedNumberOfPoints(); }

	void accumulatedData(vector<double>& newData) const { if (dataLength() > 0)
                                                              grid_->accumulatedDataWE(&data_[0], dataLength(), newData); }
    void squaredData(vector<double>& data) const;

// -- Class members
	// None

// -- Class methods
	// None

protected:

// -- Members
	// None

// -- Methods
	
	virtual void print(ostream&) const; // Change to virtual if base class	

// -- Overridden methods
	// None

// -- Class members

// -- Class methods
	// None

private:

// No copy allowed

	GridField(const GridField&);
	GridField& operator=(const GridField&);

// -- Members

	auto_ptr<Grid> grid_;

    const vector<double> data_;
	int            scanningMode_;

	int       	   frame_;
	bool           bitmap_;
	string         bitmapFile_;

	double         missingValue_;


// -- Methods
	// None

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends

	friend ostream& operator<<(ostream& s,const GridField& p)
		{ p.print(s); return s; }

};

#endif
