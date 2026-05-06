#pragma once

#include "Thread/semaphore.h"
#include "Util/util.h"
#include <mutex>
#include <queue>

/**
 * @brief 线程安全的队列实现（C++11）
 * @tparam T 队列元素类型
 */
template <typename T>
class RQSaveQueue : public toolkit::noncopyable {
public:
    using Ptr = std::shared_ptr<RQSaveQueue<T>>;

public:
    RQSaveQueue() : m_stopped(false) {}
    ~RQSaveQueue() {
        stop();
    }

    /**
     * @brief 向队列中添加元素（支持移动语义）
     */
    void push(T msg) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_stopped) {
                return; // 队列已停止，不再接受新元素
            }
            m_queue.push(std::move(msg));
        }
        m_sem.post();
    }

    /**
     * @brief 从队列中取出元素（阻塞直到有元素或队列停止）
     * @param outMsg 输出参数，用于接收取出的元素
     * @return true 成功取出元素，false 队列已停止且为空
     */
    bool pop(T& outMsg) {
        m_sem.wait();
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_queue.empty()) {
            return false; // 队列已停止且为空
        }
        
        outMsg = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    /**
     * @brief 从队列中取出元素（带超时）
     * @param timeout_ms 超时时间（毫秒）
     * @param outMsg 输出参数，用于接收取出的元素
     * @return true 成功取出元素，false 超时或队列停止
     */
    bool pop(uint32_t timeout_ms, T& outMsg) {
        if (!m_sem.wait(timeout_ms)) {
            return false; // 超时
        }
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_queue.empty()) {
            return false;
        }
        
        outMsg = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    /**
     * @brief 尝试从队列中取出元素（非阻塞）
     * @param outMsg 输出参数，用于接收取出的元素
     * @return true 成功取出元素，false 队列为空或已停止
     */
    bool tryPop(T& outMsg) {
        if (!m_sem.wait(0)) {
            return false;
        }
    
        std::lock_guard<std::mutex> lock(m_mutex);
    
        outMsg = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
    
    /**
     * @brief 一次性取出队列中的所有元素（非阻塞）
     * @param outMsgs 输出参数，用于接收取出的所有元素（按出队顺序）
     * @return 取出的元素数量
     */
    size_t popAll(std::vector<T>& outMsgs) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_queue.empty()) {
            return 0;
        }

        size_t count = m_queue.size();

        // 预分配空间以提高性能
        outMsgs.reserve(outMsgs.size() + count);

        // 将所有元素移动到输出向量中，同时消耗信号量
        while (!m_queue.empty()) {
            outMsgs.push_back(std::move(m_queue.front()));
            m_queue.pop();
            m_sem.wait(0); // 消耗对应的信号量
        }

        return count;
    }

    /**
     * @brief 检查队列是否为空
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    /**
     * @brief 获取队列大小
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    /**
     * @brief 停止队列，唤醒所有等待的线程
     */
    void stop() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_stopped) {
                return;
            }
            m_stopped = true;
        }
        // 唤醒所有等待的线程，让它们能够退出
        m_sem.post();
    }

    /**
     * @brief 检查队列是否已停止
     */
    bool isStopped() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stopped;
    }

private:
    mutable std::mutex m_mutex;
    toolkit::semaphore m_sem;
    std::queue<T> m_queue;
    bool m_stopped;
};

