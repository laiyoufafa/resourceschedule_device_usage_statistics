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

#include "bundle_active_proxy.h"

namespace OHOS {
namespace DeviceUsageStats {
int32_t BundleActiveProxy::ReportEvent(BundleActiveEvent& event, const int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return -1;
    }
    data.WriteInt32(userId);
    event.Marshalling(data);
    Remote() -> SendRequest(REPORT_EVENT, data, reply, option);

    int32_t result = reply.ReadInt32();
    return result;
}

bool BundleActiveProxy::IsBundleIdle(const std::string& bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    data.WriteString(bundleName);
    Remote() -> SendRequest(IS_BUNDLE_IDLE, data, reply, option);
    int32_t result = reply.ReadInt32();
    BUNDLE_ACTIVE_LOGI("result is %{public}d", result);
    return result;
}

std::vector<BundleActivePackageStats> BundleActiveProxy::QueryPackageStats(const int32_t intervalType,
    const int64_t beginTime, const int64_t endTime, int32_t& errCode, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<BundleActivePackageStats> result;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return result;
    }
    data.WriteInt32(intervalType);
    data.WriteInt64(beginTime);
    data.WriteInt64(endTime);
    data.WriteInt32(userId);
    data.WriteInt32(errCode);
    Remote() -> SendRequest(QUERY_USAGE_STATS, data, reply, option);
    errCode = reply.ReadInt32();
    int32_t size = reply.ReadInt32();
    std::shared_ptr<BundleActivePackageStats> tmp;
    for (int32_t i = 0; i < size; i++) {
        tmp = tmp->UnMarshalling(reply);
        if (tmp == nullptr) {
            continue;
        }
        result.push_back(*tmp);
    }
    for (uint32_t i = 0; i < result.size(); i++) {
        BUNDLE_ACTIVE_LOGI("QueryPackageStats result idx is %{public}d, bundleName_ is %{public}s, "
            "lastTimeUsed_ is %{public}lld, lastContiniousTaskUsed_ is %{public}lld, "
            "totalInFrontTime_ is %{public}lld, totalContiniousTaskUsedTime_ is %{public}lld",
            i + 1, result[i].bundleName_.c_str(),
            (long long)result[i].lastTimeUsed_, (long long)result[i].lastContiniousTaskUsed_,
            (long long)result[i].totalInFrontTime_, (long long)result[i].totalContiniousTaskUsedTime_);
    }
    return result;
}

std::vector<BundleActiveEvent> BundleActiveProxy::QueryEvents(const int64_t beginTime,
    const int64_t endTime, int32_t& errCode, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<BundleActiveEvent> result;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return result;
    }
    data.WriteInt64(beginTime);
    data.WriteInt64(endTime);
    data.WriteInt32(userId);
    data.WriteInt32(errCode);
    Remote() -> SendRequest(QUERY_EVENTS, data, reply, option);
    errCode = reply.ReadInt32();
    int32_t size = reply.ReadInt32();
    std::shared_ptr<BundleActiveEvent> tmp;
    for (int32_t i = 0; i < size; i++) {
        tmp = tmp->UnMarshalling(reply);
        if (tmp == nullptr) {
            continue;
        }
        result.push_back(*tmp);
    }
    for (uint32_t i = 0; i < result.size(); i++) {
        result[i].PrintEvent(true);
    }
    return result;
}

void BundleActiveProxy::SetBundleGroup(const std::string& bundleName, int32_t newGroup, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteString(bundleName);
    data.WriteInt32(newGroup);
    data.WriteInt32(userId);
    Remote() -> SendRequest(SET_BUNDLE_GROUP, data, reply, option);
}

