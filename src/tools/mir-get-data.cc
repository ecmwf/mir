/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Jun 2017


#include <limits>
#include <map>
#include <vector>
#include <utility>

#include "eckit/log/Log.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/api/Atlas.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/stats/detail/ScalarMinMaxFn.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Grib.h"


using neighbours_t = std::vector<mir::search::PointSearch::PointValueType>;
using prec_t = decltype(std::cout.precision());


class MIRGetData : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int minimumPositionalArguments() const {
        return 1;
    }
public:
    MIRGetData(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using eckit::option::SimpleOption;
        using eckit::option::VectorOption;
        options_.push_back(new SimpleOption< bool >("diff-atlas", "compare to Atlas coordinates, default false"));
        options_.push_back(new SimpleOption< bool >("diff-ecc", "compare to ecCodes coordinates, default false"));
        options_.push_back(new SimpleOption< double >("tolerance-lat", "Latitude tolerance (absolute), default 0."));
        options_.push_back(new SimpleOption< double >("tolerance-lon", "Longitude tolerance (absolute), default 0."));
        options_.push_back(new VectorOption< double >("closest", "Point(s) close to given latitude/longitude", 2));
        options_.push_back(new SimpleOption< size_t >("nclosest", "Number of points close to given latitude/longitude, default 1"));
        options_.push_back(new SimpleOption< prec_t >("precision", "Output precision"));
    }
};


void MIRGetData::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nPrint a latitude, longitude, data values list."
               "\n"
               "\nUsage: " << tool << " [--diff-atlas=[true|false]] [--diff-ecc=[true|false]] file.grib [file.grib [...]]"
               "\nExamples:"
               "\n  % " << tool << " 1.grib"
               "\n  % " << tool << " --diff-atlas 1.grib 2.grib 3.grib"
            << std::endl;
}


using coord_t = std::vector<double>;


struct Coordinates {
    Coordinates(const std::string&& name) : name_(name) {}
    virtual ~Coordinates() = default;
    virtual const coord_t& latitudes() const = 0;
    virtual const coord_t& longitudes() const = 0;
    const std::string& name() const {
        return name_;
    }
    size_t size() const {
        ASSERT(latitudes().size() == longitudes().size());
        return latitudes().size();
    }
    const std::string name_;
};


struct CoordinatesFromRepresentation : Coordinates {
    CoordinatesFromRepresentation(const mir::repres::Representation& rep) :
        Coordinates("mir") {
        eckit::ScopedPtr< mir::repres::Iterator > it(rep.iterator());

        const size_t N(rep.numberOfPoints());
        lats_.assign(N, std::numeric_limits<double>::signaling_NaN());
        lons_.assign(N, std::numeric_limits<double>::signaling_NaN());

        size_t n = 0;
        while (it->next()) {
            ASSERT(n < N);
            lats_[n] = (*(*it))[0];
            lons_[n] = (*(*it))[1];
            ++n;
        }
    }
    const coord_t& latitudes() const { return lats_; }
    const coord_t& longitudes() const { return lons_; }
private:
    coord_t lats_;
    coord_t lons_;
};


struct CoordinatesFromGRIB : Coordinates {
    CoordinatesFromGRIB(grib_handle* h) :
        Coordinates("ecc") {

        long Nl = 0;
        grib_get_long(h, "numberOfValues", &Nl);
        ASSERT(Nl > 0);
        const size_t N = size_t(Nl);

        lats_.assign(N, std::numeric_limits<double>::signaling_NaN());
        lons_.assign(N, std::numeric_limits<double>::signaling_NaN());

        int err = 0;
        grib_iterator* it = grib_iterator_new(h, 0, &err);
        if (err != GRIB_SUCCESS) {
            GRIB_CHECK(err, nullptr);
        }

        size_t n = 0;
        for (double lat, lon, value; grib_iterator_next(it, &lat, &lon, &value);) {
            ASSERT(n < N);
            lats_[n] = lat;
            lons_[n] = lon;
            ++n;
        }

        grib_iterator_delete(it);
    }
    const coord_t& latitudes() const { return lats_; }
    const coord_t& longitudes() const { return lons_; }
private:
    coord_t lats_;
    coord_t lons_;
};


struct CoordinatesFromAtlas : Coordinates {
    CoordinatesFromAtlas(const atlas::Grid& grid) :
        Coordinates("atlas") {

        const size_t N = grid.size();
        lats_.assign(N, std::numeric_limits<double>::signaling_NaN());
        lons_.assign(N, std::numeric_limits<double>::signaling_NaN());

        size_t n = 0;
        for (const atlas::Grid::PointLonLat p : grid.lonlat()) {
            ASSERT(n < N);
            lats_[n] = p.lat();
            lons_[n] = p.lon();
            ++n;
        }
    }
    const coord_t& latitudes() const { return lats_; }
    const coord_t& longitudes() const { return lons_; }
private:
    coord_t lats_;
    coord_t lons_;
};




