/*!
 * \brief 线程错误码
 * */

#ifndef OSADAPTOR_THREAD_THREAD_ERROR_CODE_H
#define OSADAPTOR_THREAD_THREAD_ERROR_CODE_H
#include <cstdint>
#include <string>

namespace osadaptor::thread {
/**
线程错误
    1. 线程创建失败
    2. 线程设置问题
    3. 线程无法join
    4. 线程无法detach
    5. 获取线程TID失败
    6. 线程名称错误
    7. 线程亲和性设置错误
    8. 线程调度策略设置错误
    9. 线程优先级设置错误
    10. 其他未知错误
 */
enum class ThreadErrorCode : uint32_t {
    kSuccess                            = 0,
    kThreadCreateFailed                 = 1,
    kThreadSettingError                 = 2,
    kThreadJoinFailed                   = 3,
    kThreadDetachFailed                 = 4,
    kThreadGetTidFailed                 = 5,
    kThreadSetThreadNameFailed          = 6,
    kThreadSetThreadAffinityFailed      = 7,
    kThreadSetThreadPolicyFailed        = 8,
    kThreadSetThreadPriorityFailed      = 9,
    kThreadOthers                       = 10
};

inline std::string toString(ThreadErrorCode code) {

    switch (code) {
        case ThreadErrorCode::kSuccess:
            return "kSuccess";
        case ThreadErrorCode::kThreadCreateFailed:
            return "kThreadCreateFailed";
        case ThreadErrorCode::kThreadSettingError:
            return "kThreadSettingError";
        case ThreadErrorCode::kThreadJoinFailed:
            return "kThreadJoinFailed";
        case ThreadErrorCode::kThreadDetachFailed:
            return "kThreadDetachFailed";
        case ThreadErrorCode::kThreadGetTidFailed:
            return "kThreadGetTidFailed";
        case ThreadErrorCode::kThreadSetThreadNameFailed:
            return "kThreadSetThreadNameFailed";
        case ThreadErrorCode::kThreadSetThreadAffinityFailed:
            return "kThreadSetThreadAffinityFailed";
        case ThreadErrorCode::kThreadSetThreadPolicyFailed:
            return "kThreadSetThreadPolicyFailed";
        case ThreadErrorCode::kThreadSetThreadPriorityFailed:
            return "kThreadSetThreadPriorityFailed";
        case ThreadErrorCode::kThreadOthers:
            return "kThreadOthers";
        default:
            return "unknown error";
    }
}

}  // namespace osadaptor::thread

#endif //OSADAPTOR_THREAD_THREAD_ERROR_CODE_H
