/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "GridToGridMatrixTransformer.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef Wind_H
#include "Wind.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef Field_H
#include "Field.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef Interpolator_H
#include "Interpolator.h"
#endif

#ifndef Factory_H
#include "Factory.h"
#endif

#ifndef DerivedSubgridParameters_H
#include "DerivedSubgridParameters.h"
#endif

#ifndef SubArea_H
#include "SubArea.h"
#endif

#ifndef Extraction_H
#include "Extraction.h"
#endif

#ifndef PartialDerivatives_H
#include "PartialDerivatives.h"
#endif

#include <eckit/utils/Timer.h>

#include <Eigen/Sparse>
#include <Eigen/Dense>

#include<sys/types.h>
#include<sys/msg.h>
#include <sys/shm.h>

GridToGridMatrixTransformer::GridToGridMatrixTransformer(const string& intMethod, const string& lsmMethod, int nptsInt, const string& type, const string& extrapolate, double missingValue) :
    interpolationMethod_(intMethod),
    lsmMethod_(lsmMethod),
    pointsForInterpolation_(nptsInt),
    type_(type),
    extrapolate_(extrapolate),
    missingValue_(missingValue) {
}

GridToGridMatrixTransformer::~GridToGridMatrixTransformer() {
}

Field* GridToGridMatrixTransformer::transform(const Field& in, const Field& out) const {
    eckit::Timer t1("GridToGridMatrixTransformer::transform");
    const GridField& input  = dynamic_cast<const GridField&>(in);
    const GridField& output = dynamic_cast<const GridField&>(out);

    if ((input == output) ) {
        cout << "---------------------------------------------------"  <<  endl;
        cout << "||| GridToGridMatrixTransformer::transform Input and Output field are the SAME! |||"  <<  endl;
        cout << "---------------------------------------------------"  <<  endl;
        if(IS_SET("ECREGRID_FORCE_INTERPOLATION")) {
            cout << "GridToGridMatrixTransformer::transform FORCE INTERPOLATION" << endl;
        } else {
            return 0;
        }
    }
    int inScMode = input.scanningMode();
    double inMissingValue = input.missingValue();

    double missingValue = output.missingValue();
    if(input.bitmap())
        missingValue = input.missingValue();

    // reference to input data
    const vector<double>& data = input.data();

    // Get the input and output grid definitions as strings
    stringstream ss;
    ss << interpolationMethod_ << "_"<< input.grid().predefinedLsmFileName() <<"_to_" << output.grid().predefinedLsmFileName();
    string uniqueName = ss.str();

    // work out a shm object filename from this:
    std::string folder = getDataDir();
    std::string shm_file = folder + "/" + ss.str();

    // We need to create the file if it doesn't already exist
    FILE* filePtr = fopen(shm_file.c_str(),"r");
    if (!filePtr)
        filePtr = fopen(shm_file.c_str(),"w");
    fclose(filePtr);

    // need input field size N and output field size M so we can build sparse
    // NxM matrix

    const size_t output_point_count = output.grid().calculatedNumberOfPoints();
    const size_t input_point_count = input.grid().calculatedNumberOfPoints();

    // Look for this matrix and see whether it is cached.
    //
    Eigen::SparseMatrix<double> weights = Eigen::SparseMatrix<double>(output_point_count, input_point_count);

    // this is the set of (i,j,value) triplets that we either load from shm or generate
    // NB This appears to be how one serialises an Eigen SparseMatrix
    std::vector<Eigen::Triplet<double> > insertions;


    // Do we have it available in shared memory?
    key_t key = ftok(shm_file.c_str(),1);

    // test we have one
    int shmid = shmget(key, 0, 0600);
    bool available = (shmid != -1);

    if (available) {
        // get pointer to the shm memory
        char* shmptr = (char*)shmat(shmid, NULL, 0);

        // running pointer to read from it
        char* ptr = shmptr;

        // read out the triplets into vector
        // The first entry is the number of triplets (long). Then the
        // triplets are written as int, int, double values
        long total_size = 0;
        total_size = *(long*)&ptr[0];
        ptr += sizeof(long);

        int row, col;
        double val;

        {
            eckit::Timer triplet_timer("loading of triplets");
            insertions.resize(total_size);

            double val;
            int row, col;
            for (unsigned int i = 0; i < total_size; i++) {
                memcpy(&row, ptr, sizeof(row));
                ptr += sizeof(row);
                memcpy(&col, ptr, sizeof(col));
                ptr += sizeof(col);
                memcpy(&val, ptr, sizeof(val));
                ptr += sizeof(val);
                insertions[i] = Eigen::Triplet<double>(row,col,val);
            }

        }
        // done with shared memory now
        shmdt(shmptr);
    } else {
        eckit::Timer t3("Weight generation");
        // Generate a set of triplets according to your
        // interpolation scheme and then filling the matrix accordingly.
        //
        // for each lat / lon in target grid B we find the relevant points in grid
        // A and assign a weight to each of these in the matrix
        //
        // NB This part of the code is not particularly optimal
        //


        // Generate input and output grids
        vector<Point> inPts;
        input.grid().generateGrid1D(inPts);

        vector<Point> outPts;
        output.grid().generateGrid1D(outPts);

        // create an interpolator according to what was requested
        Factory factory;
        bool useLsm = false;     // TODO handle this
        bool useBitmap = false;  // TODO handle this
        auto_ptr<Interpolator> method(factory.interpolationMethod(interpolationMethod_, pointsForInterpolation_,input,output.grid(),useLsm,"off",extrapolate_,missingValue_,useBitmap));

        vector<FieldPoint> nearests;
        nearests.reserve(pointsForInterpolation_);
        auto_ptr<GridContext> ctx(input.grid().getGridContext());


        // for each output point, get the set of input weights to use for its
        // calculation
        for (unsigned int i = 0; i < outPts.size(); i++) {
            // find nearest points:
            input.grid().nearestPoints(ctx.get(),outPts[i],nearests,data,inScMode,nearests.size());

            // we now have N nearest points. For each of these, generate
            // weights and write them to the correct position in the current row
            vector<double> w(nearests.size(), 0.0);
            method->interpolationWeights(outPts[i], nearests, w);
            // now find the linear index corresponding to the i,j grid index
            // of each nearest point, and write the relevant weight to this
            // index
            for (unsigned int j = 0; j < nearests.size(); j++) {
                FieldPoint& pt = nearests[j];
                long row_index = input.grid().getIndex(pt.iIndex(), pt.jIndex());
                insertions.push_back(Eigen::Triplet<double>(i, row_index, w[j]));
            }
        }

        // Store in shared memory

        const int trip_size = 2*sizeof(int)+sizeof(double);

        // store in a shared memory segment
        // Total size == all the triplets plus the number of triplets written
        // as a long
        //
        long shm_size =  sizeof(long) + trip_size * insertions.size();

        // create the shared memory
        int id = shmget(key, shm_size, IPC_CREAT|0600);
        // get its address
        const char* shmptr = (char*)shmat(id, NULL, 0);

        if (shmptr > 0) {
            long total_size = insertions.size();
            cout << "Writing " << total_size << " weights to shared memory" << endl;

            // Keep an index of the location of the next write to shared
            // memory
            long ind = 0;
            // write total size
            *(long*)&shmptr[0] = total_size;
            ind += sizeof(long);

            // write all insertions
            for (unsigned int j = 0; j < insertions.size(); j++) {
                Eigen::Triplet<double>& ins = insertions[j];
                *(int*)&shmptr[ind] = ins.row();
                ind += sizeof(int);
                *(int*)&shmptr[ind] = ins.col();
                ind += sizeof(int);
                *(double*)&shmptr[ind] = ins.value();
                ind += sizeof(double);
            }

        }
        // done with shared memory now
        shmdt(shmptr);
    }

    // Now insert the triplets into the SparseMatrix. Unfortunately this
    // appears to be necessary even when caching
    {
        eckit::Timer t4("Setting weights from triplets");
        weights.setFromTriplets(insertions.begin(), insertions.end());
    }

    // Create input matrix and fill from input data
    Eigen::MatrixXd A(input_point_count, 1);
    A.col(0) = Eigen::VectorXd::Map(&data[0],data.size());

    // Create output matrix of correct size
    Eigen::MatrixXd B(1, output_point_count);

    {
        eckit::Timer t4("Matrix multiplication");
        B = weights * A;
    }


    /*
    // debug - output the calculation that we will do
    for (unsigned int iout = 0; iout < output_point_count; iout++)
    {
        vector<double> inner_values(input_point_count, 0.0);
        Eigen::VectorXf tmp = weights.col(iout);
        ///std::copy(tmp.data(), tmp.data() + input_point_count, inner_values.begin());
        // Write the output values to STL vector
        //Eigen::Map<Eigen::VectorXd>(inner_values.data(), input_point_count) = weights.col(out);
        //Eigen::Map<Eigen::VectorXd>(values.data(), B.rows()) = B.col(0);
        stringstream ss;
        //for (Eigen::SparseMatrix<double>::InnerIterator it(weights, iout); it; ++it)
        //for (Eigen::VectorXf::InnerIterator it(tmp); it; ++it)
        //for (std::vector<double>::iterator it = inner_values.begin(); it != inner_values.end(); it++)
        for (unsigned int iin = 0; iin < tmp.size(); iin++)
        {

        //for (unsigned int iin = 0; iin < input_point_count; iin++)
       // {
            //double w = weights.coeffRef(iout, iin);
            //double w = it.value();
            double w = tmp(iin);
            if (fabs(w) > 1e-10)
            {
                //int iin = it.index();
                ss << " + ("<< w << " * " << A.coeffRef(iin, 0) << ")";
            }
        }
        cout << "Output pt " << iout << " = " << ss.str() << endl;

    }
    */

    // use existing ecregrid code to handle this data as required:
    {
        Factory factory;
        auto_ptr<Extraction> extraction(factory.multiExtraction(output,missingValue));

        Grid* possibleGrid   = output.grid().newGrid(input.grid());

        if( *possibleGrid == output.grid() && !output.grid().area().empty()) {
            if(DEBUG) {
                cout << "************************************************************************************************************" << endl;
                cout << "*** GridToGridMatrixTransformer::transform - Required  Output Area is NOT changed => "  << output.grid().area() <<   endl;
                cout << "************************************************************************************************************" << endl;
            }
        } else {
            if(DEBUG) {
                cout << "************************************************************************************************************" << endl;
                cout << "*** GridToGridMatrixTransformer::transform  - Output Area changed to => "  << possibleGrid->area() <<   endl;
                cout << "************************************************************************************************************" << endl;
            }
        }

        const long valuesSize = possibleGrid->calculatedNumberOfPoints();
        vector<double> values(valuesSize);
        // Write the output values to STL vector
        Eigen::Map<Eigen::VectorXd>(values.data(), B.rows()) = B.col(0);
        // write first N values
        cout.precision(15);
        for (unsigned int i = 0; i < 1024; i++)
            cout << "B value["<<i<<"] = " << B.col(0)[i]  << endl;

        if(DEBUG)
            cout << "GridToGridMatrixTransformer::transform possible Grid " << *possibleGrid << endl;

        bool bitmap = input.bitmap();
        if(input.grid().equals(*possibleGrid) && (type_ != "zonalderivativ" && type_ != "meridionalderivativ")) {
            SubArea subarea(possibleGrid->area());
            //		SubArea subarea(output.grid().area());
            subarea.extract(input,values);
            if(DEBUG) {
                cout << "****************************************************" << endl;
                cout << "GridToGridMatrixTransformer::transform Sub Area extraction " << endl;
                cout << "****************************************************" << endl;
            }
            if(extraction.get()) {
                bitmap = true;
                extraction->extract(*possibleGrid, values);
                if(DEBUG)
                    cout << "GridToGridMatrixTransformer::transform Extraction on top of Sub Area extraction" << *extraction << endl;
            }
        }

        return new GridField(possibleGrid, input, output.bitsPerValue(), output.editionNumber(), output.scanningMode(), bitmap, values, missingValue);

    }
}

