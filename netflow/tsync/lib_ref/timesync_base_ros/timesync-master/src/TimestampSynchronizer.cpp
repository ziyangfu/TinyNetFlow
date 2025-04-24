#include <cmath>

#include "TimestampSynchronizer.h"


TimestampSynchronizer::TimestampSynchronizer(Options defaultOptions) : np_("~timestamp_synchronizer") {

    // TimestampSynchronizer gets its own private namespace inside of the node private namespace

    // advertise the debug topic, which publishes the values passed to the sync() method
    debugPublisher_ = np_.advertise<timesync::TimesyncDebug>("debug_info", 10);
    
    // load options from ROS params if available, if not, from default options passed by caller 
    options_.useMedianFilter = np_.param("useMedianFilter", defaultOptions.useMedianFilter);
    options_.medianFilterWindow = np_.param("medianFilterWindow", defaultOptions.medianFilterWindow);
    options_.useHoltWinters = np_.param("useHoltWinters", defaultOptions.useHoltWinters);
    options_.alfa_HoltWinters = np_.param("alfa_HoltWinters", defaultOptions.alfa_HoltWinters);
    options_.beta_HoltWinters = np_.param("beta_HoltWinters", defaultOptions.beta_HoltWinters);
    options_.alfa_HoltWinters_early = np_.param("alfa_HoltWinters_early", defaultOptions.alfa_HoltWinters_early);
    options_.beta_HoltWinters_early = np_.param("beta_HoltWinters_early", defaultOptions.beta_HoltWinters_early);
    options_.earlyClamp = np_.param("earlyClamp", defaultOptions.earlyClamp);
    options_.earlyClampWindow = np_.param("earlyClampWindow", defaultOptions.earlyClampWindow);
    options_.timeOffset = np_.param("timeOffset", defaultOptions.timeOffset);
    options_.initialB_HoltWinters = np_.param("initialB_HoltWinters", defaultOptions.initialB_HoltWinters);

    ROS_DEBUG("--------------------------");
    ROS_DEBUG("TimestampSynchronizer init");
    ROS_DEBUG("useMedianFilter: %d", options_.useMedianFilter);
    ROS_DEBUG("medianFilterWindow: %d", options_.medianFilterWindow);
    ROS_DEBUG("useHoltWinters: %d", options_.useHoltWinters);
    ROS_DEBUG("alfa_HoltWinters: %lf", options_.alfa_HoltWinters);
    ROS_DEBUG("beta_HoltWinters: %lf", options_.beta_HoltWinters);
    ROS_DEBUG("alfa_HoltWinters_early: %lf", options_.alfa_HoltWinters_early);
    ROS_DEBUG("beta_HoltWinters_early: %lf", options_.beta_HoltWinters_early);
    ROS_DEBUG("earlyClamp: %d", options_.earlyClamp);
    ROS_DEBUG("earlyClampWindow: %d", options_.earlyClampWindow);
    ROS_DEBUG("timeOffset: %lf", options_.timeOffset);
    ROS_DEBUG("initialB_HoltWinters: %.10lf", options_.initialB_HoltWinters);
    ROS_DEBUG("--------------------------");

    init();
}

