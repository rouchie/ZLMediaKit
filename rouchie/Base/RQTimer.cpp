#include "RQTimer.h"

// 每秒增加1000，达到int64_t最大值需要约292271年，基本可以认为不会重复
static std::atomic_int64_t g_timer_id { 1024 };

toolkit::EventPoller::DelayTask::Ptr
RQTimer::CreateTimer(uint64_t interval, const std::function<bool()> &callback, bool immediate, const toolkit::EventPoller::Ptr &poller)
{
    auto pollerPtr = poller;
    if (!pollerPtr) {
        pollerPtr = toolkit::EventPollerPool::Instance().getPoller();
    }

    auto task = [interval, callback]() -> uint64_t {
        bool nRet = true;
        try {
            nRet = callback();
        } catch (std::exception &ex) {
            ErrorL << "Exception occurred when do timer task: " << ex.what();
        }

        if (!nRet) {
            return 0;
        }

        return interval;
    };

    interval = immediate ? 0 : interval;
    return pollerPtr->doDelayTask(interval, task);
}

RQTimer::RQTimer(uint64_t interval, const std::function<bool()> &callback, bool immediate, const toolkit::EventPoller::Ptr &poller)
    : RQTimer()
{
    _delayTask = CreateTimer(interval, callback, immediate, poller);
}

RQTimer::~RQTimer()
{
    toolkit::EventPoller::DelayTask::Ptr ptr = _delayTask.lock();
    if (ptr) {
        ptr->cancel();
    }
    TraceL << "RQTimer Delete:" << _id;
}

RQTimer::RQTimer()
    : _id(++g_timer_id)
{
    TraceL << "RQTimer Create:" << _id;
}

int RQTimer::Start(uint64_t interval, const std::function<bool()>& callback, bool immediate, const toolkit::EventPoller::Ptr& poller)
{
    auto ptr = CreateTimer(interval, callback, immediate, poller);
    _delayTask = ptr;
    if (!ptr) {
        return false;
    }
    return true;
}

int64_t RQTimer::ID() const
{
    return _id;
}

