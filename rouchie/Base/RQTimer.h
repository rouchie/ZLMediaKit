#pragma once

#include <functional>

#include "Poller/EventPoller.h"

class RQTimer {
public:
    /**
     * @param interval 定时器间隔，单位毫秒
     * @param callback 定时器任务，返回true表示重复下次任务，否则不重复，如果任务中抛异常，则默认重复下次任务
     * @param immediate 是否立即执行一次任务
     * @param poller EventPoller对象，可以为nullptr
     * @return 定时器任务对象指针，用于取消定时器任务，调用 cancel() 方法即可取消任务
     */
    static toolkit::EventPoller::DelayTask::Ptr
    CreateTimer(uint64_t interval, const std::function<bool()> &callback, bool immediate = false, const toolkit::EventPoller::Ptr &poller = nullptr);

public:
    using Ptr = std::shared_ptr<RQTimer>;

public:
    /**
     * 构造定时器
     * @param interval 定时器间隔，单位毫秒
     * @param callback 定时器任务，返回true表示重复下次任务，否则不重复，如果任务中抛异常，则默认重复下次任务
     * @param immediate 是否立即执行一次任务
     * @param poller EventPoller对象，可以为nullptr
     */
    RQTimer(uint64_t interval, const std::function<bool()> &callback, bool immediate = false, const toolkit::EventPoller::Ptr &poller = nullptr);
    virtual ~RQTimer();

    RQTimer();

    /**
     * 构造定时器
     * @param interval 定时器间隔，单位毫秒
     * @param callback 定时器任务，返回true表示重复下次任务，否则不重复，如果任务中抛异常，则默认重复下次任务
     * @param immediate 是否立即执行一次任务
     * @param poller EventPoller对象，可以为nullptr
     */
    int Start(uint64_t interval, const std::function<bool()> &callback, bool immediate = false, const toolkit::EventPoller::Ptr &poller = nullptr);

    /**
     * 获取定时器任务ID
     */
    int64_t ID() const;

private:
    int64_t _id = 0;
    std::weak_ptr<toolkit::EventPoller::DelayTask> _delayTask;
};
