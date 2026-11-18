// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <fstream>
#include <functional>
#include <iostream>
#include <string>


namespace mir::action {


/// Append a line to a file, or to the standard output ("-")
inline void append_line(const std::string& path, const std::function<void(std::ostream&)>& write) {
    if (path == "-") {
        write(std::cout);
        std::cout << std::endl;
        return;
    }

    std::ofstream out(path, std::ios::app);
    write(out);
    out << std::endl;
}


}  // namespace mir::action
