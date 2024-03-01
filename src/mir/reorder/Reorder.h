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


#include <iosfwd>
#include <string>
#include <vector>


namespace mir::reorder {


using Renumber = std::vector<size_t>;


struct Reorder {
    Reorder() = default;

    virtual ~Reorder() = default;

    virtual Renumber reorder() const = 0;

    Reorder(const Reorder&)            = delete;
    Reorder(Reorder&&)                 = delete;
    Reorder& operator=(const Reorder&) = delete;
    Reorder& operator=(Reorder&&)      = delete;
};


class ReorderFactory {
private:
    std::string name_;
    virtual Reorder* make(size_t N) = 0;

protected:
    explicit ReorderFactory(const std::string& name);

    virtual ~ReorderFactory();

public:
    ReorderFactory(const ReorderFactory&)            = delete;
    ReorderFactory(ReorderFactory&&)                 = delete;
    ReorderFactory& operator=(const ReorderFactory&) = delete;
    ReorderFactory& operator=(ReorderFactory&&)      = delete;

    static Reorder* build(const std::string& name, size_t N);
    static std::ostream& list(std::ostream& out);
};


template <class T>
class ReorderBuilder : public ReorderFactory {
    Reorder* make(size_t N) override { return new T(N); }

public:
    explicit ReorderBuilder(const std::string& name) : ReorderFactory(name) {}
};


}  // namespace mir::reorder