/*
bool GridToGridMatrixTransformer::transform(const GridField& input, const Grid& outGrid, const Extraction* extraction, vector<double>& values) const
{
    cout << "GridToGridMatrixTransformer::transform(const GridField& input, const Grid& outGrid, const Extraction* extraction, vector<double>& values)" << endl;
	Factory factory;
	bool bitmap = input.bitmap();
    // We assume the input vector has been sized

	if(input.grid() == outGrid) {
        const vector<double>& data = input.data();
        values = data;

		if(extraction){
			bitmap = true;
            extraction->extract(input.grid(), values);
			if(DEBUG)
				cout << "GridToGridMatrixTransformer::transform Extraction without Interplation - " << *extraction << endl;
		}
	}
	else{
		if(DEBUG)
			cout << "GridToGridMatrixTransformer::transform Requested Interpolation method: "  << interpolationMethod_ <<  endl;

		bool applyLsm = input.lsm();
		if (DISABLE_LSM || lsmMethod_ == "off")
       		applyLsm = false;
		string lsmSource = lsmMethod_;
    	char* lsmMethod = getenv("ECREGRID_LSM_SOURCE");
    	if(lsmMethod){
			lsmSource = string(lsmMethod);
    	}


		if( type_ == "meridionalderivativ"){
			meridionalPartialDerivatives(input,values);
			return false;
		}

		if( type_ == "zonalderivativ"){
			zonalPartialDerivatives(input,values);
			return false;
		}
		if(DEBUG)
			cout << "GridToGridMatrixTransformer::transform Lsm Source: "  << lsmSource <<  endl;
		auto_ptr<Interpolator>method(factory.interpolationMethod(interpolationMethod_, pointsForInterpolation_,input,outGrid,applyLsm,lsmSource,extrapolate_,missingValue_,bitmap));
		if(type_ != "interpolation") {
			if( type_ == "stddev") {
				method->standardDeviation(input, outGrid, values);
			}
			else{
				auto_ptr<DerivedSubgridParameters> derived(factory.selectDerivedSubgridParameter(type_));
				method->derivedSubgridParameters(input, outGrid, values, *derived);
			}
		}
		else{
			method->interpolate(input, outGrid, values);
		}

		if(extraction) {
			bitmap = true;
			extraction->extract(outGrid, values);
		}
	}
	return bitmap;
}
*/

Wind* GridToGridMatrixTransformer::transformVector(const Field& inU, const Field& inV, const Field& out) const {
    throw NotImplementedFeature("GridToGridMatrixTransformer::transformVector");
}






