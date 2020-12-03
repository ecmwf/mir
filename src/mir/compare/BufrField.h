/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_compare_BufrField_H
#define mir_compare_BufrField_H

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "mir/compare/Field.h"


namespace mir {
namespace compare {

class BufrEntry {

    std::string full_;
    std::string name_;

    std::vector<long> l_;
    std::vector<double> d_;
    std::string s_;

    int type_;
    bool ignore_;

public:
    BufrEntry(const std::string& full, const std::vector<long>& l, const std::vector<double>& d, const std::string& s,
              int type);

    void print(std::ostream& out) const;
    void printValue(std::ostream& out) const;
    void json(eckit::JSON& json) const;

    friend eckit::JSON& operator<<(eckit::JSON& s, const BufrEntry& x) {
        x.json(s);
        return s;
    }

    friend std::ostream& operator<<(std::ostream& s, const BufrEntry& x) {
        x.print(s);
        return s;
    }

    const std::string& full() const { return full_; }

    const std::string& name() const { return name_; }

    bool ignore() const { return ignore_; }

    bool operator==(const BufrEntry& other) const;
    bool operator!=(const BufrEntry& other) const;
    bool operator<(const BufrEntry& other) const;
};

class BufrField : public FieldBase {
public:
    static Field field(const char* buffer, size_t size, const std::string& path, off_t offset,
                       const std::vector<std::string>& ignore);

public:
    static void addOptions(std::vector<eckit::option::Option*>& options);
    static void setOptions(const eckit::option::CmdArgs& args);

private:
    BufrField(const char* buffer, size_t size, const std::string& path, off_t offset,
              const std::vector<std::string>& ignore);
    ~BufrField();

    bool operator==(const BufrField& other) const;


private:
    std::vector<long> descriptors_;

    std::vector<BufrEntry> activeEntries_;
    std::vector<BufrEntry> allEntries_;

    std::map<std::string, size_t> entriesByName_;
    std::set<std::string> ignored_;


    virtual void print(std::ostream& out) const override;
    virtual bool wrapped() const override;
    virtual bool less_than(const FieldBase& other) const override;
    virtual void whiteListEntries(std::ostream&) const override;
    virtual size_t differences(const FieldBase& other) const override;
    virtual std::ostream& printDifference(std::ostream&, const FieldBase& other) const override;
    virtual void compareExtra(std::ostream&, const FieldBase& other) const override;
    virtual bool same(const FieldBase& other) const override;
    virtual bool match(const FieldBase& other) const override;
    virtual std::ostream& printGrid(std::ostream&) const override;
    virtual bool match(const std::string&, const std::string&) const override;
    virtual size_t numberOfPoints() const override;
    virtual const std::string& format() const override;
    virtual bool canCompareFieldValues() const override;
    virtual void json(eckit::JSON& json) const override;
};


}  // namespace compare
}  // namespace mir


#endif
