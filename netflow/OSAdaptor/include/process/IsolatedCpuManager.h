/*!
 * \brief 基于 Linux isolcpus 实现的 CPU 隔离管理器，
 *          预留部分CPU核心用于实时性任务
 * \details
 * 设置CPU隔离后：
 *      1. CPU核心隔离
 *      2. 中断不运行在 isolated cpu上
 * 方法：
        对于实时性要求高、高性能计算等场景，如果要让某个任务独占CPU，最理想的选择是：
            1. 采用isolcpus隔离CPU
            2. 将指定任务绑定到隔离CPU
            3. 小心意外地把中断、内核线程绑定到了隔离CPU，排查到这些“意外”分子
            4. 使能NO_HZ_FULL，则效果更佳，因为连timer tick中断也不打扰你了。
 * \file IsolatedCpuManager.h
 * */

#ifndef OSADAPTOR_PROCESS_ISOLATED_CPU_MANAGER_H
#define OSADAPTOR_PROCESS_ISOLATED_CPU_MANAGER_H

#include <vector>
#include <string>

namespace osadaptor::process {
namespace isolatedCpuManager {
/*!
 * \brief 初始化：设置哪些核心需要隔离（需要root权限）
 * */
bool isolateCpus(const std::vector<int>& cpus);

bool unsetIsolateCpus();
/*!
 * \brief 从system中，查看是否设置了CPU隔离核心
 * */
bool isSetIsolatedCpuToSystem();
/*!
 * \brief 从system中，获取当前隔离的核心列表
 * */
const std::vector<int> getIsolatedCpusFromSystem();

/**在普通变量中加入 static： 作用域仅限于当前文件 */
} // namespace isolatedCpuManager
} // namespace osadaptor::process



#endif //OSADAPTOR_PROCESS_ISOLATED_CPU_MANAGER_H
