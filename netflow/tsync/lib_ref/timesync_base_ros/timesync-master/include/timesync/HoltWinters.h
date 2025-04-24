#ifndef HOLTWINTERS_H
#define HOLTWINTERS_H
#include <ros/ros.h>

/*! This class implements a recursive exponential smoothing filter which
 *  also estimates the derivative of the signal in order to produce
 *  an estimate without lag in steady state, as described in
 *  https://en.wikipedia.org/wiki/Exponential_smoothing#Double_exponential_smoothing
 */
class HoltWintersSmoothFilter
{
public:
    HoltWintersSmoothFilter( double initialB = defaultInitialB_, double alfa = defaultAlfa_, double beta = defaultBeta_);
    /*! Inserts the current reading into the filter
     */
    void insert(const double& v );

    /*! Returns the current smoothed value
     */
    double getFiltered();

    void reset(double initialB = defaultInitialB_);
    void setAlfa(double alfa);
    void setBeta(double beta);

private:

    static constexpr double defaultAlfa_ = 1e-3;
    static constexpr double defaultBeta_ = 1e-4;
    static constexpr double defaultInitialB_ = -3e-7;

    bool gotFirst_;
    double b_, s_;
    double alfa_, beta_;
};

#endif
