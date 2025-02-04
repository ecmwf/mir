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
#include <string>
#include <vector>


namespace mir::reorder {


using Renumber = std::vector<size_t>;


class Reorder {
public:
    Reorder() = default;

    virtual ~Reorder() = default;

    // N is the size of the array to reorder
    // TODO: change to Representation (numberOfPoints())
    virtual Renumber reorder(size_t N) const = 0;

    Reorder(const Reorder&)            = delete;
    Reorder(Reorder&&)                 = delete;
    Reorder& operator=(const Reorder&) = delete;
    Reorder& operator=(Reorder&&)      = delete;

private:
    virtual void print(std::ostream&) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const Reorder& p) {
        p.print(s);
        return s;
    }
};


class ReorderFactory {
private:
    std::string name_;
    virtual Reorder* make() = 0;

protected:
    explicit ReorderFactory(const std::string& name);

    virtual ~ReorderFactory();

public:
    ReorderFactory(const ReorderFactory&)            = delete;
    ReorderFactory(ReorderFactory&&)                 = delete;
    ReorderFactory& operator=(const ReorderFactory&) = delete;
    ReorderFactory& operator=(ReorderFactory&&)      = delete;

    static Reorder* build(const std::string& name);
    static std::ostream& list(std::ostream& out);
};


template <class T>
class ReorderBuilder : public ReorderFactory {
    Reorder* make() override { return new T; }

public:
    explicit ReorderBuilder(const std::string& name) : ReorderFactory(name) {}
};


}  // namespace mir::reorder
