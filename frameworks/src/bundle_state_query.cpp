/*
 * Copyright (c) 2022  Huawei Device Co., Ltd.
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

#include <string>
#include "securec.h"

#include "bundle_active_log.h"
#include "bundle_state_common.h"
#include "bundle_state_data.h"
#include "bundle_state_inner_errors.h"

namespace OHOS {
namespace DeviceUsageStats {
const uint32_t IS_IDLE_STATE_MIN_PARAMS = 1;
const uint32_t IS_IDLE_STATE_PARAMS = 2;
const uint32_t PRIORITY_GROUP_MIN_PARAMS = 0;
const uint32_t PRIORITY_GROUP_PARAMS = 1;
const uint32_t STATES_MIN_PARAMS = 2;
const uint32_t STATES_PARAMS = 3;
const uint32_t APP_USAGE_MIN_PARAMS_BY_INTERVAL = 3;
const uint32_t APP_USAGE_PARAMS_BY_INTERVAL = 4;
const uint32_t APP_USAGE_MIN_PARAMS = 2;
const uint32_t APP_USAGE_PARAMS = 3;
const uint32_t MODULE_RECORDS_MIN_PARAMS = 0;
const uint32_t MODULE_RECORDS_MIDDLE_PARAMS = 1;
const uint32_t MODULE_RECORDS_PARAMS = 2;
const uint32_t SECOND_ARG = 2;
const uint32_t THIRD_ARG = 3;
const int32_t MAXNUM_UP_LIMIT = 1000;

napi_value ParseModuleRecordsParameters(const napi_env &env, const napi_callback_info &info,
    ModuleRecordParamsInfo &params)
{
    size_t argc = MODULE_RECORDS_PARAMS;
    napi_value argv[MODULE_RECORDS_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc == MODULE_RECORDS_MIN_PARAMS || argc == MODULE_RECORDS_MIDDLE_PARAMS ||
        argc == MODULE_RECORDS_PARAMS, "Invalid number of parameters");

    if (argc == MODULE_RECORDS_MIN_PARAMS) {
        params.maxNum = MAXNUM_UP_LIMIT;
    }
    if (argc == MODULE_RECORDS_MIDDLE_PARAMS) {
        if (BundleStateCommon::GetInt32NumberValue(env, argv[0], params.maxNum) == nullptr) {
            BUNDLE_ACTIVE_LOGI("get module info has only one callback param");
            napi_valuetype valuetype = napi_undefined;
            NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
            NAPI_ASSERT(env, valuetype == napi_function,
                "ParseModuleRecordsParameters invalid parameter type, function expected.");
            params.maxNum = MAXNUM_UP_LIMIT;
            napi_create_reference(env, argv[0], 1, &params.callback);
        } else if (params.maxNum > MAXNUM_UP_LIMIT || params.maxNum <= 0) {
            BUNDLE_ACTIVE_LOGI("get module info has only one maxNum param");
            BUNDLE_ACTIVE_LOGE("ParseModuleRecordsParameters failed, maxNum is larger than 1000 or less/equal than 0");
            params.errorCode = ERR_MODULE_STATS_MAXNUM_INVALID;
        }
    }
    if (argc == MODULE_RECORDS_PARAMS) {
        // argv[0] : maxNum
        if (BundleStateCommon::GetInt32NumberValue(env, argv[0], params.maxNum) == nullptr) {
            BUNDLE_ACTIVE_LOGE("ParseModuleRecordsParameters failed, maxNum type is invalid.");
            params.errorCode = ERR_MODULE_STATS_MAXNUM_INVALID;
        }

        if (params.maxNum > MAXNUM_UP_LIMIT || params.maxNum <= 0) {
            BUNDLE_ACTIVE_LOGE("ParseModuleRecordsParameters failed, maxNum is larger than 1000 or less/equal than 0");
            params.errorCode = ERR_MODULE_STATS_MAXNUM_INVALID;
        }

        // argv[1] : callback
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function,
            "ParseModuleRecordsParameters invalid parameter type, function expected.");
        napi_create_reference(env, argv[1], 1, &params.callback);
    }
    return BundleStateCommon::NapiGetNull(env);
}

napi_value GetModuleUsageRecord(napi_env env, napi_callback_info info)
{
    ModuleRecordParamsInfo params;
    ParseModuleRecordsParameters(env, info, params);
    if (params.errorCode != ERR_OK) {
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    napi_value promise = nullptr;
    AsyncCallbackInfoModuleRecord *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoModuleRecord;
    if (!asyncCallbackInfo) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_NULLPTR;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    if (memset_s(asyncCallbackInfo, sizeof(AsyncCallbackInfoModuleRecord), 0,
        sizeof(AsyncCallbackInfoModuleRecord)) != EOK) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_INIT_FAILED;
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    asyncCallbackInfo->env = env;
    asyncCallbackInfo->asyncWork = nullptr;
    asyncCallbackInfo->maxNum = params.maxNum;
    BundleStateCommon::SettingCallbackPromiseInfo(env, params.callback, asyncCallbackInfo->info, promise);
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "GetModuleUsageRecord", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoModuleRecord *asyncCallbackInfo = (AsyncCallbackInfoModuleRecord *)data;
            if (asyncCallbackInfo != nullptr) {
                asyncCallbackInfo->info.errorCode =
                    asyncCallbackInfo->info.errorCode =
                        BundleActiveClient::GetInstance().QueryFormStatistics(asyncCallbackInfo->maxNum,
                        asyncCallbackInfo->moduleRecords);
            } else {
                BUNDLE_ACTIVE_LOGE("QueryBundleStateInfoByInterval, asyncCallbackInfo == nullptr");
            }
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoModuleRecord *asyncCallbackInfo = (AsyncCallbackInfoModuleRecord *)data;
            if (asyncCallbackInfo != nullptr) {
                napi_value result = nullptr;
                napi_create_array(env, &result);
                BundleStateCommon::GetModuleRecordForResult(env, asyncCallbackInfo->moduleRecords, result);
                BundleStateCommon::GetCallbackPromiseResult(env, asyncCallbackInfo->info, result);

                if (asyncCallbackInfo->info.callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->info.callback);
                }

                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    if (asyncCallbackInfo->info.isCallback) {
        return BundleStateCommon::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseIsIdleStateParameters(const napi_env &env, const napi_callback_info &info,
    IsIdleStateParamsInfo &params)
{
    size_t argc = IS_IDLE_STATE_PARAMS;
    napi_value argv[IS_IDLE_STATE_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc == IS_IDLE_STATE_MIN_PARAMS || argc == IS_IDLE_STATE_PARAMS,
        "Invalid number of parameters");

    // argv[0] : bundleName
    std::string result = "";
    params.bundleName = BundleStateCommon::GetTypeStringValue(env, argv[0], result);
    if (params.bundleName.empty()) {
        BUNDLE_ACTIVE_LOGE("ParseIsIdleStateParameters failed, bundleName is empty.");
        params.errorCode = ERR_USAGE_STATS_BUNDLENAME_EMPTY;
    }
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    if ((valuetype != napi_string) && (params.errorCode == ERR_OK)) {
        BUNDLE_ACTIVE_LOGE("Wrong argument type, string expected.");
        params.errorCode = ERR_USAGE_STATS_BUNDLENAME_TYPE;
    }

    // argv[1]: callback
    if (argc == IS_IDLE_STATE_PARAMS) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function,
            "ParseIsIdleStateParameters invalid parameter type, function expected.");
        napi_create_reference(env, argv[1], 1, &params.callback);
    }
    return BundleStateCommon::NapiGetNull(env);
}

napi_value IsIdleState(napi_env env, napi_callback_info info)
{
    IsIdleStateParamsInfo params;
    ParseIsIdleStateParameters(env, info, params);
    if (params.errorCode != ERR_OK) {
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    napi_value promise = nullptr;
    AsyncCallbackInfoIsIdleState *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoIsIdleState;
    if (!asyncCallbackInfo) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_NULLPTR;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    if (memset_s(asyncCallbackInfo, sizeof(AsyncCallbackInfoIsIdleState), 0, sizeof(AsyncCallbackInfoIsIdleState))
        != EOK) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_INIT_FAILED;
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    asyncCallbackInfo->env = env;
    asyncCallbackInfo->asyncWork = nullptr;
    asyncCallbackInfo->bundleName = params.bundleName;
    BundleStateCommon::SettingCallbackPromiseInfo(env, params.callback, asyncCallbackInfo->info, promise);
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "IsIdleState", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoIsIdleState *asyncCallbackInfo = (AsyncCallbackInfoIsIdleState *)data;
            if (asyncCallbackInfo != nullptr) {
                asyncCallbackInfo->state = BundleActiveClient::GetInstance().IsBundleIdle(
                    asyncCallbackInfo->bundleName);
            } else {
                BUNDLE_ACTIVE_LOGE("IsIdleState, asyncCallbackInfo == nullptr");
            }
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoIsIdleState *asyncCallbackInfo = (AsyncCallbackInfoIsIdleState *)data;
            if (asyncCallbackInfo != nullptr) {
                napi_value result = nullptr;
                napi_get_boolean(env, asyncCallbackInfo->state, &result);
                BundleStateCommon::GetCallbackPromiseResult(env, asyncCallbackInfo->info, result);

                if (asyncCallbackInfo->info.callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->info.callback);
                }

                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    if (asyncCallbackInfo->info.isCallback) {
        return BundleStateCommon::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParsePriorityGroupParameters(const napi_env &env, const napi_callback_info &info,
    PriorityGroupParamsInfo &params)
{
    size_t argc = PRIORITY_GROUP_PARAMS;
    napi_value argv[PRIORITY_GROUP_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc == PRIORITY_GROUP_MIN_PARAMS || argc == PRIORITY_GROUP_PARAMS,
        "Invalid number of parameters");

    // argv[0]: callback
    if (argc == PRIORITY_GROUP_PARAMS) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "ParsePriorityGroupParameters invalid parameter type. "
            "Function expected.");
        napi_create_reference(env, argv[0], 1, &params.callback);
    }
    return BundleStateCommon::NapiGetNull(env);
}

napi_value QueryAppUsagePriorityGroup(napi_env env, napi_callback_info info)
{
    PriorityGroupParamsInfo params;
    ParsePriorityGroupParameters(env, info, params);
    napi_value promise = nullptr;
    AsyncCallbackInfoPriorityGroup *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoPriorityGroup;
    if (!asyncCallbackInfo) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_NULLPTR;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    if (memset_s(asyncCallbackInfo, sizeof(AsyncCallbackInfoPriorityGroup), 0, sizeof(AsyncCallbackInfoPriorityGroup))
        != EOK) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_INIT_FAILED;
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    asyncCallbackInfo->env = env;
    asyncCallbackInfo->asyncWork = nullptr;
    BundleStateCommon::SettingCallbackPromiseInfo(env, params.callback, asyncCallbackInfo->info, promise);
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "QueryAppUsagePriorityGroup", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoPriorityGroup *asyncCallbackInfo = (AsyncCallbackInfoPriorityGroup *)data;
            if (asyncCallbackInfo != nullptr) {
                asyncCallbackInfo->priorityGroup = BundleActiveClient::GetInstance().QueryPackageGroup();
            } else {
                BUNDLE_ACTIVE_LOGE("QueryAppUsagePriorityGroup, asyncCallbackInfo == nullptr");
            }
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoPriorityGroup *asyncCallbackInfo = (AsyncCallbackInfoPriorityGroup *)data;
            if (asyncCallbackInfo != nullptr) {
                napi_value result = nullptr;
                napi_create_int32(env, asyncCallbackInfo->priorityGroup, &result);
                BundleStateCommon::GetCallbackPromiseResult(env, asyncCallbackInfo->info, result);
                if (asyncCallbackInfo->info.callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->info.callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    if (asyncCallbackInfo->info.isCallback) {
        return BundleStateCommon::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseStatesParameters(const napi_env &env, const napi_callback_info &info, StatesParamsInfo &params)
{
    size_t argc = STATES_PARAMS;
    napi_value argv[STATES_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc == STATES_MIN_PARAMS || argc == STATES_PARAMS,
        "Invalid number of parameters");

    // argv[0] : beginTime
    if (BundleStateCommon::GetInt64NumberValue(env, argv[0], params.beginTime) == nullptr) {
        BUNDLE_ACTIVE_LOGE("ParseStatesParameters failed, beginTime type is invalid.");
        params.errorCode = ERR_USAGE_STATS_BEGIN_TIME_INVALID;
    }
    if ((params.errorCode == ERR_OK)
        && (params.beginTime < TIME_NUMBER_MIN)) {
        BUNDLE_ACTIVE_LOGE("ParseStatesParameters failed, beginTime value is invalid.");
        params.errorCode = ERR_USAGE_STATS_BEGIN_TIME_INVALID;
    }

    // argv[1] : endTime
    if ((params.errorCode == ERR_OK)
        && (BundleStateCommon::GetInt64NumberValue(env, argv[1], params.endTime) == nullptr)) {
        BUNDLE_ACTIVE_LOGE("ParseStatesParameters failed, endTime type is invalid.");
        params.errorCode = ERR_USAGE_STATS_END_TIME_INVALID;
    }
    if ((params.errorCode == ERR_OK)
        && (params.endTime < TIME_NUMBER_MIN)) {
        BUNDLE_ACTIVE_LOGE("ParseStatesParameters failed, endTime value is invalid.");
        params.errorCode = ERR_USAGE_STATS_END_TIME_INVALID;
    }
    if ((params.errorCode == ERR_OK) && (params.endTime <= params.beginTime)) {
        BUNDLE_ACTIVE_LOGE("ParseStatesParameters endTime(%{public}lld) <= beginTime(%{public}lld)",
            (long long)params.endTime, (long long)params.beginTime);
        params.errorCode = ERR_USAGE_STATS_TIME_INTERVAL;
    }

    // argv[SECOND_ARG]: callback
    if (argc == STATES_PARAMS) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[SECOND_ARG], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "ParseStatesParameters invalid parameter type. "
            "Function expected.");
        napi_create_reference(env, argv[SECOND_ARG], 1, &params.callback);
    }
    return BundleStateCommon::NapiGetNull(env);
}

napi_value QueryCurrentBundleActiveStates(napi_env env, napi_callback_info info)
{
    StatesParamsInfo params;
    ParseStatesParameters(env, info, params);
    if (params.errorCode != ERR_OK) {
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    napi_value promise = nullptr;
    AsyncCallbackInfoStates *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoStates;
    if (!asyncCallbackInfo) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_NULLPTR;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    if (memset_s(asyncCallbackInfo, sizeof(AsyncCallbackInfoStates), 0, sizeof(AsyncCallbackInfoStates))
        != EOK) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_INIT_FAILED;
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    asyncCallbackInfo->env = env;
    asyncCallbackInfo->asyncWork = nullptr;
    asyncCallbackInfo->beginTime = params.beginTime;
    BUNDLE_ACTIVE_LOGI("QueryCurrentBundleActiveStates asyncCallbackInfo->beginTime: %{public}lld",
        (long long)asyncCallbackInfo->beginTime);
    asyncCallbackInfo->endTime = params.endTime;
    BUNDLE_ACTIVE_LOGI("QueryCurrentBundleActiveStates asyncCallbackInfo->endTime: %{public}lld",
        (long long)asyncCallbackInfo->endTime);
    BundleStateCommon::SettingCallbackPromiseInfo(env, params.callback, asyncCallbackInfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "QueryCurrentBundleActiveStates", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoStates *asyncCallbackInfo = (AsyncCallbackInfoStates *)data;
            if (asyncCallbackInfo != nullptr) {
                asyncCallbackInfo->BundleActiveState =
                    BundleActiveClient::GetInstance().QueryCurrentEvents(asyncCallbackInfo->beginTime,
                        asyncCallbackInfo->endTime);
            } else {
                BUNDLE_ACTIVE_LOGE("QueryCurrentBundleActiveStates, asyncCallbackInfo == nullptr");
            }
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoStates *asyncCallbackInfo = (AsyncCallbackInfoStates *)data;
            if (asyncCallbackInfo != nullptr) {
                napi_value result = nullptr;
                napi_create_array(env, &result);
                BundleStateCommon::GetBundleActiveEventForResult(env, asyncCallbackInfo->BundleActiveState, result);
                BundleStateCommon::GetCallbackPromiseResult(env, asyncCallbackInfo->info, result);

                if (asyncCallbackInfo->info.callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->info.callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

    if (asyncCallbackInfo->info.isCallback) {
        return BundleStateCommon::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value QueryBundleActiveStates(napi_env env, napi_callback_info info)
{
    StatesParamsInfo params;
    ParseStatesParameters(env, info, params);
    if (params.errorCode != ERR_OK) {
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    napi_value promise = nullptr;
    AsyncCallbackInfoStates *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoStates;
    if (!asyncCallbackInfo) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_NULLPTR;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    if (memset_s(asyncCallbackInfo, sizeof(AsyncCallbackInfoStates), 0, sizeof(AsyncCallbackInfoStates))
        != EOK) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_INIT_FAILED;
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    asyncCallbackInfo->env = env;
    asyncCallbackInfo->asyncWork = nullptr;
    asyncCallbackInfo->beginTime = params.beginTime;
    BUNDLE_ACTIVE_LOGI("QueryBundleActiveStates asyncCallbackInfo->beginTime: %{public}lld",
        (long long)asyncCallbackInfo->beginTime);
    asyncCallbackInfo->endTime = params.endTime;
    BUNDLE_ACTIVE_LOGI("QueryBundleActiveStates asyncCallbackInfo->endTime: %{public}lld",
        (long long)asyncCallbackInfo->endTime);
    BundleStateCommon::SettingCallbackPromiseInfo(env, params.callback, asyncCallbackInfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "QueryBundleActiveStates", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoStates *asyncCallbackInfo = (AsyncCallbackInfoStates *)data;
            if (asyncCallbackInfo != nullptr) {
                asyncCallbackInfo->BundleActiveState =
                    BundleActiveClient::GetInstance().QueryEvents(asyncCallbackInfo->beginTime,
                        asyncCallbackInfo->endTime, asyncCallbackInfo->info.errorCode);
            } else {
                BUNDLE_ACTIVE_LOGE("QueryBundleActiveStates, asyncCallbackInfo == nullptr");
            }
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoStates *asyncCallbackInfo = (AsyncCallbackInfoStates *)data;
            if (asyncCallbackInfo != nullptr) {
                napi_value result = nullptr;
                napi_create_array(env, &result);
                BundleStateCommon::GetBundleActiveEventForResult(env, asyncCallbackInfo->BundleActiveState, result);
                BundleStateCommon::GetCallbackPromiseResult(env, asyncCallbackInfo->info, result);
                if (asyncCallbackInfo->info.callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->info.callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    if (asyncCallbackInfo->info.isCallback) {
        return BundleStateCommon::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseAppUsageParametersByInterval(const napi_env &env, const napi_callback_info &info,
    AppUsageParamsByIntervalInfo &params)
{
    size_t argc = APP_USAGE_PARAMS_BY_INTERVAL;
    napi_value argv[APP_USAGE_PARAMS_BY_INTERVAL] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc == APP_USAGE_MIN_PARAMS_BY_INTERVAL || argc == APP_USAGE_PARAMS_BY_INTERVAL,
        "Invalid number of parameters");

    // argv[0] : intervalType
    if (BundleStateCommon::GetInt32NumberValue(env, argv[0], params.intervalType) == nullptr) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParametersByInterval failed, intervalType is invalid.");
        params.errorCode = ERR_USAGE_STATS_INTERVAL_TYPE;
    }
    if ((params.errorCode == ERR_OK) && ((params.intervalType < INTERVAL_NUMBER_MIN)
        || (params.intervalType > INTERVAL_NUMBER_MAX))) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParametersByInterval failed, intervalType number is invalid.");
        params.errorCode = ERR_USAGE_STATS_INTERVAL_NUMBER;
    }

    // argv[1] : beginTime
    if ((params.errorCode == ERR_OK)
        && (BundleStateCommon::GetInt64NumberValue(env, argv[1], params.beginTime) == nullptr)) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParametersByInterval failed, beginTime type is invalid.");
        params.errorCode = ERR_USAGE_STATS_BEGIN_TIME_INVALID;
    }
    if ((params.errorCode == ERR_OK)
        && (params.beginTime < TIME_NUMBER_MIN)) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParametersByInterval failed, beginTime value is invalid.");
        params.errorCode = ERR_USAGE_STATS_BEGIN_TIME_INVALID;
    }

    // argv[SECOND_ARG] : endTime
    if ((params.errorCode == ERR_OK)
        && (BundleStateCommon::GetInt64NumberValue(env, argv[SECOND_ARG], params.endTime) == nullptr)) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParametersByInterval failed, endTime type is invalid.");
        params.errorCode = ERR_USAGE_STATS_END_TIME_INVALID;
    }
    if ((params.errorCode == ERR_OK)
        && (params.endTime < TIME_NUMBER_MIN)) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParametersByInterval failed, endTime value is invalid.");
        params.errorCode = ERR_USAGE_STATS_END_TIME_INVALID;
    }
    if ((params.errorCode == ERR_OK) && (params.endTime <= params.beginTime)) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParametersByInterval endTime(%{public}lld) <= beginTime(%{public}lld)",
            (long long)params.endTime, (long long)params.beginTime);
        params.errorCode = ERR_USAGE_STATS_TIME_INTERVAL;
    }

    // argv[THIRD_ARG]: callback
    if (argc == APP_USAGE_PARAMS_BY_INTERVAL) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[THIRD_ARG], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "ParseAppUsageParametersByInterval invalid parameter type. "
            "Function expected.");
        napi_create_reference(env, argv[THIRD_ARG], 1, &params.callback);
    }
    return BundleStateCommon::NapiGetNull(env);
}

napi_value QueryBundleStateInfoByInterval(napi_env env, napi_callback_info info)
{
    AppUsageParamsByIntervalInfo params;
    ParseAppUsageParametersByInterval(env, info, params);
    if (params.errorCode != ERR_OK) {
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    napi_value promise = nullptr;
    AsyncCallbackInfoAppUsageByInterval *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoAppUsageByInterval;
    if (!asyncCallbackInfo) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_NULLPTR;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    if (memset_s(asyncCallbackInfo, sizeof(AsyncCallbackInfoAppUsageByInterval), 0,
        sizeof(AsyncCallbackInfoAppUsageByInterval)) != EOK) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_INIT_FAILED;
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    asyncCallbackInfo->env = env;
    asyncCallbackInfo->asyncWork = nullptr;
    asyncCallbackInfo->intervalType = params.intervalType;
    BUNDLE_ACTIVE_LOGI("QueryBundleStateInfoByInterval asyncCallbackInfo->intervalType: %{public}d",
        asyncCallbackInfo->intervalType);
    asyncCallbackInfo->beginTime = params.beginTime;
    BUNDLE_ACTIVE_LOGI("QueryBundleStateInfoByInterval asyncCallbackInfo->beginTime: %{public}lld",
        (long long)asyncCallbackInfo->beginTime);
    asyncCallbackInfo->endTime = params.endTime;
    BUNDLE_ACTIVE_LOGI("QueryBundleStateInfoByInterval asyncCallbackInfo->endTime: %{public}lld",
        (long long)asyncCallbackInfo->endTime);
    BundleStateCommon::SettingCallbackPromiseInfo(env, params.callback, asyncCallbackInfo->info, promise);
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "QueryBundleStateInfoByInterval", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoAppUsageByInterval *asyncCallbackInfo = (AsyncCallbackInfoAppUsageByInterval *)data;
            if (asyncCallbackInfo != nullptr) {
                asyncCallbackInfo->packageStats =
                    BundleActiveClient::GetInstance().QueryPackageStats(asyncCallbackInfo->intervalType,
                        asyncCallbackInfo->beginTime, asyncCallbackInfo->endTime, asyncCallbackInfo->info.errorCode);
            } else {
                BUNDLE_ACTIVE_LOGE("QueryBundleStateInfoByInterval, asyncCallbackInfo == nullptr");
            }
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoAppUsageByInterval *asyncCallbackInfo = (AsyncCallbackInfoAppUsageByInterval *)data;
            if (asyncCallbackInfo != nullptr) {
                napi_value result = nullptr;
                napi_create_array(env, &result);
                BundleStateCommon::GetBundleStateInfoByIntervalForResult(env, asyncCallbackInfo->packageStats, result);
                BundleStateCommon::GetCallbackPromiseResult(env, asyncCallbackInfo->info, result);

                if (asyncCallbackInfo->info.callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->info.callback);
                }

                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    if (asyncCallbackInfo->info.isCallback) {
        return BundleStateCommon::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseAppUsageParameters(const napi_env &env, const napi_callback_info &info, AppUsageParamsInfo &params)
{
    size_t argc = APP_USAGE_PARAMS;
    napi_value argv[APP_USAGE_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc == APP_USAGE_MIN_PARAMS || argc == APP_USAGE_PARAMS,
        "Invalid number of parameters");

    // argv[0] : beginTime
    if (BundleStateCommon::GetInt64NumberValue(env, argv[0], params.beginTime) == nullptr) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParameters failed, beginTime type is invalid.");
        params.errorCode = ERR_USAGE_STATS_BEGIN_TIME_INVALID;
    }
    if ((params.errorCode == ERR_OK) && (params.beginTime < TIME_NUMBER_MIN)) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParameters failed failed, beginTime value is invalid.");
        params.errorCode = ERR_USAGE_STATS_BEGIN_TIME_INVALID;
    }

    // argv[1] : endTime
    if ((params.errorCode == ERR_OK)
        && (BundleStateCommon::GetInt64NumberValue(env, argv[1], params.endTime) == nullptr)) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParameters failed, endTime type is invalid.");
        params.errorCode = ERR_USAGE_STATS_END_TIME_INVALID;
    }
    if ((params.errorCode == ERR_OK) && (params.endTime < TIME_NUMBER_MIN)) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParameters failed failed, endTime value is invalid.");
        params.errorCode = ERR_USAGE_STATS_END_TIME_INVALID;
    }
    if ((params.errorCode == ERR_OK) && (params.endTime <= params.beginTime)) {
        BUNDLE_ACTIVE_LOGE("ParseAppUsageParameters endTime(%{public}lld) <= beginTime(%{public}lld)",
            (long long)params.endTime, (long long)params.beginTime);
        params.errorCode = ERR_USAGE_STATS_TIME_INTERVAL;
    }

    // argv[SECOND_ARG]: callback
    if (argc == APP_USAGE_PARAMS) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[SECOND_ARG], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "ParseAppUsageParameters invalid parameter type. "
            "Function expected.");
        napi_create_reference(env, argv[SECOND_ARG], 1, &params.callback);
    }
    return BundleStateCommon::NapiGetNull(env);
}

napi_value QueryBundleStateInfos(napi_env env, napi_callback_info info)
{
    AppUsageParamsInfo params;
    ParseAppUsageParameters(env, info, params);
    if (params.errorCode != ERR_OK) {
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    napi_value promise = nullptr;
    AsyncCallbackInfoAppUsage *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoAppUsage;
    if (!asyncCallbackInfo) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_NULLPTR;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    if (memset_s(asyncCallbackInfo, sizeof(AsyncCallbackInfoAppUsage), 0, sizeof(AsyncCallbackInfoAppUsage))
        != EOK) {
        params.errorCode = ERR_USAGE_STATS_ASYNC_CALLBACK_INIT_FAILED;
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        return BundleStateCommon::JSParaError(env, params.callback, params.errorCode);
    }
    asyncCallbackInfo->env = env;
    asyncCallbackInfo->asyncWork = nullptr;
    asyncCallbackInfo->beginTime = params.beginTime;
    BUNDLE_ACTIVE_LOGI("QueryBundleStateInfos asyncCallbackInfo->beginTime: %{public}lld",
        (long long)asyncCallbackInfo->beginTime);
    asyncCallbackInfo->endTime = params.endTime;
    BUNDLE_ACTIVE_LOGI("QueryBundleStateInfos asyncCallbackInfo->endTime: %{public}lld",
        (long long)asyncCallbackInfo->endTime);
    BundleStateCommon::SettingCallbackPromiseInfo(env, params.callback, asyncCallbackInfo->info, promise);
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "QueryBundleStateInfos", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoAppUsage *asyncCallbackInfo = (AsyncCallbackInfoAppUsage *)data;
            if (asyncCallbackInfo != nullptr) {
                asyncCallbackInfo->packageStats = BundleStateCommon::GetPackageStats(asyncCallbackInfo->beginTime,
                    asyncCallbackInfo->endTime, asyncCallbackInfo->info.errorCode);
            } else {
                BUNDLE_ACTIVE_LOGE("QueryBundleStateInfos asyncCallbackInfo == nullptr");
            }
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoAppUsage *asyncCallbackInfo = (AsyncCallbackInfoAppUsage *)data;
            if (asyncCallbackInfo != nullptr) {
                napi_value result = nullptr;
                napi_create_object(env, &result);
                BundleStateCommon::GetBundleStateInfoForResult(env, asyncCallbackInfo->packageStats, result);
                BundleStateCommon::GetCallbackPromiseResult(env, asyncCallbackInfo->info, result);
                if (asyncCallbackInfo->info.callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->info.callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    if (asyncCallbackInfo->info.isCallback) {
        return BundleStateCommon::NapiGetNull(env);
    } else {
        return promise;
    }
}
}  // namespace DeviceUsageStats
}  // namespace OHOS

