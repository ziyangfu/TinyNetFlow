/*!
 * \brief Linux Cgroups V2 资源控制管理
 * \details
 *        ubuntu 20.04 LTS 默认为 cgroup v1，需切换至 cgroup v2
 *        要求：
 *          1. 启用cgroup v2，并挂载
 *              查看是否挂载， ubuntu20.04默认将V2挂载在/sys/fs/cgroup/unified/，但没有启用：
                root@fzy-Lenovo:/sys/fs/cgroup/user.slice/netflow.group# mount | grep cgroup
                cgroup2 on /sys/fs/cgroup type cgroup2 (rw,nosuid,nodev,noexec,relatime,nsdelegate)
           2. 为subtree_controller，添加设置项
                默认的可配置项只有 memory pids，添加cpu、io等
                root@fzy-Lenovo:/sys/fs/cgroup/user.slice/netflow.group# echo "+cpuset +cpu +io" > cgroup.subtree_control
           3. 可手动创建netflow.group，也可以使用CgroupV2Controller创建， 在user.slice文件夹
 *        可参考：
 *        [Ubuntu启用Cgroups V2](https://blog.csdn.net/Kiritow/article/details/118079768)
 *        [详解Cgroup V2](https://zorrozou.github.io/docs/%E8%AF%A6%E8%A7%A3Cgroup%20V2.html)
 * */

#ifndef OSADAPTOR_PROCESS_CGROUP_V2_CONTROLLER_H
#define OSADAPTOR_PROCESS_CGROUP_V2_CONTROLLER_H

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

class CGroupV2Controller {
public:
    explicit CGroupV2Controller(const std::string& group_name)
            : cgroup_path_(get_cgroup2_mountpoint() / group_name) {
        create_cgroup();
    }

    ~CGroupV2Controller() {
        // 需要先移动所有进程回根cgroup才能删除
        // 实际生产环境应更谨慎处理
        try { fs::remove_all(cgroup_path_); }
        catch (...) {}
    }

    // 设置CPU限制（百分比）
    void set_cpu_limit(double percentage) {
        if (percentage <= 0 || percentage > 100) {
            throw std::invalid_argument("Invalid CPU percentage");
        }

        const uint64_t period = 100000;  // 100ms (默认值)
        const uint64_t quota = static_cast<uint64_t>(period * percentage / 100);

        write_value(cgroup_path_ / "cpu.max",
                    std::to_string(quota) + " " + std::to_string(period));
    }

    // 设置内存限制（MB）
    void set_memory_limit(uint64_t megabytes) {
        const uint64_t bytes = megabytes * 1024 * 1024;
        write_value(cgroup_path_ / "memory.max", std::to_string(bytes));
    }

    // 添加进程到cgroup
    void add_process(pid_t pid) const {
        write_value(cgroup_path_ / "cgroup.procs", std::to_string(pid));
    }

    // 创建子进程并自动加入cgroup
    template <typename Func>
    void run_in_cgroup(Func&& func) const {
        pid_t child_pid = fork();
        if (child_pid == 0) { // 子进程
            add_process(getpid());
            func();
            exit(0);
        }
        // 父进程返回
    }

private:
    fs::path cgroup_path_;

    static fs::path get_cgroup2_mountpoint() {
        // 检查标准挂载点
        const fs::path default_path = "/sys/fs/cgroup";
        if (fs::exists(default_path / "cgroup.controllers")) {
            return default_path;
        }
        throw std::runtime_error("cgroup v2 not mounted at /sys/fs/cgroup");
    }

    void create_cgroup() {
        try {
            fs::create_directory(cgroup_path_);

            // 启用必要的控制器
            enable_controllers({"cpu", "memory"});
        } catch (const fs::filesystem_error& e) {
            throw std::runtime_error("Failed to create cgroup: "
                                     + std::string(e.what()));
        }
    }

    void enable_controllers(const std::vector<std::string>& controllers) {
        std::ofstream file(cgroup_path_ / "cgroup.subtree_control");
        if (!file) {
            throw std::runtime_error("Cannot open subtree_control file");
        }

        for (const auto& ctrl : controllers) {
            file << "+" << ctrl << " ";
        }
        file << "\n";
    }

    static void write_value(const fs::path& file_path, const std::string& value) {
        std::ofstream file(file_path);
        if (!file) {
            throw std::runtime_error("Failed to open " + file_path.string());
        }
        file << value;
        if (!file) {
            throw std::runtime_error("Failed to write to " + file_path.string());
        }
    }
};

// 使用示例
int main() {
    try {
        CGroupV2Controller cgroup("netflow_group");

        // 设置资源限制
        cgroup.set_cpu_limit(50.0);    // 限制50% CPU
        cgroup.set_memory_limit(500); // 限制500MB内存

        // 启动受控进程
        cgroup.run_in_cgroup([]{
            std::cout << "Process running in cgroup!\n";
            // 这里执行实际工作代码
            while(true); // 示例CPU密集型任务
        });

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}


#endif //OSADAPTOR_PROCESS_CGROUP_V2_CONTROLLER_H
