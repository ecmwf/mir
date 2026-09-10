// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <string>
#include <vector>


namespace mir::netcdf {
class Dataset;
}  // namespace mir::netcdf


namespace mir::netcdf {


class Dimension {
public:
    Dimension(Dataset& owner, const std::string& name, size_t len);

    Dimension(const Dimension&) = delete;
    Dimension(Dimension&&)      = delete;

    virtual ~Dimension();

    void operator=(const Dimension&) = delete;
    void operator=(Dimension&&)      = delete;

    const std::string& name() const;
    size_t count() const;

    virtual int id() const;
    virtual bool sameAs(const Dimension&) const;
    virtual void dump(std::ostream&) const;
    virtual void create(int nc) const;
    virtual void clone(Dataset& owner) const;
    virtual void grow(size_t n);
    virtual bool inUse() const;
    virtual void realDimensions(std::vector<size_t>& dims) const;

protected:
    // -- Members

    Dataset& owner_;
    std::string name_;
    size_t len_;

private:
    // -- Methods
    virtual void print(std::ostream&) const = 0;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Dimension& v) {
        v.print(s);
        return s;
    }
};


}  // namespace mir::netcdf
