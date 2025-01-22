#include "IO/ipc/internal/RingBuffer.h"
#include <gtest/gtest.h>
#include <cstring>
#include <thread>

using namespace osadaptor::ipc::internal;

// 测试构造函数
TEST(RingBufferTest, Constructor) {
RingBuffer buffer(1024); // 1KB
EXPECT_EQ(buffer.bufferCapacity(), 1024);
EXPECT_TRUE(buffer.isEmpty());
EXPECT_FALSE(buffer.isFull());
}

// 测试写入和读取
TEST(RingBufferTest, WriteAndRead) {
RingBuffer buffer(64); // 64 bytes

const char* data = "Hello, RingBuffer!";
std::size_t dataSize = std::strlen(data) + 1;

// 写入数据
EXPECT_TRUE(buffer.write(data, dataSize));
EXPECT_FALSE(buffer.isEmpty());
EXPECT_EQ(buffer.usedCapacity(), dataSize);

// 读取数据
char readBuffer[64] = {0};
EXPECT_TRUE(buffer.read(readBuffer, dataSize));
EXPECT_EQ(std::string(readBuffer), "Hello, RingBuffer!");
EXPECT_TRUE(buffer.isEmpty());
}

// 测试缓冲区满
TEST(RingBufferTest, BufferFull) {
RingBuffer buffer(16); // 16 bytes

const char* data = "1234567890"; // 10 bytes
std::size_t dataSize = std::strlen(data) + 1;

// 写入数据
EXPECT_TRUE(buffer.write(data, dataSize));
EXPECT_FALSE(buffer.isEmpty());
EXPECT_EQ(buffer.usedCapacity(), dataSize);

// 再次写入数据（缓冲区已满）
const char* moreData = "abcdef";
EXPECT_FALSE(buffer.write(moreData, std::strlen(moreData) + 1));
}

// 测试缓冲区空
TEST(RingBufferTest, BufferEmpty) {
RingBuffer buffer(64); // 64 bytes

char readBuffer[64] = {0};
EXPECT_FALSE(buffer.read(readBuffer, 10)); // 读取空缓冲区
}

// 测试容量计算
TEST(RingBufferTest, CapacityCalculation) {
RingBuffer buffer(32); // 32 bytes

EXPECT_EQ(buffer.bufferCapacity(), 32);
EXPECT_EQ(buffer.freeCapacity(), 32);

const char* data = "12345"; // 5 bytes
std::size_t dataSize = std::strlen(data) + 1;

// 写入数据
EXPECT_TRUE(buffer.write(data, dataSize));
EXPECT_EQ(buffer.usedCapacity(), dataSize);
EXPECT_EQ(buffer.freeCapacity(), 32 - dataSize);

// 读取数据
char readBuffer[64] = {0};
EXPECT_TRUE(buffer.read(readBuffer, dataSize));
EXPECT_EQ(buffer.usedCapacity(), 0);
EXPECT_EQ(buffer.freeCapacity(), 32);
}

// 测试边界条件
TEST(RingBufferTest, BoundaryConditions) {
RingBuffer buffer(16); // 16 bytes

const char* data1 = "12345678"; // 8 bytes
const char* data2 = "abcdefgh"; // 8 bytes

// 写入数据1
EXPECT_TRUE(buffer.write(data1, std::strlen(data1) + 1));
EXPECT_EQ(buffer.usedCapacity(), 9); // 包括 null 终止符

// 写入数据2
EXPECT_TRUE(buffer.write(data2, std::strlen(data2) + 1));
EXPECT_TRUE(buffer.isFull());

// 读取数据1
char readBuffer1[16] = {0};
EXPECT_TRUE(buffer.read(readBuffer1, std::strlen(data1) + 1));
EXPECT_EQ(std::string(readBuffer1), "12345678");

// 读取数据2
char readBuffer2[16] = {0};
EXPECT_TRUE(buffer.read(readBuffer2, std::strlen(data2) + 1));
EXPECT_EQ(std::string(readBuffer2), "abcdefgh");

EXPECT_TRUE(buffer.isEmpty());
}

// 测试多线程写入和读取
TEST(RingBufferTest, MultiThreadedWriteAndRead) {
RingBuffer buffer(1024); // 1KB

auto writer = [&buffer]() {
    for (int i = 0; i < 100; ++i) {
        std::string data = "Data" + std::to_string(i);
        while (!buffer.write(data.c_str(), data.size() + 1)) {
            // 等待可用空间
        }
    }
};

auto reader = [&buffer]() {
    for (int i = 0; i < 100; ++i) {
        char readBuffer[64] = {0};
        while (!buffer.read(readBuffer, 64)) {
            // 等待数据
        }
        EXPECT_EQ(std::string(readBuffer), "Data" + std::to_string(i));
    }
};

std::thread writerThread(writer);
std::thread readerThread(reader);

writerThread.join();
readerThread.join();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}