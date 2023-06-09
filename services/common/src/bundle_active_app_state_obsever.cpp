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

#include "time_service_client.h"

#include "bundle_active_app_state_observer.h"
#include "bundle_active_report_handler.h"
#include "bundle_active_event.h"
#include "bundle_active_account_helper.h"

namespace OHOS {
namespace DeviceUsageStats {
#ifndef OS_ACCOUNT_PART_ENABLED
namespace {
constexpr int32_t UID_TRANSFORM_DIVISOR = 200000;
void GetOsAccountIdFromUid(int32_t uid, int32_t &osAccountId)
{
    osAccountId = uid / UID_TRANSFORM_DIVISOR;
}
} // namespace
#endif // OS_ACCOUNT_PART_ENABLED

void BundleActiveAppStateObserver::Init(const std::shared_ptr<BundleActiveReportHandler>& reportHandler)
{
    if (reportHandler != nullptr) {
        BUNDLE_ACTIVE_LOGI("Init report handler is not null, init success");
        reportHandler_ = reportHandler;
    }
}

void BundleActiveAppStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    if (!ValidateAbilityStateData(abilityStateData)) {
        BUNDLE_ACTIVE_LOGE("validate ability state data failed!");
        return;
    }
    if (abilityStateData.abilityType != 1) {
        return;
    }
    int32_t userId = -1;
    OHOS::ErrCode ret = BundleActiveAccountHelper::GetUserId(abilityStateData.uid, userId);
    if (ret == ERR_OK && userId != -1) {
        std::stringstream stream;
        stream << abilityStateData.token.GetRefPtr();
        std::string abilityId = stream.str();
        BundleActiveReportHandlerObject tmpHandlerObject(userId, "");
        BundleActiveEvent event(abilityStateData.bundleName, abilityStateData.abilityName,
            abilityStateData.abilityName, abilityStateData.moduleName);
        tmpHandlerObject.event_ = event;
        sptr<MiscServices::TimeServiceClient> timer = MiscServices::TimeServiceClient::GetInstance();
        tmpHandlerObject.event_.timeStamp_ = timer->GetBootTimeMs();
        switch (abilityStateData.abilityState) {
            case static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND):
                tmpHandlerObject.event_.eventId_ = BundleActiveEvent::ABILITY_FOREGROUND;
                break;
            case static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND):
                tmpHandlerObject.event_.eventId_ = BundleActiveEvent::ABILITY_BACKGROUND;
                break;
            case static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_TERMINATED):
                tmpHandlerObject.event_.eventId_ = BundleActiveEvent::ABILITY_STOP;
                break;
            default:
                return;
        }
        BUNDLE_ACTIVE_LOGI("OnAbilityStateChangeduser id is %{public}d, bundle name is %{public}s, "
            "ability name is %{public}s, ability id is %{public}s, event id is %{public}d,"
            "uid is %{public}d, pid is %{public}d",
            tmpHandlerObject.userId_, tmpHandlerObject.event_.bundleName_.c_str(),
            tmpHandlerObject.event_.abilityName_.c_str(), abilityId.c_str(), tmpHandlerObject.event_.eventId_,
            abilityStateData.uid, abilityStateData.pid);
        if (reportHandler_ != nullptr) {
            BUNDLE_ACTIVE_LOGI("BundleActiveAppStateObserver::OnAbilityStateChanged handler not null, "
                "send report event msg");
            std::shared_ptr<BundleActiveReportHandlerObject> handlerobjToPtr =
                std::make_shared<BundleActiveReportHandlerObject>(tmpHandlerObject);
            auto event = AppExecFwk::InnerEvent::Get(BundleActiveReportHandler::MSG_REPORT_EVENT, handlerobjToPtr);
            reportHandler_->SendEvent(event);
        }
    }
    return;
}
}  // namespace DeviceUsageStats
}  // namespace OHOS

