/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <algorithm>
#include <memory>

#include "eckit/exception/Exceptions.h"
#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/linalg/SparseMatrix.h"
#include "eckit/linalg/Vector.h"
#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "eckit/log/ProgressTimer.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"
#include "eckit/log/TraceTimer.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"

#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/method/WeightMatrix.h"
#include "mir/output/GribFileOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/search/Tree.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Domain.h"
#include "mir/util/MIRStatistics.h"


using namespace mir;
using namespace eckit::linalg;


class MIRClimateFilter : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);

    int numberOfPositionalArguments() const { return 2; }

    void usage(const std::string& tool) const {
        eckit::Log::info() << "\nUsage: " << tool
#if 0
                           << " --grid=WE/SN|--gridname=<namedgrid>|--griddef=<path> [--key1=value [--key2=value [...]]] input.grib output.grib"
                              "\nExamples: "
                              "\n  % "
                           << tool << " --grid=1/1 --area=90/-180/-90/179 in out"
#endif
                           << std::endl;
    }

public:
    MIRClimateFilter(int argc, char** argv) : tools::MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<size_t>("multiply-on-nnz", "Number of non-zeros to trigger interpolation"));
        options_.push_back(new eckit::option::SimpleOption<size_t>("nclosest", "Number of points neighbours to weight (k)"));
        options_.push_back(new eckit::option::SimpleOption<double>("distance", "Radius [m] of neighbours to weight (k) (default 1.)"));
        options_.push_back(new eckit::option::SimpleOption<double>("delta", "Climate filter (topographic data smoothing operator) width of filter edge, must be greater than 'distance' (default 1000.)"));
        options_.push_back(new eckit::option::FactoryOption<LinearAlgebra>("backend", "Linear algebra backend (default '" + LinearAlgebra::backend().name() + "')"));
        options_.push_back(new eckit::option::FactoryOption<search::TreeFactory>("point-search-trees", "k-d tree control"));
    }
};


struct BetterSearch : protected mir::search::PointSearch {
    using closest_t = std::vector<PointValueType>;

    using PointSearch::statsPrint;
    using PointSearch::statsReset;

    BetterSearch(const param::MIRParametrisation& param, const repres::Representation& repres) :
        PointSearch(param, repres),
        mode_(1) {}

    void closest(const PointType& p, size_t n, double radius, closest_t& closest) {
        closestNPoints(p, n, a_);

        auto trim = [=](closest_t& v) {
            if (v.size() > n) {
                v.assign(v.begin(), v.begin() + n);
            }
        };

        if (a_.size() >= n) {
            trim(a_);
            a_.swap(closest);
        }


        closestWithinRadius(p, radius, b_);

        (a_.size() >= b_.size() ? a_ : b_).swap(closest);

        trim(closest);

#if 0
        mode_ > 0 ? closestWithinRadius(p, distance_, closest) : closestNPoints(p, nclosest_, closest);
        if (closest.size() < nclosest_) {
            mode_ ? closestNPoints(p, nclosest_, b_) : closestWithinRadius(p, distance_, b_);

            if (closest.size() < b_.size()) {
                closest.swap(b_);
                mode_ = !mode_;
                eckit::Log::info() << "swaping to " << mode_ << std::endl;
            }

            if (closest.size() > nclosest_) {
                eckit::Log::info() << "clipping from" << closest.size() << " to " << nclosest_ << std::endl;
                auto end = closest.begin() + closest_t::difference_type(nclosest_);
                closest.assign(closest.begin(), end);
            }
        }
#endif
    }

private:
    closest_t a_;
    closest_t b_;
    int mode_;  // >= 0: radius search, otherwise n-closest search
};


struct SimpleTimer {
    SimpleTimer(std::string name) : name_(std::move(name)), t_(0) {}
    friend std::ostream& operator<<(std::ostream& s, SimpleTimer& p) {
        s << " " << p.name_ << ": " << eckit::Seconds(p.t_);
        p.t_ = 0;
        return s;
    }
    double& operator+=(double more) { return (t_ += more); }
private:
    std::string name_;
    double t_;
};