size_t diff(eckit::Channel& log,
          double toleranceLat,
          double toleranceLon,
          const Coordinates& coord1,
          const Coordinates& coord2) {

    using point_2d_t = mir::repres::Iterator::point_2d_t;

    ASSERT(coord1.size() == coord2.size());
    size_t N = coord1.size();

    const coord_t
            &lat1 = coord1.latitudes(),
            &lon1 = coord1.longitudes(),
            &lat2 = coord2.latitudes(),
            &lon2 = coord2.longitudes();

    mir::stats::detail::ScalarMinMaxFn<double> statsLat, statsLon;
    auto showPointAt = [&](std::ostream& out, size_t n) -> std::ostream& {
        return out << "\n\t@[0]=" << n
                   << '\t' << point_2d_t(lat1[n], lon1[n])
                   << '\t' << point_2d_t(lat2[n], lon2[n]);
    };

    size_t Ndiff = 0;
    for (size_t n = 0; n < N; ++n) {
        double dlat = mir::Latitude(lat1[n]).distance(lat2[n]).value();
        double dlon = mir::LongitudeDouble(lon1[n]).distance(lon2[n]).value();

        statsLat(dlat);
        statsLon(dlon);

        if (dlat > toleranceLat || dlon > toleranceLon) {
            ++Ndiff;
            showPointAt(log, n);
        }
    }

    log << "\n|" << coord1.name() << " - " << coord2.name() << "|: #Δ = " << Ndiff << " of " << N;

    if (Ndiff && statsLat.max() > toleranceLat) {
        showPointAt(log, statsLat.maxIndex() - 1) << " <- max(|Δlat|) = " << statsLat.max();
    }
    if (Ndiff && statsLon.max() > toleranceLon) {
        showPointAt(log, statsLon.maxIndex() - 1) << " <- max(|Δlon|) = " << statsLon.max();
    }
    log << std::endl;

    return Ndiff;
}


const neighbours_t& getNeighbours(const eckit::geometry::Point2& p, size_t n, const mir::repres::Representation& rep, const mir::param::MIRParametrisation& param) {
    static std::map<std::string, neighbours_t> cache;

    auto& key = rep.uniqueName();
    auto cached = cache.find(key);
    if (cached != cache.end()) {
        return cached->second;
    }

    mir::search::PointSearch::PointType pt;
    atlas::util::Earth::convertSphericalToCartesian(p, pt);

    mir::search::PointSearch sptree(param, rep);

    neighbours_t closest;
    sptree.closestNPoints(pt, n, closest);
    ASSERT(n == closest.size());

    return (cache[key] = std::move(closest));
}


void MIRGetData::execute(const eckit::option::CmdArgs& args) {
    using eckit::geometry::Point2;
    using eckit::geometry::Point3;
    using mir::repres::Iterator;

    auto& log = eckit::Log::info();

    prec_t precision;
    auto old = args.get("precision", precision) ? log.precision(precision)
                                                : log.precision();

    const mir::param::ConfigurationWrapper args_wrap(args);

    bool ecc = false;
    args.get("diff-ecc", ecc);


    bool atlas = false;
    args.get("diff-atlas", atlas);


    double toleranceLat = 0.;
    args.get("tolerance-lat", toleranceLat);
    ASSERT(toleranceLat >= 0.);


    double toleranceLon = 0.;
    args.get("tolerance-lon", toleranceLon);
    ASSERT(toleranceLon >= 0.);


    size_t nclosest = 1;
    args.get("nclosest", nclosest);


    Point2 p;
    std::vector< double > closest;
    if (args.get("closest", closest)) {
        ASSERT(closest.size() == 2);
        p = Point2(closest[1], closest[0]);
    } else {
        nclosest = 0;
    }

    for (size_t i = 0; i < args.count(); ++i) {
        mir::input::GribFileInput grib(args(i));
        const mir::input::MIRInput& input = grib;


        size_t count = 0;
        while (grib.next()) {
            log << "\n'" << args(i) << "' #" << ++count << std::endl;

            mir::data::MIRField field = input.field();
            ASSERT(field.dimensions() == 1);

            mir::repres::RepresentationHandle rep(field.representation());

            if (!atlas && !ecc && !nclosest) {
                eckit::ScopedPtr< Iterator > it(rep->iterator());
                for (const double& v: field.values(0)) {
                    ASSERT(it->next());
                    const Iterator::point_2d_t& P(**it);
                    log << "\n\t" << P[0] << '\t' << P[1] << '\t' << v;
                }

                log << std::endl;
                ASSERT(!it->next());

                continue;
            }

            eckit::ScopedPtr<Coordinates> crd(new CoordinatesFromRepresentation(*rep));

            if (nclosest) {
                size_t c = 1;
                for (auto& n : getNeighbours(p, nclosest, *rep, args_wrap)) {
                    size_t i = n.payload();
                    Point2 q(crd->longitudes()[i], crd->latitudes()[i]);

                    log << "- " << c++ << " -"
                        << " index=" << i
                        << " latitude=" << q[1]
                        << " longitude=" << q[0]
                        << " distance=" << atlas::util::Earth::distance(p, q) / 1000. << " (Km)"
                        << std::endl;
                }
            }

            bool err = false;
            if (atlas) {
                eckit::ScopedPtr<Coordinates> atl(new CoordinatesFromAtlas(rep->atlasGrid()));
                err = diff(log, toleranceLat, toleranceLon, *crd, *atl);
            }

            if (ecc) {
                eckit::ScopedPtr<Coordinates> ecc(new CoordinatesFromGRIB(input.gribHandle()));
                err = diff(log, toleranceLat, toleranceLon, *crd, *ecc) || err;
            }

            if (err) {
                throw eckit::UserError("Comparison failed");
            }
        }
    }

    log.precision(old);
}


int main(int argc, char **argv) {
    MIRGetData tool(argc, argv);
    return tool.start();
}

