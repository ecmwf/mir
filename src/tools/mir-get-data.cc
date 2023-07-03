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


#include <algorithm>
#include <limits>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/stats/detail/Counter.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir::tools {


using coord_t      = std::vector<double>;
using neighbours_t = std::vector<search::PointSearch::PointValueType>;
using prec_t       = decltype(Log::info().precision());


struct MIRGetData : MIRTool {
    MIRGetData(int argc, char** argv) : MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(new SimpleOption<bool>("diff-atlas", "compare to Atlas coordinates, default false"));
        options_.push_back(new SimpleOption<bool>("diff-ecc", "compare to ecCodes coordinates, default false"));
        options_.push_back(new SimpleOption<double>("tolerance-lat", "Latitude tolerance (absolute), default 0."));
        options_.push_back(new SimpleOption<double>("tolerance-lon", "Longitude tolerance (absolute), default 0."));
        options_.push_back(new VectorOption<double>("closest", "Point(s) close to given latitude/longitude", 2));
        options_.push_back(
            new SimpleOption<size_t>("nclosest", "Number of points close to given latitude/longitude, default 1"));
        options_.push_back(new SimpleOption<prec_t>("precision", "Output precision"));
    }

    int minimumPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nPrint a latitude, longitude, data values list."
                       "\n"
                       "\nUsage: "
                    << tool
                    << " [--diff-atlas=[true|false]] [--diff-ecc=[true|false]] file.grib [file.grib [...]]"
                       "\nExamples:"
                       "\n  % "
                    << tool
                    << " 1.grib"
                       "\n  % "
                    << tool << " --diff-atlas 1.grib 2.grib 3.grib" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& /*args*/) override;
};


struct Coordinates {
    Coordinates(const std::string&& name) : name_(name) {}
    virtual ~Coordinates() = default;

    Coordinates(const Coordinates&)            = delete;
    Coordinates(Coordinates&&)                 = delete;
    Coordinates& operator=(const Coordinates&) = delete;
    Coordinates& operator=(Coordinates&&)      = delete;

    virtual const coord_t& latitudes() const  = 0;
    virtual const coord_t& longitudes() const = 0;
    const std::string& name() const { return name_; }
    size_t size() const {
        ASSERT(latitudes().size() == longitudes().size());
        return latitudes().size();
    }

    const std::string name_;
};


struct CoordinatesFromRepresentation : Coordinates {
    CoordinatesFromRepresentation(const repres::Representation& rep) : Coordinates("mir") {
        const size_t N(rep.numberOfPoints());
        lats_.assign(N, std::numeric_limits<double>::signaling_NaN());
        lons_.assign(N, std::numeric_limits<double>::signaling_NaN());

        for (const std::unique_ptr<repres::Iterator> it(rep.iterator()); it->next();) {
            const Point2& P(**it);
            lats_.at(it->index()) = P[0];
            lons_.at(it->index()) = P[1];
        }
    }
    const coord_t& latitudes() const override { return lats_; }
    const coord_t& longitudes() const override { return lons_; }

private:
    coord_t lats_;
    coord_t lons_;
};


struct CoordinatesFromGRIB : Coordinates {
    CoordinatesFromGRIB(grib_handle* h) : Coordinates("ecc") {

        long Nl = 0;
        codes_get_long(h, "numberOfDataPoints", &Nl);
        ASSERT(Nl > 0);
        auto N = size_t(Nl);

        lats_.assign(N, std::numeric_limits<double>::signaling_NaN());
        lons_.assign(N, std::numeric_limits<double>::signaling_NaN());

        int err = 0;
        auto it = codes_grib_iterator_new(h, 0, &err);
        if (err != CODES_SUCCESS) {
            CODES_CHECK(err, nullptr);
        }

        size_t n = 0;
        for (double lat, lon, value; codes_grib_iterator_next(it, &lat, &lon, &value) != 0;) {
            ASSERT(n < N);
            lats_[n] = lat;
            lons_[n] = lon;
            ++n;
        }

        codes_grib_iterator_delete(it);
    }
    const coord_t& latitudes() const override { return lats_; }
    const coord_t& longitudes() const override { return lons_; }

private:
    coord_t lats_;
    coord_t lons_;
};


struct CoordinatesFromAtlas : Coordinates {
    CoordinatesFromAtlas(const atlas::Grid& grid) : Coordinates("atlas") {
#if mir_HAVE_ATLAS
        auto N = size_t(grid.size());
        lats_.assign(N, std::numeric_limits<double>::signaling_NaN());
        lons_.assign(N, std::numeric_limits<double>::signaling_NaN());

        size_t n = 0;
        for (auto p : grid.lonlat()) {
            ASSERT(n < N);
            lats_[n] = p.lat();
            lons_[n] = p.lon();
            ++n;
        }
#else
        NOTIMP;
#endif
    }
    const coord_t& latitudes() const override { return lats_; }
    const coord_t& longitudes() const override { return lons_; }

private:
    coord_t lats_;
    coord_t lons_;
};


