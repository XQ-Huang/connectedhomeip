/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "SubscribeRequestMessage.h"
#include "MessageDefHelper.h"

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR SubscribeRequestMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int TagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("SubscribeRequestMessage =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kKeepSubscriptions):
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kKeepSubscriptions))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kKeepSubscriptions));
            VerifyOrReturnError(TLV::kTLVType_Boolean == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool keepSubscriptions;
                ReturnErrorOnFailure(reader.Get(keepSubscriptions));
                PRETTY_PRINT("\tKeepSubscriptions = %s, ", keepSubscriptions ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kMinIntervalFloorSeconds):
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kMinIntervalFloorSeconds))),
                                CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kMinIntervalFloorSeconds));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t MinIntervalFloorSeconds;
                ReturnErrorOnFailure(reader.Get(MinIntervalFloorSeconds));
                PRETTY_PRINT("\tMinIntervalFloorSeconds = 0x%" PRIx16 ",", MinIntervalFloorSeconds);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kMaxIntervalCeilingSeconds):
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kMaxIntervalCeilingSeconds))),
                                CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kMaxIntervalCeilingSeconds));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t MaxIntervalCeilingSeconds;
                ReturnErrorOnFailure(reader.Get(MaxIntervalCeilingSeconds));
                PRETTY_PRINT("\tMaxIntervalCeilingSeconds = 0x%" PRIx16 ",", MaxIntervalCeilingSeconds);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kAttributeRequests):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kAttributeRequests))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kAttributeRequests));
            {
                AttributePathIBs::Parser attributeRequests;
                ReturnErrorOnFailure(attributeRequests.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(attributeRequests.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kEventRequests):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kEventRequests))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kEventRequests));
            {
                EventPaths::Parser eventRequests;
                ReturnErrorOnFailure(eventRequests.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(eventRequests.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kEventFilters):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kEventFilters))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kEventFilters));
            {
                EventFilters::Parser eventFilters;
                ReturnErrorOnFailure(eventFilters.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(eventFilters.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kIsProxy):
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kIsProxy))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kIsProxy));
            VerifyOrReturnError(TLV::kTLVType_Boolean == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool isProxy;
                ReturnErrorOnFailure(reader.Get(isProxy));
                PRETTY_PRINT("\tIsProxy = %s, ", isProxy ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kIsFabricFiltered):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kIsFabricFiltered))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kIsFabricFiltered));
#if CHIP_DETAIL_LOGGING
            {
                bool isFabricFiltered;
                ReturnErrorOnFailure(reader.Get(isFabricFiltered));
                PRETTY_PRINT("\tisFabricFiltered = %s, ", isFabricFiltered ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT("");

    if (CHIP_END_OF_TLV == err)
    {
        const int RequiredFields = (1 << to_underlying(Tag::kIsFabricFiltered)) |
            (1 << to_underlying(Tag::kMinIntervalFloorSeconds)) | (1 << to_underlying(Tag::kMaxIntervalCeilingSeconds));

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_SUBSCRIBE_REQUEST_MESSAGE;
        }
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR SubscribeRequestMessage::Parser::GetKeepSubscriptions(bool * const apKeepExistingSubscription) const
{
    return GetSimpleValue(to_underlying(Tag::kKeepSubscriptions), TLV::kTLVType_Boolean, apKeepExistingSubscription);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetMinIntervalFloorSeconds(uint16_t * const apMinIntervalFloorSeconds) const
{
    return GetUnsignedInteger(to_underlying(Tag::kMinIntervalFloorSeconds), apMinIntervalFloorSeconds);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetMaxIntervalCeilingSeconds(uint16_t * const apMaxIntervalCeilingSeconds) const
{
    return GetUnsignedInteger(to_underlying(Tag::kMaxIntervalCeilingSeconds), apMaxIntervalCeilingSeconds);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetAttributeRequests(AttributePathIBs::Parser * const apAttributeRequests) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kAttributeRequests)), reader));
    return apAttributeRequests->Init(reader);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetEventRequests(EventPaths::Parser * const apEventRequests) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kEventRequests)), reader));
    return apEventRequests->Init(reader);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetEventFilters(EventFilters::Parser * const apEventFilters) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kEventFilters)), reader));
    return apEventFilters->Init(reader);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetIsProxy(bool * const apIsProxy) const
{
    return GetSimpleValue(to_underlying(Tag::kIsProxy), TLV::kTLVType_Boolean, apIsProxy);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetIsFabricFiltered(bool * const apIsFabricFiltered) const
{
    return GetSimpleValue(to_underlying(Tag::kIsFabricFiltered), TLV::kTLVType_Boolean, apIsFabricFiltered);
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::KeepSubscriptions(const bool aKeepSubscriptions)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kKeepSubscriptions)), aKeepSubscriptions);
    }
    return *this;
}

SubscribeRequestMessage::Builder &
SubscribeRequestMessage::Builder::MinIntervalFloorSeconds(const uint16_t aMinIntervalFloorSeconds)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kMinIntervalFloorSeconds)), aMinIntervalFloorSeconds);
    }
    return *this;
}

SubscribeRequestMessage::Builder &
SubscribeRequestMessage::Builder::MaxIntervalCeilingSeconds(const uint16_t aMaxIntervalCeilingSeconds)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kMaxIntervalCeilingSeconds)), aMaxIntervalCeilingSeconds);
    }
    return *this;
}

AttributePathIBs::Builder & SubscribeRequestMessage::Builder::CreateAttributeRequests()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeRequests.Init(mpWriter, to_underlying(Tag::kAttributeRequests));
    }
    return mAttributeRequests;
}

EventPaths::Builder & SubscribeRequestMessage::Builder::CreateEventRequests()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventRequests.Init(mpWriter, to_underlying(Tag::kEventRequests));
    }
    return mEventRequests;
}

EventFilters::Builder & SubscribeRequestMessage::Builder::CreateEventFilters()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventFilters.Init(mpWriter, to_underlying(Tag::kEventFilters));
    }
    return mEventFilters;
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::IsProxy(const bool aIsProxy)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kIsProxy)), aIsProxy);
    }
    return *this;
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::IsFabricFiltered(const bool aIsFabricFiltered)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kIsFabricFiltered)), aIsFabricFiltered);
    }
    return *this;
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::EndOfSubscribeRequestMessage()
{
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip
