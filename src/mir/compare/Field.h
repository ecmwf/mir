/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Field.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   May 2016

#ifndef mir_compare_Field_H
#define mir_compare_Field_H

#include <iosfwd>
#include <string>
#include <set>
#include <vector>

#include "mir/compare/FieldInfo.h"


namespace eckit {
namespace option {
class Option;
class CmdArgs;
}
}

namespace mir {
namespace compare {

class FieldSet;

//----------------------------------------------------------------------------------------------------------------------
//

class Field {
public:

    Field(const std::string& path, off_t offset, size_t length);

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
    void packing(const std::string& packing);

    bool operator<(const Field& other) const;

    void bitmap(bool on);

    std::map<std::string, std::string>::const_iterator begin() const;

    std::map<std::string, std::string>::const_iterator end() const;

    std::map<std::string, std::string>::const_iterator find(const std::string& key) const;

    std::vector<Field> bestMatches(const FieldSet& fields) const;

    double compare(const Field& other) const;

    bool same(const Field& other) const;
    bool match(const Field& other) const;


    bool sameArea(const Field& other) const;
    bool samePacking(const Field& other) const;
    bool sameAccuracy(const Field& other) const;
    bool sameBitmap(const Field& other) const;
    bool sameFormat(const Field& other) const;
    bool sameField(const Field& other) const;
    bool sameGrid(const Field& other) const;
    bool sameRotation(const Field& other) const;
    bool sameResol(const Field& other) const;
    bool sameGridname(const Field& other) const;
    bool sameGridtype(const Field& other) const;
    bool sameParam(const Field& other) const;
    bool sameNumberOfPoints(const Field& other) const;

    off_t offset() const ;

    size_t length() const ;

    const std::string& path() const ;

    std::ostream& printDifference(std::ostream& out, const Field & other) const;

    static void addOptions(std::vector<eckit::option::Option*>& options);
    static void setOptions(const eckit::option::CmdArgs &args);

private:

    bool operator==(const Field& other) const;

    FieldInfo info_;

    std::map<std::string, std::string> values_;

    long param_;

    bool area_;
    double north_;
    double west_;
    double south_;
    double east_;

    long accuracy_;

    bool grid_;
    double west_east_;
    double north_south_;

    bool rotation_;
    double rotation_latitude_;
    double rotation_longitude_;

    std::string packing_;

    bool bitmap_;
    long resol_;
    std::string gridname_;
    std::string gridtype_;
    std::string format_;

    long numberOfPoints_;

private:


    double compareAreas(const Field& other) const;

    void print(std::ostream &out) const;

    friend std::ostream &operator<<(std::ostream &s, const Field &x) {
        x.print(s);
        return s;
    }

};


//----------------------------------------------------------------------------------------------------------------------

} // namespace mir
} // namespace compare

#endif
