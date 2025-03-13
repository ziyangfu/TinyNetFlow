//
// Created by fzy on 2025/3/6.
//
// test/ProcessTest.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <optional>
#include <memory>
#include "process/Process.h"

using namespace osadaptor::process;

// 模拟 Cgroup 类
class MockCgroup : public Cgroup {
public:
    MockCgroup(const std::string& subsystem, const std::string& name)
            : Cgroup(subsystem, name) {}

    MOCK_METHOD(bool, create, (), (override));
    MOCK_METHOD(bool, remove, (), (override));
    MOCK_METHOD(bool, addProcess, (pid_t pid), (override));
    MOCK_METHOD(bool, setMemoryLimit, (size_t limit), (override));
    MOCK_METHOD(bool, setCpuQuota, (long quota), (override));
};

// 模拟 Process 类
class MockProcess : public Process {
public:
    MockProcess(ProcessSettings settings)
            : Process(settings) {}

    MOCK_METHOD(void, processCreate, (const std::string &program, const std::vector<std::string> &args), (override));
    MOCK_METHOD(void, setCpuAffinity, (), (override));
    MOCK_METHOD(bool, configureScheduler, (pid_t pid, SchedulerPolicy policy, int priority), (override));
};

// 测试用例
class ProcessTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置日志
        auto console = std::make_shared<spdlog::logger>("console", std::make_shared<spdlog::sinks::basic_file_sink_mt>("test.log", true));
        spdlog::register_logger(console);
    }

    void TearDown() override {
        spdlog::drop_all();
    }
};

TEST_F(ProcessTest, TestProcessCreate) {
    ProcessSettings settings;
    settings.policy = SchedulerPolicy::OTHER;
    settings.priority = 0;

    MockProcess process(settings);

    // 模拟 Cgroup 对象
    MockCgroup cgroup("cpu", "my_cgroup");

    // 设置期望
    EXPECT_CALL(cgroup, create()).WillOnce(::testing::Return(true));
    EXPECT_CALL(cgroup, addProcess(::testing::_)).WillOnce(::testing::Return(true));
    EXPECT_CALL(process, configureScheduler(::testing::_, ::testing::_, ::testing::_)).WillOnce(::testing::Return(true));
    EXPECT_CALL(process, setCpuAffinity()).WillOnce(::testing::Return());

    // 调用 processCreate
    process.processCreate("/bin/ls", {"-l"});
}

TEST_F(ProcessTest, TestProcessCreateForkFailure) {
    ProcessSettings settings;
    settings.policy = SchedulerPolicy::OTHER;
    settings.priority = 0;

    MockProcess process(settings);

    // 模拟 fork 失败
    EXPECT_CALL(process, processCreate(::testing::_, ::testing::_)).WillOnce([this]() {
        errno = EAGAIN;
        SPDLOG_ERROR("Fork failed");
        exit(EXIT_FAILURE);
    });

    // 调用 processCreate
    EXPECT_EXIT(process.processCreate("/bin/ls", {"-l"}), ::testing::ExitedWithCode(EXIT_FAILURE), "Fork failed");
}

TEST_F(ProcessTest, TestProcessCreateCgroupFailure) {
    ProcessSettings settings;
    settings.policy = SchedulerPolicy::OTHER;
    settings.priority = 0;

    MockProcess process(settings);

    // 模拟 Cgroup 对象
    MockCgroup cgroup("cpu", "my_cgroup");

    // 设置期望
    EXPECT_CALL(cgroup, create()).WillOnce(::testing::Return(false));

    // 调用 processCreate
    EXPECT_EXIT(process.processCreate("/bin/ls", {"-l"}), ::testing::ExitedWithCode(EXIT_FAILURE), "Failed to create cgroup");
}

TEST_F(ProcessTest, TestProcessCreateAddPidFailure) {
    ProcessSettings settings;
    settings.policy = SchedulerPolicy::OTHER;
    settings.priority = 0;

    MockProcess process(settings);

    // 模拟 Cgroup 对象
    MockCgroup cgroup("cpu", "my_cgroup");

    // 设置期望
    EXPECT_CALL(cgroup, create()).WillOnce(::testing::Return(true));
    EXPECT_CALL(cgroup, addProcess(::testing::_)).WillOnce(::testing::Return(false));

    // 调用 processCreate
    EXPECT_EXIT(process.processCreate("/bin/ls", {"-l"}), ::testing::ExitedWithCode(EXIT_FAILURE), "Failed to add process to cgroup");
}

TEST_F(ProcessTest, TestProcessCreateConfigureSchedulerFailure) {
    ProcessSettings settings;
    settings.policy = SchedulerPolicy::OTHER;
    settings.priority = 0;

    MockProcess process(settings);

    // 模拟 Cgroup 对象
    MockCgroup cgroup("cpu", "my_cgroup");

    // 设置期望
    EXPECT_CALL(cgroup, create()).WillOnce(::testing::Return(true));
    EXPECT_CALL(cgroup, addProcess(::testing::_)).WillOnce(::testing::Return(true));
    EXPECT_CALL(process, configureScheduler(::testing::_, ::testing::_, ::testing::_)).WillOnce(::testing::Return(false));

    // 调用 processCreate
    EXPECT_EXIT(process.processCreate("/bin/ls", {"-l"}), ::testing::ExitedWithCode(EXIT_FAILURE), "Failed to configure scheduler");
}

TEST_F(ProcessTest, TestProcessCreateSetCpuAffinityFailure) {
    ProcessSettings settings;
    settings.policy = SchedulerPolicy::OTHER;
    settings.priority = 0;

    MockProcess process(settings);

    // 模拟 Cgroup 对象
    MockCgroup cgroup("cpu", "my_cgroup");

    // 设置期望
    EXPECT_CALL(cgroup, create()).WillOnce(::testing::Return(true));
    EXPECT_CALL(cgroup, addProcess(::testing::_)).WillOnce(::testing::Return(true));
    EXPECT_CALL(process, configureScheduler(::testing::_, ::testing::_, ::testing::_)).WillOnce(::testing::Return(true));
    EXPECT_CALL(process, setCpuAffinity()).WillOnce([this]() {
        errno = EINVAL;
        SPDLOG_ERROR("Error setting CPU affinity");
        exit(EXIT_FAILURE);
    });

    // 调用 processCreate
    EXPECT_EXIT(process.processCreate("/bin/ls", {"-l"}), ::testing::ExitedWithCode(EXIT_FAILURE), "Error setting CPU affinity");
}

TEST_F(ProcessTest, TestProcessCreateExecveFailure) {
    ProcessSettings settings;
    settings.policy = SchedulerPolicy::OTHER;
    settings.priority = 0;

    MockProcess process(settings);

    // 模拟 Cgroup 对象
    MockCgroup cgroup("cpu", "my_cgroup");

    // 设置期望
    EXPECT_CALL(cgroup, create()).WillOnce(::testing::Return(true));
    EXPECT_CALL(cgroup, addProcess(::testing::_)).WillOnce(::testing::Return(true));
    EXPECT_CALL(process, configureScheduler(::testing::_, ::testing::_, ::testing::_)).WillOnce(::testing::Return(true));
    //EXPECT_CALL(process,