/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BUNDLE_ACTIVE_PROXY_H
#define BUNDLE_ACTIVE_PROXY_H

#include "ibundle_active_service.h"
#include "bundle_active_event.h"
#include "bundle_active_event_stats.h"
#include "bundle_active_package_stats.h"
#include "bundle_active_module_record.h"
#include "ibundle_active_group_callback.h"

namespace OHOS {
namespace DeviceUsageStats {
class BundleActiveProxy : public IRemoteProxy<IBundleActiveService> {
public:
    /*
    * function: ReportEvent, used to report event.
    * parameters: event, userId
    * return: errorcode.
    */
    int32_t ReportEvent(BundleActiveEvent& event, const int32_t userId) override;

    /*
    * function: IsBundleIdle, used to check whether specific bundle is idle.
    * parameters: bundleName
    * return: if bundle is idle, return true. if bundle is not idle, return false.
    */
    bool IsBundleIdle(const std::string& bundleName, int32_t& errCode, int32_t userId = -1) override;

    /*
    * function: QueryBundleStatsInfoByInterval, query all bundle usage statistics in specific time span for calling user.
    * parameters: intervalType, beginTime, endTime, errCode
    * return: vector of bundle usage statistics.
    */
    std::vector<BundleActivePackageStats> QueryBundleStatsInfoByInterval(const int32_t intervalType, const int64_t beginTime,
        const int64_t endTime, int32_t& errCode, int32_t userId = -1) override;

    /*
    * function: QueryBundleEvents, query all events in specific time span for calling user.
    * parameters: beginTime, endTime, errCode
    * return: vector of events.
    */
    std::vector<BundleActiveEvent> QueryBundleEvents(const int64_t beginTime, const int64_t endTime,
        int32_t& errCode, int32_t userId = -1) override;

    /*
    * function: SetAppGroup, set specific bundle of specific user to a priority group.
    * parameters: bundleName, newGroup, userId
    */
    int32_t SetAppGroup(const std::string& bundleName, int32_t newGroup, int32_t errCode, int32_t userId) override;

    /*
    * function: QueryBundleStatsInfos, query bundle usage statistics in specific time span for calling bundle.
    * parameters: intervalType, beginTime, endTime
    * return: vector of calling bundle usage statistics.
    */
    std::vector<BundleActivePackageStats> QueryBundleStatsInfos(const int32_t intervalType, const int64_t beginTime,
        const int64_t endTime) override;

    /*
    * function: QueryCurrentBundleEvents, query bundle usage statistics in specific time span for calling bundle.
    * parameters: beginTime, endTime
    * return: vector of calling bundle events.
    */
    std::vector<BundleActiveEvent> QueryCurrentBundleEvents(const int64_t beginTime, const int64_t endTime) override;

    /*
    * function: QueryAppGroup, query bundle priority group calling bundle.
    * return: the priority group of calling bundle.
    */
    int32_t QueryAppGroup(std::string& bundleName, const int32_t userId) override;

    /*
    * function: QueryModuleUsageRecords, query all from usage statistics in specific time span for calling user.
    * parameters: maxNum, results, userId, default userId is -1 for JS API,
    * if other SAs call this API, they should explicit define userId.
    * return: errorcode.
    */
    int32_t QueryModuleUsageRecords(int32_t maxNum, std::vector<BundleActiveModuleRecord>& results,
        int32_t userId = -1) override;

    /*
    * function: QueryDeviceEventStates, query all from event stats in specific time span for calling user.
    * parameters: beginTime, endTime, eventStats, userId, default userId is -1 for JS API,
    * if other SAs call this API, they should explicit define userId.
    * return: errorcode.
    */
    int32_t QueryDeviceEventStates(int64_t beginTime, int64_t endTime,
        std::vector<BundleActiveEventStats>& eventStats, int32_t userId) override;

    /*
    * function: QueryNotificationNumber, query all app notification number in specific time span for calling user.
    * parameters: beginTime, endTime, eventStats, userId, default userId is -1 for JS API,
    * if other SAs call this API, they should explicit define userId.
    * return: errorcode.
    */
    int32_t QueryNotificationNumber(int64_t beginTime, int64_t endTime,
        std::vector<BundleActiveEventStats>& eventStats, int32_t userId) override;

    /*
    * function: BundleActiveProxy, default constructor.
    * parameters: impl
    */
    explicit BundleActiveProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IBundleActiveService>(impl) {}

    /*
    * function: RegisterAppGroupCallBack, register the observer to groupObservers.
    * parameters: observer
    * return: result of RegisterAppGroupCallBack, true or false.
    */
    int32_t RegisterAppGroupCallBack(const sptr<IBundleActiveGroupCallback> &observer) override;

    /*
    * function: UnRegisterAppGroupCallBack, remove the observer from groupObservers.
    * parameters: observer
    * return: result of UnRegisterAppGroupCallBack, true or false.
    */
    int32_t UnRegisterAppGroupCallBack(const sptr<IBundleActiveGroupCallback> &observer) override;

    /*
    * function: ~BundleActiveProxy, default destructor.
    */
    virtual ~BundleActiveProxy() {}

private:
    static inline BrokerDelegator<BundleActiveProxy> delegator_;
    int32_t IPCCommunication(int64_t beginTime, int64_t endTime, std::vector<BundleActiveEventStats>& eventStats,
        int32_t userId, int32_t communicationFlag);
};
}  // namespace DeviceUsageStats
}  // namespace OHOS
#endif  // BUNDLE_ACTIVE_PROXY_H