void MIRClimateFilter::execute(const eckit::option::CmdArgs& args) {
    eckit::ResourceUsage usage("MIRClimateFilter");
    eckit::TraceTimer<LibMir> timer("MIRClimateFilter");


    // linear algebra backend, statistics, input and output
    auto& la(LinearAlgebra::hasBackend("mkl") ? LinearAlgebra::getBackend("mkl") : LinearAlgebra::backend());

    util::MIRStatistics statistics;
    std::unique_ptr<input::MIRInput> in(new input::GribFileInput(args(0)));
    std::unique_ptr<output::MIROutput> out(new output::GribFileOutput(args(1)));
    ASSERT(in);
    ASSERT(out);

    auto& log = eckit::Log::info();
    static const param::DefaultParametrisation defaults;
    static const param::ConfigurationWrapper commandLine(args);


    int field = 0;
    while (in->next()) {
        log << "============> field: " << ++field << std::endl;

        // setup parametrisation
        std::unique_ptr<param::MIRParametrisation> param(
            new param::CombinedParametrisation(commandLine, in->parametrisation(), defaults));
        ASSERT(param);


        size_t nnz = 1000;
        param->get("multiply-on-nnz", nnz);
        ASSERT(nnz >= 2);

        size_t nclosest = 4;
        param->get("nclosest", nclosest);
        ASSERT(nclosest);

        double distance = 1.;
        param->get("distance", distance = 1.);
        ASSERT(distance > 0.);

        double delta = 1000.;
        param->get("delta", delta);
        ASSERT(delta > 0.);


        // set field (input values, copied to output)
        double t = timer.elapsed();
        context::Context ctx(*in, statistics);
        ASSERT(ctx.field().dimensions() == 1);


        auto& input = ctx.field().values(0);
        mir::MIRValuesVector output(input);

        repres::RepresentationHandle rep(ctx.field().representation());
        if (!rep->domain().isGlobal()) {
            throw eckit::UserError("MIRClimateFilter: input field should be global");
        }
        log << "field: " << eckit::Seconds(timer.elapsed() - t) << std::endl;


        // build/get k-d tree
        t = timer.elapsed();
        BetterSearch tree(*param, *rep);
        log << "k-d tree: " << eckit::Seconds(timer.elapsed() - t) << std::endl;


        // allocating temporary memory
        const auto n = rep->numberOfPoints();

        std::vector<search::PointSearch::PointValueType> closest;
        std::vector<method::WeightMatrix::Triplet> triplets;
        triplets.reserve(std::min(n * nclosest, size_t(nnz * 1.5)));

        {
            eckit::ProgressTimer progress("Locating", n, "point", double(5), log);
            size_t i = 0;

            SimpleTimer tClosest("closest");
            SimpleTimer tWeights("weights");
            SimpleTimer tMultipl("multiply");

            const std::unique_ptr<repres::Iterator> it(rep->iterator());
            while (it->next()) {
                if (++progress) {
                    log << PointLatLon(it->pointUnrotated()) << tClosest << tWeights << tMultipl << std::endl;
                    tree.statsPrint(log, false);
                    tree.statsReset();
                }

                ASSERT(i < n);
                const Point3 p(it->point3D());

                t = timer.elapsed();
//                log << "searching" << std::endl;
                tree.closest(p, nclosest, distance, closest);
                ASSERT(!closest.empty() && closest.size() <= nclosest);
                tClosest += timer.elapsed() - t;

                t = timer.elapsed();
                {
                    using method::WeightMatrix;

                    const size_t N = closest.size();

                    // calculate neighbour points weights, and their total (for normalisation)
                    std::vector<double> weights(N);
                    double sum = 0.;
                    for (size_t j = 0; j < N; ++j) {
                        auto r = Point3::distance(p, closest[j].point());
                        auto h = 0.5 + 0.5 * std::cos(M_PI_2 * (r - 0.5 * distance + delta) / delta);
                        h      = std::max(0., std::min(0.99, h));

                        weights[j] = h;
                        sum += h;
                    }

                    ASSERT(sum > 0.);

                    // normalise all weights according to the total, and set sparse matrix triplets
                    for (size_t j = 0; j < N; ++j) {
                        triplets.emplace_back(WeightMatrix::Triplet{i, closest[j].payload(), weights[j] / sum});
                    }
                }
                tWeights += timer.elapsed() - t;


                if (triplets.size() > nnz) {
                    t = timer.elapsed();

                    auto off = triplets.front().row();
                    for (auto& tri : triplets) {
                        tri.row() -= off;
                    }

                    SparseMatrix M(n - off, n, triplets);
                    triplets.clear();

                    Vector x(const_cast<double*>(input.data()), n);
                    Vector y(output.data() + off, n - off);

                    la.spmv(M, x, y);
                    tMultipl += timer.elapsed() - t;
                }

                ++i;
            }
        }


        if (!triplets.empty()) {
            auto off = triplets.front().row();
            for (auto& tri : triplets) {
                tri.row() -= off;
            }

            SparseMatrix M(n - off, n, triplets);
            triplets.clear();

            Vector x(const_cast<double*>(input.data()), n);
            Vector y(output.data() + off, n - off);

            la.spmv(M, x, y);
        }


        log << eckit::Plural(field, "field") << " in " << eckit::Seconds(timer.elapsed())
            << ", rate: " << double(field) / double(timer.elapsed()) << " "
            << "field/s" << std::endl;
    }

    statistics.report(log);
}

int main(int argc, char** argv) {
    MIRClimateFilter tool(argc, argv);
    return tool.start();
}
