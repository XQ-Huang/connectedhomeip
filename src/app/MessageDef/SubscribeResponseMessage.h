/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once
#include "EventPaths.h"
#include "StructBuilder.h"
#include "StructParser.h"
#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace SubscribeResponseMessage {
enum
{
    kCsTag_SubscriptionId            = 0,
    kCsTag_MinIntervalFloorSeconds   = 1,
    kCsTag_MaxIntervalCeilingSeconds = 2,
};

class Parser : public StructParser
{
public:
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     */
    CHIP_ERROR CheckSchemaValidity() const;
#endif

    /**
     *  @brief Get Subscription ID. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetSubscriptionId(uint64_t * const apSubscriptionId) const;

    /**
     *  @brief Get Final MinIntervalFloorSeconds. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetMinIntervalFloorSeconds(uint16_t * const apMinIntervalFloorSeconds) const;

    /**
     *  @brief Get Final MaxIntervalCeilingSeconds. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetMaxIntervalCeilingSeconds(uint16_t * const apMaxIntervalCeilingSeconds) const;
};

class Builder : public StructBuilder
{
public:
    /**
     *  @brief final subscription Id for the subscription back to the client.s.
     */
    SubscribeResponseMessage::Builder & SubscriptionId(const uint64_t SubscriptionId);

    /**
     *  @brief Final Min Interval for the subscription back to the clients.
     */
    SubscribeResponseMessage::Builder & MinIntervalFloorSeconds(const uint16_t aMinIntervalFloorSeconds);

    /**
     *  @brief Final Max Interval for the subscription back to the clients.
     */
    SubscribeResponseMessage::Builder & MaxIntervalCeilingSeconds(const uint16_t aMaxIntervalCeilingSeconds);

    /**
     *  @brief Mark the end of this SubscribeResponseMessage
     */
    SubscribeResponseMessage::Builder & EndOfSubscribeResponseMessage();
};
} // namespace SubscribeResponseMessage
} // namespace app
} // namespace chip
