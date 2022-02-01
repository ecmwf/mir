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


#include <memory>

#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/key/grid/Grid.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace tools {


struct MIRGridInfo : MIRTool {
    MIRGridInfo(int argc, char** argv) : MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(new VectorOption<double>("grid", "West-East & South-North increments", 2));
        options_.push_back(new SimpleOption<std::string>("gridname", "grid name"));
        options_.push_back(new VectorOption<double>("area", "North/West/South/East", 4));
    }

    void usage(const std::string& tool) const override {
        Log::info() << tool << " --area=n/w/s/e --gridname=name|--grid=we/sn" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& /*args*/) override;
};


template <class T>
struct Sorter {
    T ref_;

    T above_;
    T below_;

    T dabove_;
    T dbelow_;

    Sorter(const T& ref) : ref_(ref), above_(-90), below_(90), dabove_(90), dbelow_(-90) {}

    void push(const T& l) {
        auto x = l - ref_;
        if (x >= 0) {
            if (x < dabove_) {
                above_  = l;
                dabove_ = x;
            }
        }
        if (x <= 0) {
            if (x > dbelow_) {
                below_  = l;
                dbelow_ = x;
            }
        }
    }
};


void MIRGridInfo::execute(const eckit::option::CmdArgs& args) {
    auto& log = Log::info();

    if (args.count() > 0) {
        const param::ConfigurationWrapper args_wrap(args);

        for (size_t i = 0, j = 0; i < args.count(); ++i) {
            std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(args(i), args_wrap));
            while (input->next()) {
                repres::RepresentationHandle repres(input->field().representation());
                log << "#" << ++j << ": " << *repres << std::endl;
            }
        }

        return;
    }

    std::vector<double> value;
    if (args.get("grid", value)) {
        ASSERT(value.size() == 2);
    }

    std::string gridname;
    repres::RepresentationHandle rep(
        args.has("grid")                 ? new repres::latlon::RegularLL(util::Increments(value[0], value[1]))
        : args.get("gridname", gridname) ? key::grid::Grid::lookup(gridname).representation()
                                         : throw exception::UserError("'grid' or 'gridname' should be provided"));
    ASSERT(rep);


    util::BoundingBox bbox;
    if (args.get("area", value)) {
        ASSERT(value.size() == 4);
        bbox = util::BoundingBox(value[0], value[1], value[2], value[3]);
    }

    Sorter<Latitude> n(bbox.north());
    Sorter<Latitude> s(bbox.south());

    Sorter<Longitude> w(bbox.west());
    Sorter<Longitude> e(bbox.east());

    for (const std::unique_ptr<repres::Iterator> it(rep->iterator()); it->next();) {
        const auto& p = it->pointUnrotated();

        n.push(p.lat());
        s.push(p.lat());

        w.push(p.lon());
        w.push(p.lon() + Longitude::GLOBE);
        w.push(p.lon() - Longitude::GLOBE);

        e.push(p.lon());
        e.push(p.lon() + Longitude::GLOBE);
        e.push(p.lon() - Longitude::GLOBE);
    }

    log << "north " << n.above_ << ' ' << n.ref_ << ' ' << n.below_ << ' ' << n.dabove_ << ' ' << n.dbelow_
        << "\n"
           "west "
        << w.above_ << ' ' << w.ref_ << ' ' << w.below_ << ' ' << w.dabove_ << ' ' << w.dbelow_
        << "\n"
           "south "
        << s.above_ << ' ' << s.ref_ << ' ' << s.below_ << ' ' << s.dabove_ << ' ' << s.dbelow_
        << "\n"
           "east "
        << e.above_ << ' ' << e.ref_ << ' ' << e.below_ << ' ' << e.dabove_ << ' ' << e.dbelow_ << std::endl;
}


}  // namespace tools
}  // namespace mir


int main(int argc, char** argv) {
    mir::tools::MIRGridInfo tool(argc, argv);
    return tool.start();
}
