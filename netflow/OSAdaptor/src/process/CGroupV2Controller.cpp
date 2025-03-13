#include "process/CGroupV2Controller.h"
#include "spdlog/spdlog.h"

namespace osadaptor::process {
/*!
 * \brief 使用默认的路径:/sys/fs/cgroup/user.slice/netflow.group
 * */
CGroupV2Controller::CGroupV2Controller()
    : currentCgroupPath_(getCgroup2MountPoint() / userSlice_ / netflowGroupName_),
      cgroupVersion_(2)
{
    createCgroup();
}

CGroupV2Controller::CGroupV2Controller(const std::string &group_name)
    : currentCgroupPath_(getCgroup2MountPoint() / userSlice_ / group_name),
      cgroupVersion_(2)
{
    createCgroup();
}

/*!
 * \details
 *      要删除一个cgroup，必须确保它下面没有进程，并且所有子cgroup都已被删除。
 *      如果当前cgroup中还有进程，直接删除目录会失败
 * */
CGroupV2Controller::~CGroupV2Controller()  {
    try {
        // 移动所有进程到父 cgroup
        auto parent_cgroup = currentCgroupPath_.parent_path();
        auto procs_file = currentCgroupPath_ / "cgroup.procs";

        // 读取当前 cgroup 进程列表
        std::ifstream in(procs_file);
        if (in) {
            std::string pid;
            while (std::getline(in, pid)) {
                try {
                    writeValue(parent_cgroup / "cgroup.procs", pid);
                    SPDLOG_DEBUG("Moved PID {} to parent cgroup", pid);
                } catch (const std::exception& e) {
                    SPDLOG_ERROR("Failed to move PID {}: {}", pid, e.what());
                }
            }
        }
        // 递归删除 cgroup 目录
        fs::remove_all(currentCgroupPath_);
        SPDLOG_INFO("Cgroup {} removed", currentCgroupPath_.string());
    } catch (const fs::filesystem_error& e) {
        SPDLOG_ERROR("Filesystem error: {}", e.what());
    } catch (...) {
        SPDLOG_ERROR("Unknown error during cgroup destruction");
    }
}

/*!
 * \brief 添加进程至当前cgroup
 * */
void CGroupV2Controller::addProcess(pid_t pid) const {
    writeValue(currentCgroupPath_ / "cgroup.procs", std::to_string(pid));
}

void CGroupV2Controller::removeProcess(pid_t pid) const {
    try {
        // 获取父 cgroup 的路径
        auto parent_cgroup = currentCgroupPath_.parent_path();
        auto parent_procs_file = parent_cgroup / "cgroup.procs";

        // 将进程移至父 cgroup
        writeValue(parent_procs_file, std::to_string(pid));
        SPDLOG_INFO("Moved PID {} to parent cgroup", pid);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Failed to remove process PID {}: {}", pid, e.what());
    }
}

/*!
 * \brief 设置CPU限制，以百分比计
 * \details cpu.max：文件支持2个值，格式为：$MAX $PERIOD
 *                   在 100ms的时间周期里，有periodUs * percentage / 100是留给本cgroup的
 * */
void CGroupV2Controller::setCPULimit(double percentage) {
    if (percentage <= 0 || percentage > 100) {
        SPDLOG_ERROR("Invalid CPU percentage");
    }
    const uint64_t periodUs = 100000;
    const uint64_t quota = static_cast<uint64_t>(periodUs * percentage / 100);

    writeValue(currentCgroupPath_ / "cpu.max",
                std::to_string(quota) + " " + std::to_string(periodUs));
}

/*!
 * \brief 绑定CPU核心
 * */
void CGroupV2Controller::bindCPUCore() {

}
/*!
 * \brief 设置memory限制，以MB计
 * */
void CGroupV2Controller::setMemoryLimit(uint64_t MBs) {
    const uint64_t bytes = MBs * 1024 * 1024;
    writeValue(currentCgroupPath_ / "memory.max", std::to_string(bytes));
}

fs::path &CGroupV2Controller::getCurrentCgroupPath() {
    return currentCgroupPath_;
}

/*!
 * \brief 创建 CGroup, 并启用必要的控制器 "cpuset", "cpu", "io", "memory"
 * */
void CGroupV2Controller::createCgroup() {
    try {
        fs::create_directory(currentCgroupPath_);
        std::vector<std::string> controllers {"cpuset", "cpu", "io", "memory"};
        enableControllers(controllers);
    } catch (const fs::filesystem_error& e) {
        SPDLOG_ERROR("Failed to create cgroup: {}", std::string(e.what()));
    }
}

/*!
 * \brief 使能可管理资源
 * \details controller可以是：cpuset cpu io memory hugetlb pids rdma misc
 * */
void CGroupV2Controller::enableControllers(const std::vector<std::string> &controllers) {
    std::ofstream file(currentCgroupPath_ / "cgroup.subtree_control");
    if (!file) {
        throw std::runtime_error("Cannot open subtree_control file");
    }
    for (const auto& ctrl : controllers) {
        file << "+" << ctrl << " ";
    }
    file << "\n";
}

/*!
 * \brief 检查CGroup V2标准挂载点
 * */
fs::path CGroupV2Controller::getCgroup2MountPoint() {
    const fs::path defaultPath = "/sys/fs/cgroup";
    if (fs::exists(defaultPath / "cgroup.controllers")) {
        return defaultPath;
    }
    else {
        SPDLOG_ERROR("cgroup v2 not mounted at /sys/fs/cgroup");
    }
}

/*!
 * \brief 往cgroup设定项中写入值
 * */
void CGroupV2Controller::writeValue(const fs::path &filePath, const std::string &value) {
    std::ofstream file(filePath);
    if (!file) {
        SPDLOG_ERROR("Failed to open {}", filePath.string());
    }
    file << value;
    if (!file) {
        SPDLOG_ERROR("Failed to write to {}", filePath.string());
    }
}

int CGroupV2Controller::getCgroupVersion() {
    return cgroupVersion_;
}

} // namespace osadaptor::process
