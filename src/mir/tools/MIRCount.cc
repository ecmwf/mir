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

#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/Increments.h"

#include "mir/tools/MIRCount.h"


namespace mir {
namespace tools {


using prec_t = decltype(std::cout.precision());


void MIRCount::usage(const std::string& tool) const {
    eckit::Log::info() << "\nCount MIR representation number of values, compared to the GRIB numberOfValues."
                          "\n"
                          "\nUsage:"
                          "\n\t"
                       << tool
                       << " [--area=N/W/S/E] file.grib [file.grib [...]]"
                          "\n\t"
                       << tool
                       << " [--area=N/W/S/E] --grid=WE/SN"
                          "\n\t"
                       << tool
                       << " [--area=N/W/S/E] --grid=WE/SN --ni-nj"
                          "\n\t"
                       << tool
                       << " [--area=N/W/S/E] --gridname=[FNOfno][1-9][0-9]*  # ..."
                          "\n"
                          "\n"
                          "Examples:"
                          "\n\t"
                          "% "
                       << tool
                       << " 1.grib"
                          "\n\t"
                          "% "
                       << tool
                       << " --area=6/0/0/6 1.grib 2.grib"
                          "\n\t"
                          "% "
                       << tool
                       << " --area=6/0/0/6 --gridname=O1280"
                          "\n\t"
                          "% "
                       << tool << " --area=6/0/0/6 --grid=1/1 --ni-nj" << std::endl;
}


template <class T>
std::ostream& operator<<(std::ostream& s, const std::set<std::pair<T, T> >& x) {
    size_t i = 0;
    for (auto e : x) {
        s << ' ' << e.first << " (" << e.second << ")";
        if (++i >= 2) {
            break;
        }
    }
    return s;
}


util::BoundingBox get_bounding_box(const std::vector<double>& area) {
    if (area.empty()) {
        return {};
    }
    ASSERT(area.size() == 4);
    return {area[0], area[1], area[2], area[3]};
}


MIRCount::counter_t::counter_t(const util::BoundingBox& bbox_) :
    bbox(bbox_),
    first(true),
    count(0),
    values(0),
    n(0),
    s(0),
    e(0),
    w(0) {}


MIRCount::counter_t::counter_t(std::vector<double>& area) : counter_t(get_bounding_box(area)) {}


void MIRCount::counter_t::insert(const PointLatLon& point) {
    values++;

    nn.insert(DistanceLat(bbox.north().distance(point.lat()), point.lat()));
    ss.insert(DistanceLat(bbox.south().distance(point.lat()), point.lat()));

    ee.insert(DistanceLon(bbox.east().distance(point.lon()), point.lon()));
    ww.insert(DistanceLon(bbox.west().distance(point.lon()), point.lon()));

    // std::cout << point.lat << " " << point.lon << " => " << bbox.contains(point.lat, point.lon) << std::endl;

    if (bbox.contains(point)) {

        const Latitude& lat = point.lat();
        const Longitude lon = point.lon().normalise(bbox.west());

        if (first) {
            n = s = lat;
            e = w = lon;
            first = false;
        }
        else {
            if (n < lat) {
                n = lat;
            }
            if (s > lat) {
                s = lat;
            }
            if (e < lon) {
                e = lon;
            }
            if (w > lon) {
                w = lon;
            }
        }


        count++;
    }
}


void MIRCount::counter_t::json(eckit::JSON& j, bool enclose) const {
    if (enclose) {
        j.startObject();
    }

    j << "count" << count;
    j << "values" << values;

    j << "point";
    j.startObject();
    j << "n" << n.value();
    j << "w" << w.value();
    j << "s" << s.value();
    j << "e" << e.value();
    j.endObject();

    j << "bbox";
    j.startObject();
    j << "n" << bbox.north().value();
    j << "w" << bbox.west().value();
    j << "s" << bbox.south().value();
    j << "e" << bbox.east().value();
    j.endObject();

    j << "distance_to_bbox";
    j.startObject();
    j << "n" << (bbox.north() - n).value();
    j << "w" << (w - bbox.west()).value();
    j << "s" << (s - bbox.south()).value();
    j << "e" << (bbox.east() - e).value();
    j.endObject();

    if (!nn.empty() && !ww.empty() && !ss.empty() && !ee.empty()) {
        j << "distance_to_closest";
        j.startObject();
        j << "n" << (nn.begin()->first).value();
        j << "w" << (ww.begin()->first).value();
        j << "s" << (ss.begin()->first).value();
        j << "e" << (ee.begin()->first).value();
        j.endObject();
    }

    if (enclose) {
        j.endObject();
    }
}


void countRepresentationInBoundingBox(MIRCount::counter_t& counter, const repres::Representation& rep) {
    for (std::unique_ptr<repres::Iterator> iter(rep.iterator()); iter->next();) {
        counter.insert(iter->pointUnrotated());
    }
}


void MIRCount::execute(const eckit::option::CmdArgs& args) {
    auto& log = eckit::Log::info();
    eckit::JSON j(log);

    prec_t precision;
    args.get("precision", precision) ? log.precision(precision) : log.precision();

    std::vector<double> area;
    args.get("area", area);


    // setup a regular lat/lon representation and perfom count
    std::vector<double> grid;
    if (args.get("grid", grid)) {
        ASSERT(!args.has("gridname"));
        counter_t counter(area);

        countOnGridIncrements(counter, grid);

        counter.json(j);
        return;
    }


    // setup a representation from gridname and perfom count
    std::string gridname;
    if (args.get("gridname", gridname)) {
        ASSERT(!args.has("grid"));
        counter_t counter(area);

        countOnNamedGrid(counter, gridname);

        counter.json(j);
        return;
    }


    // count each file(s) message(s)
    util::BoundingBox bbox(get_bounding_box(area));

    j.startObject();
    j << "files";
    j.startList();

    for (size_t i = 0, k = 0; i < args.count(); ++i, k = 0) {

        input::GribFileInput grib(args(i));
        while (grib.next()) {
            ++k;

            data::MIRField field = static_cast<const input::MIRInput&>(grib).field();
            ASSERT(field.dimensions() == 1);

            repres::RepresentationHandle rep(field.representation());
            counter_t counter(bbox);

            countRepresentationInBoundingBox(counter, *rep);

            j.startObject();
            j << "file" << args(i);
            j << "fileMessage" << k;
            counter.json(j, false);
            j.endObject();
        }
    }

    j.endList();
    j.endObject();
}


void MIRCount::addOptions(std::vector<eckit::option::Option*>& options) {
    using eckit::option::SimpleOption;
    using eckit::option::VectorOption;

    // options_.push_back(new SimpleOption< bool >("sizes", "compare sizes of coordinates and values vectors,
    // default false"));
    options.push_back(new VectorOption<double>("area", "cropping area (North/West/South/East)", 4));
    options.push_back(new SimpleOption<std::string>("gridname", "grid name: [FNOfno][1-9][0-9]*"));
    options.push_back(new VectorOption<double>("grid", "regular grid increments (West-East/South-North)", 2));
    options.push_back(new SimpleOption<prec_t>("precision", "Output precision"));
}


void MIRCount::countOnNamedGrid(counter_t& counter, std::string grid) {
    ASSERT(!grid.empty());
    repres::RepresentationHandle rep(namedgrids::NamedGrid::lookup(grid).representation());

    countRepresentationInBoundingBox(counter, *rep);
}


void MIRCount::countOnGridIncrements(counter_t& counter, std::vector<double> grid) {
    ASSERT(grid.size() == 2);
    util::Increments inc(grid[0], grid[1]);

    repres::latlon::RegularLL rep(inc);

    countRepresentationInBoundingBox(counter, rep);
}


MIRCount::MIRCount(int argc, char** argv) : MIRTool(argc, argv) {
    addOptions(options_);
}


MIRCount::~MIRCount() = default;


}  // namespace tools
}  // namespace mir
