#ifndef ECFIELD_H
#define ECFIELD_H

#include <vector>

// (As yet incomplete) Field Data types
// Put within eckit as the expectation is that
// will be where they reside

namespace eckit {

class Area {
public:
    Area()  : n_(0.0), w_(0.0), s_(0.0), e_(0.0) { };
    Area(double n, double w, double s, double e)  : n_(n), w_(w), s_(s), e_(e) { if (!isValid()) throw /*smthg*/; };

    double north() const { return n_; }
    double west() const { return w_; }
    double south() const { return s_; }
    double east() const { return e_; }
   
    bool operator==(const Area& rh) const { return (n_ == rh.n_ &&
                                                    w_ == rh.w_ &&
                                                    s_ == rh.s_ &&
                                                    e_ == rh.e_);}  

    //etc
private:
    double n_;
    double w_;
    double s_;
    double e_;
    
    bool isValid() const { return true; }
};

}

#endif //ECFIELD_H
