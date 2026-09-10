// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "eckit/types/DateTime.h"

#include "mir/netcdf/Calendar.h"


namespace mir::netcdf {


class GregorianCalendar : public Calendar {
public:
    GregorianCalendar(const Variable&);
    ~GregorianCalendar() override;

private:
    eckit::DateTime reference_;
    long long offset_;
    mutable long long zero_;

    std::string units_;
    std::string calendar_;

    // -- Methods

    void print(std::ostream&) const override;

    template <class T>
    void _decode(std::vector<T>& v) const;

    void decode(std::vector<double>&) const override;
    void decode(std::vector<float>&) const override;
    void decode(std::vector<long>&) const override;
    void decode(std::vector<short>&) const override;
    void decode(std::vector<unsigned char>&) const override;
    void decode(std::vector<long long>&) const override;

    void encode(std::vector<double>&) const override;
    void encode(std::vector<float>&) const override;
    void encode(std::vector<long>&) const override;
    void encode(std::vector<short>&) const override;
    void encode(std::vector<unsigned char>&) const override;
    void encode(std::vector<long long>&) const override;

    void addAttributes(Variable&) const override;
    void updateAttributes(int nc, int varid, const std::string& path) override;
};


}  // namespace mir::netcdf
