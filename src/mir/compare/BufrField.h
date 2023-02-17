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


#pragma once

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "mir/compare/Field.h"


namespace mir::compare {

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

    void print(std::ostream&) const;
    void printValue(std::ostream&) const;
    void json(eckit::JSON&) const;

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

    bool operator==(const BufrEntry&) const;
    bool operator!=(const BufrEntry&) const;
    bool operator<(const BufrEntry&) const;
};

class BufrField : public FieldBase {
public:
    static Field field(const char* buffer, size_t size, const std::string& path, off_t offset,
                       const std::vector<std::string>& ignore);

public:
    static void addOptions(std::vector<eckit::option::Option*>&);
    static void setOptions(const eckit::option::CmdArgs&);

private:
    BufrField(const char* buffer, size_t size, const std::string& path, off_t offset,
              const std::vector<std::string>& ignore);

    bool operator==(const BufrField&) const;


private:
    std::vector<long> descriptors_;

    std::vector<BufrEntry> activeEntries_;
    std::vector<BufrEntry> allEntries_;

    std::map<std::string, size_t> entriesByName_;
    std::set<std::string> ignored_;


    void print(std::ostream&) const override;
    bool wrapped() const override;
    bool less_than(const FieldBase&) const override;
    void whiteListEntries(std::ostream&) const override;
    size_t differences(const FieldBase&) const override;
    std::ostream& printDifference(std::ostream&, const FieldBase&) const override;
    void compareExtra(std::ostream&, const FieldBase&) const override;
    bool same(const FieldBase&) const override;
    bool match(const FieldBase&) const override;
    std::ostream& printGrid(std::ostream&) const override;
    bool match(const std::string&, const std::string&) const override;
    size_t numberOfPoints() const override;
    const std::string& format() const override;
    bool canCompareFieldValues() const override;
    void json(eckit::JSON&) const override;
};


}  // namespace mir::compare
