#pragma once

#include "Base/RQModuleBase.h"
#include "Base/RQHttpHelper.h"

class ApiHello : public RQModuleHelper<ApiHello> {
public:
    ApiHello(const RQHttpHelper::Ptr helper);
    ~ApiHello();

    int OnStart() override;

private:
    const RQHttpHelper::Ptr _helper;
    int64_t _id;
};