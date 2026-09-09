// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>


namespace mir::util {


class Error {
    void print(std::ostream&) const;
    friend std::ostream& operator<<(std::ostream& s, const Error& e) {
        e.print(s);
        return s;
    }
};


}  // namespace mir::util
