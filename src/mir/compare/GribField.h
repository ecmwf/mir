/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   GribField.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   May 2016

#ifndef mir_compare_GribField_H
#define mir_compare_GribField_H

#include <iosfwd>
#include <string>
#include <set>
#include <vector>

#include "mir/compare/Field.h"
#include "mir/compare/FieldInfo.h"

struct grib_handle;

namespace mir {
namespace compare {


//----------------------------------------------------------------------------------------------------------------------
//

class GribField : public FieldBase {
public:

    static Field field(const char* buffer, size_t size,
                       const std::string& path, off_t offset,
                       const std::vector<std::string>& ignore);

private:

    GribField(const std::string& path, off_t offset, size_t length);

    void insert(const std::string& key, const std::string& value);
    void insert(const std::string& key, long value);

    void erase(const std::string& key);

    void area(double n, double w, double s, double e);
    void grid(double ns, double we);
    void rotation(double lat, double lon);
    void format(const std::string&);

    void gridtype(const std::string&);
    void gridname(const std::string&);

    void resol(size_t resol);
    void numberOfPoints(long n);

    void param(long n);
    void accuracy(long n);
    void decimalScaleFactor(long n);
    void packing(const std::string& packing);


    void missingValuesPresent(bool on);

    std::map<std::string, std::string>::const_iterator begin() const;

    std::map<std::string, std::string>::const_iterator end() const;

    std::map<std::string, std::string>::const_iterator find(const std::string& key) const;


    bool sameArea(const GribField& other) const;
    bool samePacking(const GribField& other) const;
    bool sameAccuracy(const GribField& other) const;
    bool sameBitmap(const GribField& other) const;
    bool sameFormat(const GribField& other) const;
    bool sameField(const GribField& other) const;
    bool sameGrid(const GribField& other) const;
    bool sameRotation(const GribField& other) const;
    bool sameResol(const GribField& other) const;
    bool sameGridname(const GribField& other) const;
    bool sameGridtype(const GribField& other) const;
    bool sameParam(const GribField& other) const;
    bool sameNumberOfPoints(const GribField& other) const;

    bool match(const std::string&, const std::string&) const;

    size_t numberOfPoints() const;

    const std::string& format() const ;

    std::ostream& printGrid(std::ostream &out) const;

public:

    static void addOptions(std::vector<eckit::option::Option*>& options);
    static void setOptions(const eckit::option::CmdArgs &args);

private:

    // virtual bool equal_to(const GribField& other) const;
    virtual bool wrapped() const;
    virtual bool less_than(const FieldBase& other) const;
    virtual void whiteListEntries(std::ostream&) const;
    virtual size_t differences(const FieldBase& other) const;
    virtual std::ostream& printDifference(std::ostream&, const FieldBase& other) const;
    virtual void compareAreas(std::ostream&, const FieldBase& other) const ;
    virtual bool same(const FieldBase& other) const;
    virtual bool match(const FieldBase& other) const;

    std::map<std::string, std::string> values_;

    long param_;

    bool area_;
    double north_;
    double west_;
    double south_;
    double east_;

    long accuracy_;
    long decimalScaleFactor_;

    bool grid_;
    double west_east_;
    double north_south_;

    bool rotation_;
    double rotation_latitude_;
    double rotation_longitude_;

    std::string packing_;

    bool hasMissing_;
    long resol_;
    std::string gridname_;
    std::string gridtype_;
    std::string format_;

    long numberOfPoints_;

private:


    void print(std::ostream &out) const;


    static void setGrid(GribField& field, grib_handle *h);
    static void setArea(GribField& field, grib_handle *h);

};


//----------------------------------------------------------------------------------------------------------------------

} // namespace mir
} // namespace compare

#endif
