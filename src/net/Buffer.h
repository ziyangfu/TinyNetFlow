//
// Created by fzy on 23-5-17.
//

#ifndef TINYNETFLOW_BUFFER_H
#define TINYNETFLOW_BUFFER_H

#include <algorithm>
#include <vector>

#include <assert.h>
#include <string.h>


namespace netflow::net {
/** 非阻塞IO必备， 包括读buffer和写buffer */
/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer {
public:
private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char kCRLF[]; /** 这个作用是什么？ */

};
} // namespace netflow::net



#endif //TINYNETFLOW_BUFFER_H
