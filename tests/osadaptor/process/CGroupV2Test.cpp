#include <gtest/gtest.h>
#include <sched.h>
#include <sys/wait.h>
#include <optional>

#include "process/CGroupV2Controller.h"
//#include "/home/fzy/Downloads/03_net_lib/TinyNetFlow/netflow/OSAdaptor/include/process/CGroupV2Controller.h"

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
    std::ifstream controllers_file_read(cgroup.getCurrentCgroupPath() / "cgroup.controllers");
    std::string line;
    std::getline(controllers_file_read, line);
    SCOPED_TRACE("Current line: " + line);
    EXPECT_EQ(line, "cpuset cpu io memory pids");
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
    pid_t pid = createChildProcess();
    ASSERT_NE(pid, -1);
    bool found = false;
    {
        CGroupV2Controller cgroup;
        cgroup.addProcess(pid);
        cgroup.removeProcess(pid);
        std::ifstream procs_file(cgroup.getCurrentCgroupPath() / "cgroup.procs");
        std::string line;
        while (std::getline(procs_file, line)) {
            if (line == std::to_string(pid)) {
                found = true;
                break;
            }
        }
    //sleep(1000);
    }  /** 此时应该进程移除，cgroup删除， 若进程没有移除成功，cgroup将还会存在 */
    EXPECT_FALSE(found);
    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

