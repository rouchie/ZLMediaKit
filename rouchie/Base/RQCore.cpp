#include "RQCore.h"
#include "RQDefines.h"

RQCore::Ptr RQCore::INST()
{
    static RQCore::Ptr inst = std::make_shared<RQCore>();
    return inst;
}

int RQCore::SendMsg(const RQMsg::Ptr& msg)
{
    if (!msg) return CODE_EMPTY;

    auto range = _mapModules.equal_range(msg->_recver);
    for (auto it = range.first; it != range.second;) {
        if (auto locked = it->second.lock()) {
            locked->GetPoller()->async([locked, msg]() {
                int nRet = locked->OnModuleCallback(msg);
            });
            ++it;
        } else {
            it = _mapModules.erase(it);
        }
    }

    return CODE_SUCCESS;
}

int64_t RQCore::SendDelayMsg(int64_t delay, const RQMsg::Ptr& msg)
{
    RQTimer::Ptr timer = std::make_shared<RQTimer>();
    int64_t id = timer->ID();

    timer->Start(delay, [id, msg, weak_this = WPtr(shared_from_this())]() {
        if (auto shared_this = weak_this.lock()) {
            shared_this->SendMsg(msg);
            shared_this->DelayMsgCancel(id);
        }
        return false;
    });

    _mapTimers.emplace(id, timer);

    return id;
}

void RQCore::DelayMsgCancel(int64_t timerID)
{
    _mapTimers.erase(timerID);
}

void RQCore::AddModule(mod_t id, RQModuleBase::Ptr module)
{
    if (!module)
        return;

    std::lock_guard<std::mutex> lock(_mutex);

    auto range = _mapModules.equal_range(id);
    for (auto it = range.first; it != range.second; ++it) {
        if (auto locked = it->second.lock()) {
            if (locked == module) {
                return;
            }
        }
    }

    _mapModules.emplace(id, module);

    // 回调 module 的 OnStart 函数
    module->GetPoller()->async(
        [module]() {
            module->OnStart();
        }
    );
}

void RQCore::DelModule(mod_t id)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _mapModules.erase(id);
}

void RQCore::DelModule(RQModuleBase::Ptr module)
{
    if (!module) {
        return;
    }

    std::lock_guard<std::mutex> lock(_mutex);
    mod_t id = module->ID();

    auto range = _mapModules.equal_range(id);
    for (auto it = range.first; it != range.second;) {
        if (auto locked = it->second.lock()) {
            if (locked == module) {
                _mapModules.erase(it);
                return;
            }
        }
        ++it;
    }
}