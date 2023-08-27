//
// Created by fzy on 23-8-26.
//

#ifndef TINYNETFLOW_SOMEIPCALLBACKS_H
#define TINYNETFLOW_SOMEIPCALLBACKS_H

#include <functional>

/*!
 * \brief SOME/IP的各种回调集合 */
namespace netflow::net::someip {

using SomeIpStateCallback = std::function<void()>;
using SomeIpMessageCallback = std::function<void()>;
using SomeIpConnectedCallback = std::function<void()>;
using SomeIpSubscribeCallback = std::function<void()>;
using SomeIpPublishCallback = std::function<void()>;
using SomeIpErrorCallback = std::function<void()>;


}   // namespace netflow::net::someip



#endif //TINYNETFLOW_SOMEIPCALLBACKS_H
