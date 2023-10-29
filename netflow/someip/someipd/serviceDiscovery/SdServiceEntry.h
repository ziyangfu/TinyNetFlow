//
// Created by fzy on 23-9-12.
//

#ifndef TINYNETFLOW_SDSERVICEENTRY_H
#define TINYNETFLOW_SDSERVICEENTRY_H

#include "SdEntry.h"

namespace netflow::net::someip {
/*!
 * \brief 服务发现 service entry 实现 */
class SdServiceEntry : public SdEntry {
public:
    SdServiceEntry()
        : minorVersion_(0)
    {}
    ~SdServiceEntry() = default;

    MinorVersion getMinorVersion() const;
    void setMinorVersion(MinorVersion version);

    /** TODO: 序列化 && 反序列化 */
private:
    MinorVersion minorVersion_;   /** SD service entry 独有部分 */

};
}  // namespace netflow::net::someip



#endif //TINYNETFLOW_SDSERVICEENTRY_H
