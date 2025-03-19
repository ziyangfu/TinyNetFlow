/*!
 * \brief OSAadptor-Process单元测试
 * */
#include <gtest/gtest.h>
#include <sched.h>
#include <sys/wait.h>
#include <optional>
#include <string>
#include <vector>
#include "process/Process.h"
using namespace osadaptor::process;
/** Test Process 实例化 */
TEST(ProcessTest, process_instance) {
    ProcessSettings settings {.schePolicy_=SchedulerPolicy::RR,
                              .schePriority_=0
    };
    std::string cmd = "/usr/bin/ls";
    std::vector<std::string> args {"-a", "-l", "-h"};
    Process process {cmd, args, settings};
}
/** Test process create, processSettings只有调度策略与调度优先级 */
TEST(ProcessTest, process_create) {
    ProcessSettings settings {.schePolicy_=SchedulerPolicy::OTHER,
            .schePriority_=10
    };
    std::string cmd = "/usr/bin/ls";
    std::vector<std::string> args {"-a", "-l", "-h"};
    Process process {cmd, args, settings};
    process.processCreate();
    auto pid =  process.getCurrentProcessPid();
    SCOPED_TRACE("pid is: " + std::to_string(pid));
    EXPECT_NE(pid, -1);
}

/** Test process_create_with_RR, processSettings只有调度策略与调度优先级 */
TEST(ProcessTest, process_create_with_RR) {
    ProcessSettings settings {.schePolicy_=SchedulerPolicy::RR,
            .schePriority_=10
    };
    std::string cmd = "/usr/bin/ls";
    std::vector<std::string> args {"-a", "-l", "-h"};
    Process process {cmd, args, settings};
    process.processCreate();
    auto pid =  process.getCurrentProcessPid();
    SCOPED_TRACE("pid is: " + std::to_string(pid));
    EXPECT_NE(pid, -1);
}

/** Test process create, processSettings另有cpuAffinity、uid、gid、cgroup等可选属性 */
TEST(ProcessTest, process_create_with_settings) {
//    ProcessSettings settings {
//        .schePolicy_=SchedulerPolicy::RR,
//        .schePriority_=10,
//        .cpuAffinity_= 0,
//        .cGroupSettings_= {.isSettingCGroup = true,
//                           .memoryLimit_= 512,
//                           .resourceGroupPath_="/sys/fs/cgroup/memory/test_group",
//                           },
//        .userGroupId_ = 0,
//        .userId_ = 0,
//        .currentWorkDir_ = "/tmp"
//    };
//    std::string cmd = "/usr/bin/ls";
//    std::vector<std::string> args {"-a", "-l", "-h"};
//    Process process {cmd, args, settings};
//    process.processCreate();
//    auto pid =  process.getCurrentProcessPid();
//    SCOPED_TRACE("pid is: " + std::to_string(pid));
//    EXPECT_NE(pid, -1);

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}