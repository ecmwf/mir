// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <map>
#include <string>


namespace mir::netcdf {
class Attribute;
}  // namespace mir::netcdf


namespace mir::netcdf {


class Endowed {
public:
    Endowed();
    virtual ~Endowed();

    // -- Methods
    void add(Attribute*);

    void getAttributes(int nc, int i, int nattr);
    void copyAttributes(const Endowed&);
    void mergeAttributes(const Endowed&);

    virtual const std::string& path() const = 0;
    virtual int varid() const               = 0;
    virtual const std::string& name() const = 0;

    const std::map<std::string, Attribute*>& attributes() const;

    const Attribute& getAttribute(const std::string& name) const;

protected:
    // -- Members

    std::map<std::string, Attribute*> attributes_;

private:
    Endowed(const Endowed&);
    Endowed& operator=(const Endowed&);
};


}  // namespace mir::netcdf