size_t diff(Log::Channel& out, double toleranceLat, double toleranceLon, const Coordinates& coord1,
            const Coordinates& coord2) {

    ASSERT(coord1.size() == coord2.size());
    size_t N = coord1.size();

    const coord_t& lat1 = coord1.latitudes();
    const coord_t& lon1 = coord1.longitudes();
    const coord_t& lat2 = coord2.latitudes();
    const coord_t& lon2 = coord2.longitudes();

    param::SimpleParametrisation empty;

    stats::detail::Counter statsLat(empty);
    stats::detail::Counter statsLon(empty);

    auto showPointAt = [&](std::ostream& out, size_t n) -> std::ostream& {
        return out << "\n\t@[0]=" << n << '\t' << Point2(lat1[n], lon1[n]) << '\t' << Point2(lat2[n], lon2[n]);
    };

    auto showCoordMinMax = [&](std::ostream& out, const std::string& name, const coord_t& c) -> std::ostream& {
        ASSERT(!c.empty());
        auto mm = std::minmax_element(c.begin(), c.end());
        return out << "\n\t" << name << " min/max =\t" << *(mm.first) << " /\t" << *(mm.second);
    };

    size_t Ndiff = 0;
    for (size_t n = 0; n < N; ++n) {
        double dlat = Latitude(lat1[n]).distance(lat2[n]).value();
        double dlon = LongitudeDouble(lon1[n]).distance(lon2[n]).value();

        statsLat.count(dlat);
        statsLon.count(dlon);

        if (dlat > toleranceLat || dlon > toleranceLon) {
            ++Ndiff;
            showPointAt(out, n);
        }
    }

    out << "\n|" << coord1.name() << " - " << coord2.name() << "|: #Δ = " << Ndiff << " of " << N;

    if (Ndiff > 0) {
        if (statsLat.max() > toleranceLat) {
            showPointAt(out, statsLat.maxIndex() - 1) << " <- max(|Δlat|) = " << statsLat.max();
            showCoordMinMax(out, coord1.name() + " latitude", coord1.latitudes());
            showCoordMinMax(out, coord2.name() + " latitude", coord2.latitudes());
        }
        if (statsLon.max() > toleranceLon) {
            showPointAt(out, statsLon.maxIndex() - 1) << " <- max(|Δlon|) = " << statsLon.max();
            showCoordMinMax(out, coord1.name() + " longitude", coord1.longitudes());
            showCoordMinMax(out, coord2.name() + " longitude", coord2.longitudes());
        }
    }
    out << std::endl;

    return Ndiff;
}


const neighbours_t& getNeighbours(Point2 p, size_t n, const repres::Representation& rep,
                                  const param::MIRParametrisation& param) {
    static std::map<std::string, neighbours_t> cache;

    const auto& key = rep.uniqueName();
    auto cached     = cache.find(key);
    if (cached != cache.end()) {
        return cached->second;
    }

    search::PointSearch::PointType pt = util::Earth::convertSphericalToCartesian({p[1], p[0]});

    search::PointSearch sptree(param, rep);

    neighbours_t closest;
    sptree.closestNPoints(pt, n, closest);
    ASSERT(n == closest.size());

    return (cache[key] = std::move(closest));
}


void MIRGetData::execute(const eckit::option::CmdArgs& args) {
    auto& log = Log::info();

    prec_t precision;
    auto old = args.get("precision", precision) ? log.precision(precision) : log.precision();

    const param::ConfigurationWrapper args_wrap(args);

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
    std::vector<double> closest;
    if (args.get("closest", closest)) {
        ASSERT(closest.size() == 2);
        p = Point2(closest[1], closest[0]);
    }
    else {
        nclosest = 0;
    }

    for (size_t a = 0; a < args.count(); ++a) {
        std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(args(a), args_wrap));
        ASSERT(input);

        size_t count = 0;
        while (input->next()) {
            log << "\n'" << args(a) << "' #" << ++count << std::endl;

            auto field = input->field();
            ASSERT(field.dimensions() == 1);

            const auto& values = field.values(0);

            repres::RepresentationHandle rep(field.representation());

            if (!atlas && !ecc && (nclosest == 0)) {
                for (const std::unique_ptr<repres::Iterator> it(rep->iterator()); it->next();) {
                    const Point2& P(**it);
                    log << "\t" << P[0] << '\t' << P[1] << '\t' << values.at(it->index()) << std::endl;
                }

                log << std::endl;
                continue;
            }

            std::unique_ptr<Coordinates> crd(new CoordinatesFromRepresentation(*rep));

            if (nclosest > 0) {
                size_t c = 1;
                for (const auto& n : getNeighbours(p, nclosest, *rep, args_wrap)) {
                    size_t i = n.payload();
                    eckit::geometry::PointLonLat q(crd->longitudes()[i], crd->latitudes()[i]);
                    ASSERT(i < values.size());

                    auto distance = util::Earth::distance({p.x(), p.y()}, q);

                    constexpr double THOUSAND = 1000;
                    log << "- " << c++ << " -"
                        << " index=" << i << " latitude=" << q.lat << " longitude=" << q.lon
                        << " distance=" << distance / THOUSAND << " (km)"
                        << " value=" << values[i] << std::endl;
                }
            }

            bool err = false;
            if (atlas) {
                std::unique_ptr<Coordinates> atl(new CoordinatesFromAtlas(rep->atlasGrid()));
                err = diff(log, toleranceLat, toleranceLon, *crd, *atl) != 0;
            }

            if (ecc) {
                std::unique_ptr<Coordinates> ecd(new CoordinatesFromGRIB(input->gribHandle()));
                err = diff(log, toleranceLat, toleranceLon, *crd, *ecd) != 0 || err;
            }

            if (err) {
                throw exception::UserError("Comparison failed");
            }
        }
    }

    log.precision(old);
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRGetData tool(argc, argv);
    return tool.start();
}
