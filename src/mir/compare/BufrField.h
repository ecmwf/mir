/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   BufrField.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   May 2016

#ifndef mir_compare_BufrField_H
#define mir_compare_BufrField_H

#include <iosfwd>
#include <string>
#include <set>
#include <vector>

#include "mir/compare/Field.h"
#include "mir/compare/FieldInfo.h"


namespace mir {
namespace compare {

class FieldSet;

//----------------------------------------------------------------------------------------------------------------------
//

class BufrEntry {

    std::string full_;
    std::string name_;

    long l_;
    double d_;
    std::string s_;

    int type_;
    bool ignore_;

public:

    BufrEntry(const std::string& full,
              long l,
              double d,
              const std::string& s,
              int type);

    void print(std::ostream &out) const;
    void printValue(std::ostream &out) const;
    void json(eckit::JSON& json) const;

    friend eckit::JSON &operator<<(eckit::JSON &s, const BufrEntry &x) {
        x.json(s);
        return s;
    }

    friend std::ostream &operator<<(std::ostream &s, const BufrEntry &x) {
        x.print(s);
        return s;
    }

    const std::string& full() const {
        return full_;
    }

    const std::string& name() const {
        return name_;
    }

    bool ignore() const {
        return ignore_;
    }

    bool operator==(const BufrEntry &other) const;
    bool operator!=(const BufrEntry &other) const;
    bool operator<(const BufrEntry &other) const;

};

class BufrField : public FieldBase {
public:

    static Field field(const char* buffer, size_t size,
                       const std::string& path, off_t offset,
                       const std::vector<std::string>& ignore);
public:


    static void addOptions(std::vector<eckit::option::Option*>& options);
    static void setOptions(const eckit::option::CmdArgs &args);

private:
    BufrField(const char* buffer, size_t size,
              const std::string& path, off_t offset,
              const std::vector<std::string>& ignore);
    ~BufrField();

    bool operator==(const BufrField& other) const;


private:

    std::vector<long> descriptors_;

    std::vector<BufrEntry> activeEntries_;
    std::vector<BufrEntry> allEntries_;

    std::map<std::string, size_t> entriesByName_;
    std::set<std::string> ignored_;


    virtual void print(std::ostream &out) const;
    virtual bool wrapped() const;
    virtual bool less_than(const FieldBase& other) const;
    virtual void whiteListEntries(std::ostream&) const;
    virtual size_t differences(const FieldBase& other) const;
    virtual std::ostream& printDifference(std::ostream&, const FieldBase& other) const;
    virtual void compareExtra(std::ostream&, const FieldBase& other) const ;
    virtual bool same(const FieldBase& other) const;
    virtual bool match(const FieldBase& other) const;
    virtual std::ostream& printGrid(std::ostream&) const;
    virtual bool match(const std::string&, const std::string&) const;
    virtual size_t numberOfPoints() const;
    virtual const std::string& format() const;
    virtual void json(eckit::JSON& json) const;


};


//----------------------------------------------------------------------------------------------------------------------

} // namespace mir
} // namespace compare

#endif
