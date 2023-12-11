//
// Created by fzy on 23-10-24.
//

#ifndef TINYNETFLOW_HANDLER_H
#define TINYNETFLOW_HANDLER_H

#include <functional>
#include <memory>
#include "TypeDefine.h"
#include "Message.h"

/** 回调函数的集合 */
namespace netflow::net::someip {

using MessageHandler = std::function< void (const std::shared_ptr<Message>& message)>;
using AvailabilityHandler = std::function<void (ServiceId, InstanceId, bool)>;
using SubscriptionHandler = std::function<bool (ClientId, bool)>;

}  // namespace netflow::net::someip

#endif //TINYNETFLOW_HANDLER_H
