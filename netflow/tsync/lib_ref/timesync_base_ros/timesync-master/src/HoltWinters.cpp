#include "HoltWinters.h"

HoltWintersSmoothFilter::HoltWintersSmoothFilter( double initialB, double alfa, double beta) {
    alfa_ = alfa;
    beta_ = beta;
    reset(initialB);
}

void HoltWintersSmoothFilter::insert(const double& v )
{
    double s_prev = s_;
    if(!gotFirst_) {
        s_ = v;
        gotFirst_ = true;
    }
    else {
        // update the estimate
        s_ = alfa_ * v + (1 - alfa_) * (s_ + b_);
        // update the estimate of the derivative
        b_ = beta_ * (s_ - s_prev) + (1 - beta_) * b_;
    }
}

// Returns the current smoothed value
double HoltWintersSmoothFilter::getFiltered() {
    return s_;
}

void HoltWintersSmoothFilter::reset(double initialB) {
    gotFirst_ = false;
    b_ = initialB;
}

void HoltWintersSmoothFilter::setAlfa(double alfa) {
    HoltWintersSmoothFilter::alfa_ = alfa;
}

void HoltWintersSmoothFilter::setBeta(double beta) {
    HoltWintersSmoothFilter::beta_ = beta;
}
