/*!
 * \brief USDT DTRACE_PROBE封装
 * \details
 *      由这些宏定义的用户态跟踪点，可以在运行时被动态追踪工具如DTrace、SystemTap、BPF等使用
 *      provider
 *              表示追踪点的逻辑分组单元，通常对应一个模块/组件
 *              类似命名空间，用于组织管理相关探针
 *              如： osadaptor_process，osadaptor_reactor， osadaptor_network
 *      probe（探针）
 *              表示具体的追踪点事件名称
 *              描述某个特定时刻的状态（如函数入口/出口）
 *          命名规范
 *              使用动词_名词形式（推荐）
 *              示例：process_create, process_exit，uds_send，shm_send
 * \file BpfTrace.h
 * */

#ifndef NETFLOW_BPF_BPF_TRACE_H
#define NETFLOW_BPF_BPF_TRACE_H

#ifdef BPF_TRACE
#include <sys/sdt.h>

#define BPF_USER_PROBE_1(provider, probe, parm1) \
    DTRACE_PROBE1(provider, probe, parm1)
#define BPF_USER_PROBE_2(provider, probe, parm1, parm2) \
    DTRACE_PROBE2(provider, probe, parm1, parm2)
#define BPF_USER_PROBE_3(provider, probe, parm1, parm2, parm3) \
    DTRACE_PROBE3(provider, probe, parm1, parm2, parm3)
#define BPF_USER_PROBE_4(provider, probe, parm1, parm2, parm3, parm4) \
    DTRACE_PROBE4(provider, probe, parm1, parm2, parm3, parm4)
#define BPF_USER_PROBE_5(provider, probe, parm1, parm2, parm3, parm4, parm5) \
    DTRACE_PROBE5(provider, probe, parm1, parm2, parm3, parm4, parm5)

#else
#define BPF_USER_PROBE_1(provider, probe, parm1)
#define BPF_USER_PROBE_2(provider, probe, parm1, parm2)
#define BPF_USER_PROBE_3(provider, probe, parm1, parm2, parm3)
#define BPF_USER_PROBE_4(provider, probe, parm1, parm2, parm3, parm4)
#define BPF_USER_PROBE_5(provider, probe, parm1, parm2, parm3, parm4, parm5)
#endif

#endif //NETFLOW_BPF_BPF_TRACE_H
