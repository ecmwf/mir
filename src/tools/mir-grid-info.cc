/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <queue>

#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/VectorOption.h"

#include "mir/namedgrids/NamedGrid.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/tools/MIRTool.h"


using namespace mir;


class MIRGridInfo : public tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
public:
    MIRGridInfo(int argc, char **argv) : tools::MIRTool(argc, argv) {
        options_.push_back(new eckit::option::VectorOption<double>("grid", "Provid the West-East & South-North increments", 2));
        options_.push_back(new eckit::option::SimpleOption<std::string>("gridname", "Provide grid name"));
        options_.push_back(new eckit::option::VectorOption<double>("area", "Specify the area: north/west/south/east", 4));
    }
};


void MIRGridInfo::usage(const std::string &tool) const {
    eckit::Log::info() << tool << " --area=n/w/s/e --gridname=name|--grid=we/sn" << std::endl;
}

template<class T>
static
T abs(const T& x) {
    if (x > 0) {
        return x;
    }
    return 0 - x;
}

template<class T>
struct Sorter {
    T ref_;

    T above_;
    T below_;

    T dabove_;
    T dbelow_;

    Sorter(const T& ref): ref_(ref),
        above_(-90),
        below_(90),
        dabove_(90),
        dbelow_(-90) {}

    void push(const T& l) {
        auto x = l - ref_;
        if (x >= 0) {
            if (x < dabove_) {
                above_ = l;
                dabove_ = x;
            }
        }
        if (x <= 0) {
            if (x > dbelow_) {
                below_ = l;
                dbelow_ = x;
            }
        }
    }
};


void MIRGridInfo::execute(const eckit::option::CmdArgs& args) {

    std::vector<double> value;
    util::BoundingBox bbox;

    eckit::ScopedPtr<repres::Iterator> iterator;

    if (args.get("area", value)) {
        ASSERT(value.size() == 4);
        bbox = util::BoundingBox(value[0], value[1], value[2], value[3]);
    }


    if (args.get("grid", value)) {
        ASSERT(value.size() == 2);
        util::BoundingBox globe(90, 0, -90, 360 - value[0]);
        util::Increments increments(value[0], value[1]);
        repres::latlon::RegularLL ll(globe, increments);
        repres::Representation& r = ll;
        iterator.reset(r.iterator());
    }

    std::string gridname;
    if (args.get("gridname", gridname)) {
        auto& grid = namedgrids::NamedGrid::lookup(gridname);
        iterator.reset(grid.representation()->iterator());
    }

    ASSERT(iterator.get());

    Sorter<Latitude> n(bbox.north());
    Sorter<Latitude> s(bbox.south());

    Sorter<Longitude> w(bbox.west());
    Sorter<Longitude> e(bbox.east());

    while (iterator->next()) {
        const repres::Iterator::point_ll_t& point = iterator->pointUnrotated();

        n.push(point.lat);
        s.push(point.lat);

        w.push(point.lon);
        w.push(point.lon + 360);
        w.push(point.lon - 360);

        e.push(point.lon);
        e.push(point.lon + 360);
        e.push(point.lon - 360);

    }

    std::cout << "north " << n.above_ << ' ' << n.ref_ << ' ' << n.below_ << ' ' << n.dabove_ << ' ' << n.dbelow_ << std::endl;
    std::cout << "west " << w.above_ << ' ' << w.ref_ << ' ' << w.below_ << ' ' << w.dabove_ << ' ' <<  w.dbelow_ << std::endl;
    std::cout << "south " << s.above_ << ' ' << s.ref_ << ' ' << s.below_ << ' ' << s.dabove_ << ' ' << s.dbelow_ << std::endl;
    std::cout << "east " << e.above_ << ' ' << e.ref_ << ' ' << e.below_ << ' ' << e.dabove_ << ' ' <<  e.dbelow_ << std::endl;




}


int main(int argc, char **argv) {
    MIRGridInfo tool(argc, argv);
    return tool.start();
}

