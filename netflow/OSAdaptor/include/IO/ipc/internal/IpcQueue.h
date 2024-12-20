//
// Created by fzy on 2024/12/18.
//

/**
 * 实现一个用于IPC的FIFO队列
 * */

#ifndef OSADAPTOR_IO_IPC_INTERNAL__IPC_QUEUE_H
#define OSADAPTOR_IO_IPC_INTERNAL__IPC_QUEUE_H

/**
    struct QueueNode {
       int data;
       QueueNode* next;
   };

   struct Queue {
       QueueNode* front;
       QueueNode* rear;
   };

   Queue* queue = (Queue*)mapSharedMemory(fd, size);
   queue->front = nullptr;
   queue->rear = nullptr;
 * */

#endif //OSADAPTOR_IO_IPC_INTERNAL__IPC_QUEUE_H