double TimestampSynchronizer::sync(double currentSensorTime, double currentRosTime, unsigned int seqNumber_external){

    if(!firstFrameSet_) {
        seqCounter_ = 0;
        firstFrameSet_ = true;

        startSensorTime_ = currentSensorTime;
        startRosTimeBig_ = currentRosTime;
    }

    double currentRosTimeBig = currentRosTime;
    double currentSensorTimeBig = currentSensorTime;

    // subtract starting values because we need to work with small numbers,
    // or else we will lose precision
    currentRosTime = currentRosTime - startRosTimeBig_;
    currentSensorTime = currentSensorTime - startSensorTime_;

    // the basic idea of the timestamp correction algorithm: by estimating t0 (system clock - sensor clock) 
    // we eliminate the effects of both the system clock jitter and system-sensor clock drift
    double currentHypothesis_t0 = currentRosTime - currentSensorTime;

    double meanEstimatedHypothesis_t0 = currentHypothesis_t0;
    
    if(options_.useMedianFilter) {
        pmediator_->insert(meanEstimatedHypothesis_t0);
        meanEstimatedHypothesis_t0 = pmediator_->getMedian();
    }

    if(options_.useHoltWinters) {
        // at the early stage of the algorithm, use larger alfa and beta to initialize the filter faster
        if(options_.earlyClamp && seqCounter_ < options_.earlyClampWindow) {
            // smooth interpolation between early and regular alfa/beta
            double progress = ((double) seqCounter_) / options_.earlyClampWindow;
            // progress interpolated with a logistic curve, derivation 0 at finish
            double p = 1-std::exp(0.5*(1-1/(1-progress)));

            holtWinters_.setAlfa(p*options_.alfa_HoltWinters + (1-p)*options_.alfa_HoltWinters_early);
            holtWinters_.setBeta(p*options_.beta_HoltWinters + (1-p)*options_.beta_HoltWinters_early);
        }
        else {
            holtWinters_.setAlfa(options_.alfa_HoltWinters);
            holtWinters_.setBeta(options_.beta_HoltWinters);
        }
        holtWinters_.insert(meanEstimatedHypothesis_t0);
        meanEstimatedHypothesis_t0 = holtWinters_.getFiltered();
    }

    double currentOut = meanEstimatedHypothesis_t0 + currentSensorTime;

    // add back the subtracted start time, plus an user-configurable offset
    double finalOut = currentOut + options_.timeOffset + startRosTimeBig_;

    double currentError = currentOut - currentRosTime;

    timesync::TimesyncDebug debugMessage;
    debugMessage.seq = seqNumber_external;
    debugMessage.sensor_time = currentSensorTimeBig;
    debugMessage.ros_time = currentRosTimeBig;
    debugMessage.corrected_timestamp = finalOut;
    debugPublisher_.publish(debugMessage);

    ROS_DEBUG("delta_sensor: %.10lf delta_ros: %.10lf delta_out: %.10lf frame_count: %d", currentSensorTime - previousSensorTime_, currentRosTime - previousRosTime_, currentOut - previousOut_, seqNumber_external);
    ROS_DEBUG("SENSOR_TIMESTAMP: %.10lf CUR_HYP: %.10lf", currentSensorTime, currentHypothesis_t0);
    ROS_DEBUG("ROS_TIME: %.10lf STAMP: %.10lf ERR: %+.10lf RUN_HYP: %.10lf", currentRosTime, currentOut, currentError, meanEstimatedHypothesis_t0);

    // update previous values, used for calculating deltas
    previousOut_ = currentOut;
    previousSensorTime_ = currentSensorTime;
    previousRosTime_ = currentRosTime;

    seqCounter_++;

    return finalOut;

}

void TimestampSynchronizer::setTimeOffset(double timeOffset) {
    TimestampSynchronizer::options_.timeOffset = timeOffset;
}

void TimestampSynchronizer::setMedianFilter(bool useMedianFilter, int medianFilterWindow) {
    TimestampSynchronizer::options_.useMedianFilter = useMedianFilter;
    TimestampSynchronizer::options_.medianFilterWindow = medianFilterWindow;
    initMediator();
}

void TimestampSynchronizer::setUseHoltWinters(bool useHoltWinters) {
    TimestampSynchronizer::options_.useHoltWinters = useHoltWinters;
}

void TimestampSynchronizer::setAlfa_HoltWinters(double alfa_HoltWinters) {
    TimestampSynchronizer::options_.alfa_HoltWinters = alfa_HoltWinters;
}

void TimestampSynchronizer::setBeta_HoltWinters(double beta_HoltWinters) {
    TimestampSynchronizer::options_.beta_HoltWinters = beta_HoltWinters;
}

void TimestampSynchronizer::setAlfa_HoltWinters_early(double alfa_HoltWinters_early) {
    TimestampSynchronizer::options_.alfa_HoltWinters_early = alfa_HoltWinters_early;
}

void TimestampSynchronizer::setBeta_HoltWinters_early(double beta_HoltWinters_early) {
    TimestampSynchronizer::options_.beta_HoltWinters_early = beta_HoltWinters_early;
}

void TimestampSynchronizer::reset() {
    init();
}

// allocate a Mediator object
void TimestampSynchronizer::initMediator() {
    pmediator_ = std::unique_ptr<Mediator<double> >(new Mediator<double>(options_.medianFilterWindow));
}

void TimestampSynchronizer::init() {
    firstFrameSet_ = false;
    initMediator();
    holtWinters_.reset(options_.initialB_HoltWinters);
}

void TimestampSynchronizer::setEarlyClamp(bool earlyClamp, int earlyClampWindow) {
    TimestampSynchronizer::options_.earlyClamp = earlyClamp;
    TimestampSynchronizer::options_.earlyClampWindow = earlyClampWindow;
}