std::vector<BundleActivePackageStats> BundleActiveProxy::QueryCurrentPackageStats(const int32_t intervalType,
    const int64_t beginTime, const int64_t endTime)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<BundleActivePackageStats> result;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return result;
    }
    data.WriteInt32(intervalType);
    data.WriteInt64(beginTime);
    data.WriteInt64(endTime);
    Remote() -> SendRequest(QUERY_CURRENT_USAGE_STATS, data, reply, option);
    int32_t size = reply.ReadInt32();
    std::shared_ptr<BundleActivePackageStats> tmp;
    for (int32_t i = 0; i < size; i++) {
        tmp = tmp->UnMarshalling(reply);
        if (tmp == nullptr) {
            continue;
        }
        result.push_back(*tmp);
    }
    for (uint32_t i = 0; i < result.size(); i++) {
        BUNDLE_ACTIVE_LOGI("QueryPackageStats result idx is %{public}d, bundleName_ is %{public}s, "
            "lastTimeUsed_ is %{public}lld, lastContiniousTaskUsed_ is %{public}lld, "
            "totalInFrontTime_ is %{public}lld, totalContiniousTaskUsedTime_ is %{public}lld",
            i + 1, result[i].bundleName_.c_str(),
            (long long)result[i].lastTimeUsed_, (long long)result[i].lastContiniousTaskUsed_,
            (long long)result[i].totalInFrontTime_, (long long)result[i].totalContiniousTaskUsedTime_);
    }
    return result;
}

std::vector<BundleActiveEvent> BundleActiveProxy::QueryCurrentEvents(const int64_t beginTime, const int64_t endTime)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<BundleActiveEvent> result;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return result;
    }
    data.WriteInt64(beginTime);
    data.WriteInt64(endTime);
    Remote() -> SendRequest(QUERY_CURRENT_EVENTS, data, reply, option);
    int32_t size = reply.ReadInt32();
    std::shared_ptr<BundleActiveEvent> tmp;
    for (int32_t i = 0; i < size; i++) {
        tmp = tmp->UnMarshalling(reply);
        if (tmp == nullptr) {
            continue;
        }
        result.push_back(*tmp);
    }
    for (uint32_t i = 0; i < result.size(); i++) {
        BUNDLE_ACTIVE_LOGI("QueryCurrentEvents event id is %{public}d, bundle name is %{public}s,"
            "time stamp is %{public}lld",
            result[i].eventId_, result[i].bundleName_.c_str(), (long long)result[i].timeStamp_);
    }
    return result;
}

int32_t BundleActiveProxy::QueryPackageGroup()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return -1;
    }
    Remote() -> SendRequest(QUERY_BUNDLE_GROUP, data, reply, option);
    int32_t packageGroup = reply.ReadInt32();
    BUNDLE_ACTIVE_LOGI("QueryPackageGroup result is %{public}d", packageGroup);
    return packageGroup;
}

int32_t BundleActiveProxy::QueryFormStatistics(int32_t maxNum, std::vector<BundleActiveModuleRecord>& results,
    int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return -1;
    }
    data.WriteInt32(maxNum);
    data.WriteInt32(userId);
    Remote() -> SendRequest(QUERY_FORM_STATS, data, reply, option);
    int32_t errCode = reply.ReadInt32();
    int32_t size = reply.ReadInt32();
    std::shared_ptr<BundleActiveModuleRecord> tmp;
    for (int32_t i = 0; i < size; i++) {
        tmp = tmp->UnMarshalling(reply);
        if (tmp == nullptr) {
            continue;
        }
        results.emplace_back(*tmp);
    }
    for (const auto& oneModule : results) {
        BUNDLE_ACTIVE_LOGI("bundle name is %{public}s, module name is %{public}s, "
            "lastusedtime is %{public}lld, launchcount is %{public}d", oneModule.bundleName_.c_str(),
            oneModule.moduleName_.c_str(), (long long)oneModule.lastModuleUsedTime_, oneModule.launchedCount_);
        for (const auto& oneForm : oneModule.formRecords_) {
            BUNDLE_ACTIVE_LOGI("form name is %{public}s, form dimension is %{public}d, form id is %{public}lld, "
                "lasttouchtime is %{public}lld, touchcount is %{public}d", oneForm.formName_.c_str(),
                oneForm.formDimension_, (long long)oneForm.formId_,
                (long long)oneForm.formLastUsedTime_, oneForm.count_);
        }
    }
    return errCode;
}
}  // namespace DeviceUsageStats
}  // namespace OHOS

