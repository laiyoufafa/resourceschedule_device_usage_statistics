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

#ifndef BUNDLE_ACTIVE_STUB_H
#define BUNDLE_ACTIVE_STUB_H

#include <stdint.h>
#include "ibundle_active_service.h"
#include "iremote_stub.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace DeviceUsageStats {
class BundleActiveStub : public IRemoteStub<IBundleActiveService> {
public:
    BundleActiveStub() = default;
    ~BundleActiveStub() override = default;
    DISALLOW_COPY_AND_MOVE(BundleActiveStub);
    /*
    * function: OnRemoteRequest, handle message from proxy.
    * parameters: code, data, reply, option
    * return: errorcode.
    */
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel &reply,
        MessageOption &option) override;
};
}  // namespace DeviceUsageStats
}  // namespace OHOS
#endif  // BUNDLE_ACTIVE_STUB_H

