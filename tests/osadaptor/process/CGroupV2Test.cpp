#include <gtest/gtest.h>
#include <sched.h>
#include <sys/wait.h>
#include <optional>
#include "process/CGroupV2Controller.h"

using namespace osadaptor::process;

// Helper function to create a child process
pid_t createChildProcess() {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        pause(); // Wait for signal
        _exit(0);
    } else if (pid > 0) {
        // Parent process
        return pid;
    } else {
        // Fork failed
        perror("fork");
        return -1;
    }
}

// Test createCgroup
TEST(CGroupV2ControllerTest, CreateCgroup) {
    CGroupV2Controller cgroup("test_group");
    EXPECT_TRUE(std::filesystem::exists(cgroup.getCurrentCgroupPath()));
}

// Test addProcess
TEST(CGroupV2ControllerTest, AddProcess) {
    CGroupV2Controller cgroup("test_group");
    pid_t pid = createChildProcess();
    ASSERT_NE(pid, -1);
    cgroup.addProcess(pid);
    std::ifstream procs_file(cgroup.getCurrentCgroupPath() / "cgroup.procs");
    std::string line;
    bool found = false;
    while (std::getline(procs_file, line)) {
        if (line == std::to_string(pid)) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
}

// Test setCPULimit
TEST(CGroupV2ControllerTest, SetCPULimit) {
    CGroupV2Controller cgroup("test_group");
    cgroup.setCPULimit(50.0);
    std::ifstream cpu_max_file(cgroup.getCurrentCgroupPath() / "cpu.max");
    std::string line;
    std::getline(cpu_max_file, line);
    EXPECT_EQ(line, "50000 100000");
}

// Test setMemoryLimit
TEST(CGroupV2ControllerTest, SetMemoryLimit) {
    CGroupV2Controller cgroup("test_group");
    cgroup.setMemoryLimit(1024); // 1024 MB
    std::ifstream memory_max_file(cgroup.getCurrentCgroupPath() / "memory.max");
    std::string line;
    std::getline(memory_max_file, line);
    EXPECT_EQ(line, "1073741824");
}

// Test removeProcess
TEST(CGroupV2ControllerTest, RemoveProcess) {
    CGroupV2Controller cgroup("test_group");
    pid_t pid = createChildProcess();
    ASSERT_NE(pid, -1);
    cgroup.addProcess(pid);
    cgroup.removeProcess(pid);
    std::ifstream procs_file(cgroup.getCurrentCgroupPath() / "cgroup.procs");
    std::string line;
    bool found = false;
    while (std::getline(procs_file, line)) {
        if (line == std::to_string(pid)) {
            found = true;
            break;
        }
    }
    EXPECT_FALSE(found);
    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
}

// Test removeCgroup
TEST(CGroupV2ControllerTest, RemoveCgroup) {
    CGroupV2Controller cgroup("test_group");
    EXPECT_TRUE(std::filesystem::exists(cgroup.getCurrentCgroupPath()));
    cgroup.~CGroupV2Controller(); // Explicitly call destructor
    EXPECT_FALSE(std::filesystem::exists(cgroup.getCurrentCgroupPath()));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

