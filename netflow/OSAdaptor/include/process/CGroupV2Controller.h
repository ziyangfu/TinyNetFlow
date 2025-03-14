/*!
 * \brief Linux Cgroups V2 资源控制管理
 * \details
 *        ubuntu 20.04 LTS 默认为 cgroup v1，需切换至 cgroup v2
 *        ubuntu 22.04 LTS 后默认为 cgroup v2
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
 *        [Linux内核官方文档](https://docs.kernel.org/admin-guide/cgroup-v2.html)
 *        [Ubuntu启用Cgroups V2](https://blog.csdn.net/Kiritow/article/details/118079768)
 *        [详解Cgroup V2](https://zorrozou.github.io/docs/%E8%AF%A6%E8%A7%A3Cgroup%20V2.html)
 *        [cgroup--(4)cgroup v1和cgroup v2的详细介绍](https://adtxl.com/index.php/archives/179.html)
 *
 *
 * \file CGroupV2Controller.h
 * */

#ifndef OSADAPTOR_PROCESS_CGROUP_V2_CONTROLLER_H
#define OSADAPTOR_PROCESS_CGROUP_V2_CONTROLLER_H

#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace osadaptor::process {

namespace fs = std::filesystem;
class CGroupV2Controller {
public:
    CGroupV2Controller();
    explicit CGroupV2Controller(const std::string& group_name);
    ~CGroupV2Controller();
    void addProcess(pid_t pid) const;
    void removeProcess(pid_t pid) const;
    void setCPULimit(double percentage);
    void bindCPUCore();
    void setMemoryLimit(uint64_t MBs);
    int getCgroupVersion();
    fs::path& getCurrentCgroupPath();
    fs::path getRemoveCgroupPath();
private:
    void createCgroup();
    void enableControllers(const std::vector<std::string>& controllers);
    static fs::path getCgroup2MountPoint();
    static void writeValue(const fs::path& file_path, const std::string& value);
private:
    const std::string netflowGroupName_ {"netflow.group"};
    const std::string netflowGroupTempName_ {"netflow_remove_temp.group"};
    // const std::string userSlice_ {"user.slice"};  /** 在user.slice下添加总是subtree_control总是失败 */
    const std::string systemSlice_ {"system.slice"};
    fs::path currentCgroupPath_;
    int cgroupVersion_;
};
}  // namespace osadaptor::process

/**
#!/usr/bin/bash
# 1. 迁移该 cgroup 下的所有进程到父 cgroup
# 假设父 cgroup 路径为 /sys/fs/cgroup/user.slice
echo $(cat /sys/fs/cgroup/user.slice/netflow.group/cgroup.procs) | sudo tee /sys/fs/cgroup/user.slice/cgroup.procs >/dev/null

# 2. 确保无残留进程（若仍有进程，需强制终止或检查依赖）
if [ -s /sys/fs/cgroup/user.slice/netflow.group/cgroup.procs ]; then
    echo "错误：cgroup 中仍有进程存在"
    exit 1
fi

# 3. 删除叶子节点
sudo rmdir /sys/fs/cgroup/user.slice/netflow.group
 */

/**
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// 将进程从源 cgroup 迁移到父 cgroup
bool migrate_processes(const std::string& src_cgroup, const std::string& parent_cgroup) {
    std::ifstream src_procs(src_cgroup + "/cgroup.procs");
    if (!src_procs.is_open()) {
        std::cerr << "无法打开源 cgroup.procs 文件" << std::endl;
        return false;
    }

    std::ofstream parent_procs(parent_cgroup + "/cgroup.procs", std::ios::app);
    if (!parent_procs.is_open()) {
        std::cerr << "无法打开父 cgroup.procs 文件" << std::endl;
        return false;
    }

    std::string pid;
    while (std::getline(src_procs, pid)) {
        parent_procs << pid << std::endl;
        if (parent_procs.fail()) {
            std::cerr << "写入父 cgroup 失败，PID: " << pid << std::endl;
            return false;
        }
    }

    return true;
}

// 检查 cgroup 是否为空（无进程且无子节点）
bool is_cgroup_empty(const std::string& cgroup_path) {
    // 检查是否有残留进程
    std::ifstream procs(cgroup_path + "/cgroup.procs");
    if (procs.peek() != std::ifstream::traits_type::eof()) {
        std::cerr << "cgroup 中仍有进程存在" << std::endl;
        return false;
    }

    // 检查是否有子目录
    DIR* dir = opendir(cgroup_path.c_str());
    if (!dir) {
        std::cerr << "无法打开 cgroup 目录" << std::endl;
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR &&
            std::string(entry->d_name) != "." &&
            std::string(entry->d_name) != "..") {
            closedir(dir);
            std::cerr << "cgroup 存在子目录: " << entry->d_name << std::endl;
            return false;
        }
    }
    closedir(dir);

    return true;
}

// 删除 cgroup 目录
bool delete_cgroup(const std::string& cgroup_path) {
    if (rmdir(cgroup_path.c_str()) != 0) {
        perror("rmdir 失败");
        return false;
    }
    return true;
}

int main() {
    const std::string target_cgroup = "/sys/fs/cgroup/user.slice/netflow.group";
    const std::string parent_cgroup = "/sys/fs/cgroup/user.slice";

    // 步骤 1: 迁移进程
    if (!migrate_processes(target_cgroup, parent_cgroup)) {
        return 1;
    }

    // 步骤 2: 确认 cgroup 为空
    if (!is_cgroup_empty(target_cgroup)) {
        return 1;
    }

    // 步骤 3: 删除目录
    if (!delete_cgroup(target_cgroup)) {
        return 1;
    }

    std::cout << "成功删除 cgroup: " << target_cgroup << std::endl;
    return 0;
}

 */

//// 使用示例
//int main() {
//    try {
//        CGroupV2Controller cgroup("netflow_group");
//
//        // 设置资源限制
//        cgroup.set_cpu_limit(50.0);    // 限制50% CPU
//        cgroup.set_memory_limit(500); // 限制500MB内存
//
//        // 启动受控进程
//        cgroup.run_in_cgroup([]{
//            std::cout << "Process running in cgroup!\n";
//            // 这里执行实际工作代码
//            while(true); // 示例CPU密集型任务
//        });
//
//    } catch (const std::exception& e) {
//        std::cerr << "Error: " << e.what() << "\n";
//        return 1;
//    }
//    return 0;
//}


#endif //OSADAPTOR_PROCESS_CGROUP_V2_CONTROLLER_H
