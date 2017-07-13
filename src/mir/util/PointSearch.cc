/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/util/PointSearch.h"

#include <limits>
#include "mir/repres/Representation.h"
#include "eckit/config/Resource.h"
#include "eckit/log/Timer.h"
#include "eckit/log/Plural.h"
#include "mir/config/LibMir.h"

#include "eckit/container/kdtree/KDNode.h"


namespace mir {
namespace util {




class PointSearchTreeMemory: public PointSearchTree {

    typedef PointSearchTree::Point Point;
    typedef eckit::KDTreeMemory<PointSearchTree> Tree;

    Tree tree_;

    virtual void build(std::vector<PointValueType>& v) {
        tree_.build(v);
    }

    virtual void insert(const PointValueType& pt) {
        tree_.insert(pt);
    }

    virtual void statsPrint(std::ostream& out, bool pretty) {
        tree_.statsPrint(out, pretty);
    }

    virtual void statsReset() {
        tree_.statsReset();
    }

    virtual PointValueType nearestNeighbour(const PointSearchTree::Point& pt) {
        auto nn = tree_.nearestNeighbour(pt).value();
        return PointValueType(nn.point(), nn.payload());
    }

    virtual std::vector<PointValueType> kNearestNeighbours(const Point& pt, size_t k) {
        std::vector<PointValueType> result;
        for (auto n : tree_.kNearestNeighbours(pt, k)) {
            result.push_back(PointValueType(n.point(), n.payload()));
        }
        return result;
    }

    virtual std::vector<PointValueType> findInSphere(const Point& pt, double radius) {
        std::vector<PointValueType> result;
        for (auto n : tree_.findInSphere(pt, radius)) {
            result.push_back(PointValueType(n.point(), n.payload()));
        }
        return result;
    }

    virtual bool ready() const  {
        return false;
    }

    virtual void commit() {
    }

    virtual void print(std::ostream & out)  const {
        out << "KDTreeMemory[]";
    }

};


static eckit::PathName treePath(const eckit::PathName& path) {
    path.dirName().mkdir();
    if (path.exists()) {
        return path;
    }

    return path + ".tmp";
}



class PointSearchTreeMapped: public PointSearchTree {


    typedef PointSearchTree::Point Point;
    typedef eckit::KDTreeMapped<PointSearchTree> Tree;

    eckit::PathName path_;
    eckit::PathName tmp_;

    Tree tree_;

    virtual void build(std::vector<PointValueType>& v) {
        tree_.build(v);
    }

    virtual void insert(const PointValueType& pt) {
        tree_.insert(pt);
    }

    virtual void statsPrint(std::ostream& out, bool pretty) {
        tree_.statsPrint(out, pretty);
    }

    virtual void statsReset() {
        tree_.statsReset();
    }

    virtual PointValueType nearestNeighbour(const PointSearchTree::Point& pt) {
        auto nn = tree_.nearestNeighbour(pt).value();
        return PointValueType(nn.point(), nn.payload());
    }

    virtual std::vector<PointValueType> kNearestNeighbours(const Point& pt, size_t k) {
        std::vector<PointValueType> result;
        for (auto n : tree_.kNearestNeighbours(pt, k)) {
            result.push_back(PointValueType(n.point(), n.payload()));
        }
        return result;
    }

    virtual std::vector<PointValueType> findInSphere(const Point& pt, double radius) {
        std::vector<PointValueType> result;
        for (auto n : tree_.findInSphere(pt, radius)) {
            result.push_back(PointValueType(n.point(), n.payload()));
        }
        return result;
    }


    virtual bool ready() const  {
        return path_ == tmp_;
    }

    virtual void commit() {
        eckit::PathName::rename(tmp_, path_);
    }

    virtual void print(std::ostream & out) const  {
        out << "KDTreeMapped[" << path_ << "]";

    }


public:
    PointSearchTreeMapped(const eckit::PathName& path,
                          size_t itemCount,
                          size_t metadataSize):
        path_(path),
        tmp_(treePath(path)),
        tree_(tmp_, path_ == tmp_ ? 0 : itemCount, metadataSize) {

        if (ready()) {
            eckit::Log::info() << "Loading " << *this << std::endl;
        }
    }

};

PointSearch::PointSearch(const repres::Representation& r, const CompareType& isok) {
    const size_t npts = r.numberOfPoints();
    ASSERT(npts > 0);

    if (true) { // TODO: use a resource

        const long VERSION = 1;
        std::ostringstream oss;
        oss  << LibMir::cacheDir()
             << "/mir/trees/"
             << VERSION
             << "/"
             << r.uniqueName()
             << ".kdtree";


        tree_.reset(new PointSearchTreeMapped(oss.str(), npts, 0));
    }
    else {
        tree_.reset(new PointSearchTreeMemory());
    }

    if (!tree_->ready()) {
        build(r, isok);
        tree_->commit();
    }

}

void PointSearch::build(const repres::Representation & r, const CompareType & isok) {
    const size_t npts = r.numberOfPoints();

    eckit::Timer timer("Building KDTree");
    eckit::Log::info() << "Building " << *tree_ << " for " << r << " (" << eckit::Plural(npts, "point") << ")" << std::endl;

    const double infty = std::numeric_limits< double >::infinity();
    const PointType farpoint(infty, infty, infty);

    static bool fastBuildKDTrees = eckit::Resource<bool>("$ATLAS_FAST_BUILD_KDTREES", true); // We use the same Resource as ATLAS for now

    if (fastBuildKDTrees) {
        std::vector<PointValueType> points;
        points.reserve(npts);

        const eckit::ScopedPtr<repres::Iterator> it(r.iterator());
        size_t i = 0;
        while (it->next()) {
            ASSERT(i < npts);
            points.push_back(PointValueType(isok(i) ? PointType(it->point3D()) : farpoint, i));
            ++i;
        }

        tree_->build(points);
    }
    else {
        const eckit::ScopedPtr<repres::Iterator> it(r.iterator());
        size_t i = 0;
        while (it->next()) {
            ASSERT(i < npts);
            tree_->insert(PointValueType(isok(i) ? PointType(it->point3D()) : farpoint, i));
            ++i;
        }
    }
}


void PointSearch::statsPrint(std::ostream & s, bool fancy) const {
    tree_->statsPrint(s, fancy);
}


void PointSearch::statsReset() const {
    tree_->statsReset();
}


PointSearch::PointValueType PointSearch::closestPoint(const PointSearch::PointType & pt) const {
    return tree_->nearestNeighbour(pt);
}


void PointSearch::closestNPoints(const PointType & pt, size_t n, std::vector<PointValueType>& closest) const {

    // Small optimisation
    if (n == 1) {
        closest.clear();
        closest.push_back(closestPoint(pt));
        return;
    }

    closest = tree_->kNearestNeighbours(pt, n);
}


void PointSearch::closestWithinRadius(const PointType & pt, double radius, std::vector<PointValueType>& closest) const {
    closest = tree_->findInSphere(pt, radius);
}





}  // namespace util
}  // namespace mir
