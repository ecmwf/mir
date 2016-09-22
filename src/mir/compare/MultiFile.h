/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file   MultiFile.h
/// @author Florian Rathgeber
/// @date   Jul 2016

#ifndef mir_MultiFile_H
#define mir_MultiFile_H

#include <map>
#include <string>
#include <vector>

namespace eckit {
class Stream;
}

namespace mir {
namespace compare {

//----------------------------------------------------------------------------------------------------------------------


class MultiFile {
public:

    MultiFile(const std::string& path);
    MultiFile(const std::string& name, const std::string& from);
    MultiFile(eckit::Stream& s) ;

    bool operator<(const MultiFile& other) const ;

    void add(const std::string& path);

    const std::vector<std::string>& paths() const;

    void encode(eckit::Stream& s) const ;

    void print(std::ostream& out)  const;

    void save() const;

private:

    std::string name_;
    std::string from_;
    std::vector<std::string> paths_;

private:

    friend eckit::Stream& operator<<(eckit::Stream& s, const MultiFile& x) {
        x.encode(s);
        return s;
    }

    friend std::ostream& operator<<(std::ostream& out, const MultiFile& x) {
        x.print(out);
        return out;
    }




};


//----------------------------------------------------------------------------------------------------------------------

} // namespace compare
} // namespace mir

#endif
