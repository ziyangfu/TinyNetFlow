//
// Created by fzy on 23-10-27.
//
#include "netflow/someip/interface/Runtime.h"
#include "netflow/someip/application/SomeIpRuntime.h"

using namespace netflow::net::someip;

std::shared_ptr<Runtime> Runtime::get() {
    return SomeIpRuntime::get();
}