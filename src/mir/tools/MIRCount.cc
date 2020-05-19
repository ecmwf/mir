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
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"

#include "mir/tools/MIRCount.h"


namespace mir {
namespace tools {


using prec_t = decltype(std::cout.precision());

using DistanceLat = std::pair<Latitude, Latitude>;
using DistanceLon = std::pair<Longitude, Longitude>;


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


struct counter_t {
    counter_t(const util::BoundingBox& bbox_) : bbox(bbox_), first(true), count(0), values(0), n(0), s(0), e(0), w(0) {}

    void insert(const PointLatLon& point) {
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

    void json(eckit::JSON& j, bool enclose = true) const {
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

    const util::BoundingBox bbox;
    bool first;

    size_t count;
    size_t values;

    Latitude n;
    Latitude s;
    Longitude e;
    Longitude w;

    std::set<DistanceLat> nn;
    std::set<DistanceLat> ss;
    std::set<DistanceLon> ww;
    std::set<DistanceLon> ee;
};


void countRepresentationInBoundingBox(counter_t& counter, const repres::Representation& rep) {
    for (std::unique_ptr<repres::Iterator> iter(rep.iterator()); iter->next();) {
        counter.insert(iter->pointUnrotated());
    }
}


void MIRCount::execute(const eckit::option::CmdArgs& args) {
    auto& log = eckit::Log::info();

    prec_t precision;
    args.get("precision", precision) ? log.precision(precision) : log.precision();
    eckit::JSON j(log);

    std::vector<double> value;

    util::BoundingBox bbox;
    if (args.get("area", value)) {
        ASSERT(value.size() == 4);
        bbox = util::BoundingBox(value[0], value[1], value[2], value[3]);
    }


    // setup a regular lat/lon representation and perfom count
    if (args.get("grid", value)) {
        ASSERT(!args.has("gridname"));
        ASSERT(value.size() == 2);
        util::Increments grid(value[0], value[1]);

        if (args.has("ni-nj")) {
            // Note: this *does not crop*, it is a "local" representation
            repres::latlon::RegularLL rep(grid, bbox, {bbox.south(), bbox.west()});
            j.startObject();
            j << "Ni" << rep.Ni();
            j << "Nj" << rep.Nj();
            j.endObject();
            return;
        }

        repres::latlon::RegularLL rep(grid);
        counter_t counter(bbox);

        countRepresentationInBoundingBox(counter, rep);

        counter.json(j);
        return;
    }


    // setup a representation from gridname and perfom count
    std::string gridname;
    if (args.get("gridname", gridname)) {
        ASSERT(!args.has("grid"));

        repres::RepresentationHandle rep(namedgrids::NamedGrid::lookup(gridname).representation());
        counter_t counter(bbox);

        countRepresentationInBoundingBox(counter, *rep);

        counter.json(j);
        return;
    }


    // count each file(s) message(s)
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
    options.push_back(new SimpleOption<bool>("ni-nj", "output number of increments in longitude/latitude (Ni:Nj)"));
    options.push_back(new SimpleOption<prec_t>("precision", "Output precision"));
}


MIRCount::MIRCount(int argc, char** argv) : MIRTool(argc, argv) {
    addOptions(options_);
}


MIRCount::~MIRCount() = default;


}  // namespace tools
}  // namespace mir
