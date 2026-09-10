// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>


namespace mir::netcdf {


class NCFile {
public:
    NCFile(const std::string& path);
    ~NCFile();

    int open();
    void close();

    const std::string& path() const;

protected:
    // -- Members
    std::string path_;
    int nc_;
    bool open_;

private:
    NCFile(const NCFile&);
    NCFile& operator=(const NCFile&);

    // -- Methods
    void print(std::ostream&) const;


    // -- Friends
    friend std::ostream& operator<<(std::ostream& out, const NCFile& v) {
        v.print(out);
        return out;
    }
};


}  // namespace mir::netcdf
