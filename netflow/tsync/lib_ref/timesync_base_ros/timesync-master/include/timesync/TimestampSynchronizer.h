#ifndef TIMESTAMPSYNCHRONIZER_H
#define TIMESTAMPSYNCHRONIZER_H
/**
 * \file
 * \brief TimestampSynchronizer
 *
 * \par Advertises
 *
 * - \b ~/timestamp_synchronizer[_<suffix>]/debug_info Topic which contains the system and sensor clock
 * measurements, as they were passed to the TimeSynchronizer class
 *
 * \par Parameters
 *
 * - \b ~useMedianFilter Whether to pre-filter the time difference signal with a median filter
 * - \b ~medianFilterWindow Window of the median filter
 * - \b ~useHoltWinters Whether to use the Holt-Winters filter
 * - \b ~useMedianFilter Whether to pre-filter the time difference signal with a median filter
 * - \b ~alfa_HoltWinters Alpha coefficient of the Holt-Winters filter (fast dynamics)
 * - \b ~beta_HoltWinters Beta coefficient of the Holt-Winters filter (slow dynamics of the derivative estimate)
 * - \b ~alfa_HoltWinters_early Alpha coefficient of the Holt-Winters filter in the early clamping phase
 * - \b ~beta_HoltWinters_early Beta coefficient of the Holt-Winters filter in the early clamping phase
 * - \b ~earlyClamp Whether to use the early clamping phase
 * - \b ~earlyClampWindow Early clamping phase window
 * - \b ~timeOffset An adjustment constant which is added at the very end to the estimated timestamp
 * - \b ~initialB_HoltWinters The starting value of the derivative estimate
 */
#include <string>

#include <ros/ros.h>

#include "HoltWinters.h"
#include "Mediator.h"

#include <timesync/TimesyncDebug.h>

/*! The timestamp synchronization functionality is implemented in this class
 */
class TimestampSynchronizer
{
public:
    class Options;

    /*! The constructor of the TimestampSynchronizer class accepts default options
     *  which will be used if the corresponding ROS parameters are not defined
     */
    TimestampSynchronizer(const Options defaultOptions = Options());

    /*! Accept a reading from the sensor and computer clocks, along with the sequence number,
     *  and calculate a corrected timestamp for the sensor reading.
     *
     *  \param currentSensorTime Time of the current reading, as reported by the sensor.
     *  \param currentRosTime Time of the current reading, as read from the system clock.
     *  \param seqNumber_external Sequence number of the current reading. Used for logging purposes only
     *                            (e.g. to examine if frame drops have occured). TimestampSynchronizer keeps
     *                             its own internal sequence counter.
     *
     *  \return Returns the calculated corrected timestamp.
     */
    double sync(double currentSensorTime, double currentRosTime, unsigned int seqNumber_external);

    void setTimeOffset(double timeOffset);
    void setMedianFilter(bool useMedianFilter, int medianFilterWindow = 0);
    void setUseHoltWinters(bool useHoltWinters);
    void setAlfa_HoltWinters(double alfa_HoltWinters);
    void setBeta_HoltWinters(double beta_HoltWinters);

    // gain scheduling in the Holt-Winters filter for faster initialization
    void setAlfa_HoltWinters_early(double alfa_HoltWinters_early);
    void setBeta_HoltWinters_early(double beta_HoltWinters_early);
    void setEarlyClamp(bool earlyClamp, int earlyClampWindow = 0);

    /*! Reset the state of the TimestampSynchronizer, including the states of the Median
     *  and Holt-Winters filters.
     */
    void reset();

    /*! The Options class is used to pass default options specific
     *  for the node which uses the class, from that node, during the construction of the
     *  TimestampSynchronizer class. The ROS parameters will be used if available;
     *  if not, the default options passed to the class during the construction will be used;
     *  if none were passed, the defaults defined in the class header, TimestampSynchronizer.h,
     *  will be used.
     */
    class Options {
    public:
        Options() { };
        bool useMedianFilter = true;
        int medianFilterWindow = 3000;
        bool useHoltWinters = true;
        double alfa_HoltWinters = 1e-3;
        double beta_HoltWinters = 1e-4;
        double alfa_HoltWinters_early = 1e-1;
        double beta_HoltWinters_early = 1e-2;
        bool earlyClamp = true;
        int earlyClampWindow = 500;
        double timeOffset = 0.0;
        double initialB_HoltWinters = -3e-7;
    };


private:
    double previousSensorTime_, previousRosTime_, previousOut_;

    double startRosTimeBig_;
    double startSensorTime_;
    std::unique_ptr<Mediator<double> > pmediator_;
    HoltWintersSmoothFilter holtWinters_;

    bool firstFrameSet_;
    unsigned int seqCounter_;

    void init();
    void initMediator();
    ros::NodeHandle np_;
    ros::Publisher debugPublisher_;

    Options options_;
};

#endif
