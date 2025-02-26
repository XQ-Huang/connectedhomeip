/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *      This file implements a test for  CHIP Interaction Model Message Def
 *
 */

#include <app/AppBuildConfig.h>
#include <app/MessageDef/EventFilters.h>
#include <app/MessageDef/EventStatusIB.h>
#include <app/MessageDef/InvokeRequestMessage.h>
#include <app/MessageDef/InvokeResponseMessage.h>
#include <app/MessageDef/ReadRequestMessage.h>
#include <app/MessageDef/ReportDataMessage.h>
#include <app/MessageDef/SubscribeRequestMessage.h>
#include <app/MessageDef/SubscribeResponseMessage.h>
#include <app/MessageDef/TimedRequestMessage.h>
#include <app/MessageDef/WriteRequestMessage.h>
#include <app/MessageDef/WriteResponseMessage.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <nlunit-test.h>

namespace {

using namespace chip::app;

void TLVPrettyPrinter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vprintf(aFormat, args);

    va_end(args);
}

CHIP_ERROR DebugPrettyPrint(const chip::System::PacketBufferHandle & aMsgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVReader reader;
    reader.Init(aMsgBuf.Retain());
    err = reader.Next();
    chip::TLV::Debug::Dump(reader, TLVPrettyPrinter);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogProgress(DataManagement, "DebugPrettyPrint fails with err %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

void BuildStatusIB(nlTestSuite * apSuite, StatusIB::Builder & aStatusIBBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    StatusIB statusIB;
    statusIB.mStatus = chip::Protocols::InteractionModel::Status::InvalidSubscription;
    aStatusIBBuilder.EncodeStatusIB(statusIB);
    err = aStatusIBBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseStatusIB(nlTestSuite * apSuite, StatusIB::Parser & aStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusIB::Parser StatusIBParser;
    StatusIB statusIB;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aStatusIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aStatusIBParser.DecodeStatusIB(statusIB);
    NL_TEST_ASSERT(apSuite,
                   err == CHIP_NO_ERROR && statusIB.mStatus == chip::Protocols::InteractionModel::Status::InvalidSubscription &&
                       !statusIB.mClusterStatus.HasValue());
}

void BuildEventFilterIB(nlTestSuite * apSuite, EventFilterIB::Builder & aEventFilterIBBuilder)
{
    aEventFilterIBBuilder.Node(1).EventMin(2).EndOfEventFilterIB();
    NL_TEST_ASSERT(apSuite, aEventFilterIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventFilterIB(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventFilterIB::Parser eventFilterIBParser;
    chip::NodeId node = 1;
    uint64_t eventMin = 2;

    err = eventFilterIBParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = eventFilterIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = eventFilterIBParser.GetNode(&node);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && node == 1);

    err = eventFilterIBParser.GetEventMin(&eventMin);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && eventMin == 2);
}

void BuildEventFilters(nlTestSuite * apSuite, EventFilters::Builder & aEventFiltersBuilder)
{
    EventFilterIB::Builder eventFilterBuilder = aEventFiltersBuilder.CreateEventFilter();
    NL_TEST_ASSERT(apSuite, aEventFiltersBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventFilterIB(apSuite, eventFilterBuilder);
    aEventFiltersBuilder.EndOfEventFilters();
    NL_TEST_ASSERT(apSuite, aEventFiltersBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventFilters(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventFilters::Parser eventFiltersParser;

    err = eventFiltersParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = eventFiltersParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildAttributePathIB(nlTestSuite * apSuite, AttributePathIB::Builder & aAttributePathBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    aAttributePathBuilder.EnableTagCompression(false)
        .Node(1)
        .Endpoint(2)
        .Cluster(3)
        .Attribute(4)
        .ListIndex(5)
        .EndOfAttributePathIB();
    err = aAttributePathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseAttributePathIB(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    AttributePathIB::Parser attributePathParser;
    CHIP_ERROR err              = CHIP_NO_ERROR;
    bool enableTagCompression   = true;
    chip::NodeId node           = 1;
    chip::EndpointId endpoint   = 2;
    chip::ClusterId cluster     = 3;
    chip::AttributeId attribute = 4;
    chip::ListIndex listIndex   = 5;

    err = attributePathParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = attributePathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif

    err = attributePathParser.GetEnableTagCompression(&enableTagCompression);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && enableTagCompression == false);

    err = attributePathParser.GetNode(&node);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && node == 1);

    err = attributePathParser.GetEndpoint(&endpoint);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpoint == 2);

    err = attributePathParser.GetCluster(&cluster);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && cluster == 3);

    err = attributePathParser.GetAttribute(&attribute);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && attribute == 4);

    err = attributePathParser.GetListIndex(&listIndex);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && listIndex == 5);
}

void BuildAttributePathList(nlTestSuite * apSuite, AttributePathIBs::Builder & aAttributePathListBuilder)
{
    AttributePathIB::Builder attributePathBuilder = aAttributePathListBuilder.CreateAttributePath();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePathIB(apSuite, attributePathBuilder);

    aAttributePathListBuilder.EndOfAttributePathIBs();
    NL_TEST_ASSERT(apSuite, aAttributePathListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributePathList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathIBs::Parser attributePathListParser;
    AttributePathIB::Parser attributePathParser;

    err = attributePathListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = attributePathListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildEventPath(nlTestSuite * apSuite, EventPathIB::Builder & aEventPathBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    aEventPathBuilder.Node(1).Endpoint(2).Cluster(3).Event(4).IsUrgent(true).EndOfEventPathIB();
    err = aEventPathBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseEventPath(nlTestSuite * apSuite, EventPathIB::Parser & aEventPathParser)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    chip::NodeId node         = 1;
    chip::EndpointId endpoint = 2;
    chip::ClusterId cluster   = 3;
    chip::EventId event       = 4;
    bool isUrgent             = false;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aEventPathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aEventPathParser.GetNode(&node);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && node == 1);

    err = aEventPathParser.GetEndpoint(&endpoint);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpoint == 2);

    err = aEventPathParser.GetCluster(&cluster);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && cluster == 3);

    err = aEventPathParser.GetEvent(&event);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && event == 4);

    err = aEventPathParser.GetIsUrgent(&isUrgent);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && isUrgent == true);
}

void BuildEventPaths(nlTestSuite * apSuite, EventPaths::Builder & aEventPathsBuilder)
{
    EventPathIB::Builder eventPathBuilder = aEventPathsBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, eventPathBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPath(apSuite, eventPathBuilder);

    aEventPathsBuilder.EndOfEventPaths();
    NL_TEST_ASSERT(apSuite, aEventPathsBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventPaths(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPaths::Parser eventPathListParser;

    err = eventPathListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = eventPathListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildCommandPath(nlTestSuite * apSuite, CommandPathIB::Builder & aCommandPathBuilder)
{
    aCommandPathBuilder.EndpointId(1).ClusterId(3).CommandId(4).EndOfCommandPathIB();
    NL_TEST_ASSERT(apSuite, aCommandPathBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseCommandPath(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPathParser;
    chip::EndpointId endpointId = 0;
    chip::ClusterId clusterId   = 0;
    chip::CommandId commandId   = 0;

    err = commandPathParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = commandPathParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = commandPathParser.GetEndpointId(&endpointId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && endpointId == 1);

    err = commandPathParser.GetClusterId(&clusterId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && clusterId == 3);

    err = commandPathParser.GetCommandId(&commandId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && commandId == 4);
}

void BuildEventDataIB(nlTestSuite * apSuite, EventDataIB::Builder & aEventDataIBBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    EventPathIB::Builder eventPathBuilder = aEventDataIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, eventPathBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPath(apSuite, eventPathBuilder);

    aEventDataIBBuilder.Priority(2).EventNumber(3).EpochTimestamp(4).SystemTimestamp(5).DeltaEpochTimestamp(6).DeltaSystemTimestamp(
        7);
    err = aEventDataIBBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    // Construct test event data
    {
        chip::TLV::TLVWriter * pWriter = aEventDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(EventDataIB::Tag::kData)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    aEventDataIBBuilder.EndOfEventDataIB();
}

void ParseEventDataIB(nlTestSuite * apSuite, EventDataIB::Parser & aEventDataIBParser)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    uint8_t priorityLevel         = 0;
    chip::EventNumber number      = 0;
    uint64_t EpochTimestamp       = 0;
    uint64_t systemTimestamp      = 0;
    uint64_t deltaUTCTimestamp    = 0;
    uint64_t deltaSystemTimestamp = 0;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aEventDataIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    {
        {
            EventPathIB::Parser eventPath;
            err = aEventDataIBParser.GetPath(&eventPath);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        }
        err = aEventDataIBParser.GetPriority(&priorityLevel);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && priorityLevel == 2);
        err = aEventDataIBParser.GetEventNumber(&number);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && number == 3);
        err = aEventDataIBParser.GetEpochTimestamp(&EpochTimestamp);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && EpochTimestamp == 4);
        err = aEventDataIBParser.GetSystemTimestamp(&systemTimestamp);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && systemTimestamp == 5);
        err = aEventDataIBParser.GetDeltaEpochTimestamp(&deltaUTCTimestamp);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && deltaUTCTimestamp == 6);
        err = aEventDataIBParser.GetDeltaSystemTimestamp(&deltaSystemTimestamp);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && deltaSystemTimestamp == 7);

        {
            chip::TLV::TLVReader reader;
            bool val = false;
            chip::TLV::TLVType container;
            aEventDataIBParser.GetData(&reader);
            err = reader.EnterContainer(container);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

            err = reader.Next();
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

            err = reader.Get(val);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && val);

            err = reader.ExitContainer(container);
            NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        }
    }
}

void BuildEventStatusIB(nlTestSuite * apSuite, EventStatusIB::Builder & aEventStatusIBBuilder)
{
    EventPathIB::Builder eventPathBuilder = aEventStatusIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, aEventStatusIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPath(apSuite, eventPathBuilder);

    StatusIB::Builder statusIBBuilder = aEventStatusIBBuilder.CreateErrorStatus();
    NL_TEST_ASSERT(apSuite, statusIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildStatusIB(apSuite, statusIBBuilder);

    aEventStatusIBBuilder.EndOfEventStatusIB();
    NL_TEST_ASSERT(apSuite, aEventStatusIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventStatusIB(nlTestSuite * apSuite, EventStatusIB::Parser & aEventStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPathIB::Parser eventPathParser;
    StatusIB::Parser statusParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aEventStatusIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aEventStatusIBParser.GetPath(&eventPathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aEventStatusIBParser.GetErrorStatus(&statusParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildEventReportIB(nlTestSuite * apSuite, EventReportIB::Builder & aEventReportIBBuilder)
{
    EventDataIB::Builder eventDataIBBuilder = aEventReportIBBuilder.CreateEventData();
    NL_TEST_ASSERT(apSuite, aEventReportIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventDataIB(apSuite, eventDataIBBuilder);

    aEventReportIBBuilder.EndOfEventReportIB();
    NL_TEST_ASSERT(apSuite, aEventReportIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventReportIB(nlTestSuite * apSuite, EventReportIB::Parser & aEventReportIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventStatusIB::Parser eventStatusParser;
    EventDataIB::Parser eventDataParser;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aEventReportIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif

    err = aEventReportIBParser.GetEventData(&eventDataParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildEventReports(nlTestSuite * apSuite, EventReports::Builder & aEventReportsBuilder)
{
    EventReportIB::Builder eventReportIBBuilder = aEventReportsBuilder.CreateEventReport();
    NL_TEST_ASSERT(apSuite, aEventReportsBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventReportIB(apSuite, eventReportIBBuilder);

    aEventReportsBuilder.EndOfEventReports();
    NL_TEST_ASSERT(apSuite, aEventReportsBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseEventReports(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventReports::Parser eventReportsParser;

    err = eventReportsParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = eventReportsParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildAttributeStatusIB(nlTestSuite * apSuite, AttributeStatusIB::Builder & aAttributeStatusIBBuilder)
{
    AttributePathIB::Builder attributePathBuilder = aAttributeStatusIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, attributePathBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePathIB(apSuite, attributePathBuilder);

    StatusIB::Builder statusIBBuilder = aAttributeStatusIBBuilder.CreateErrorStatus();
    NL_TEST_ASSERT(apSuite, statusIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildStatusIB(apSuite, statusIBBuilder);

    aAttributeStatusIBBuilder.EndOfAttributeStatusIB();
    NL_TEST_ASSERT(apSuite, aAttributeStatusIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeStatusIB(nlTestSuite * apSuite, AttributeStatusIB::Parser & aAttributeStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathIB::Parser attributePathParser;
    StatusIB::Parser StatusIBParser;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aAttributeStatusIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aAttributeStatusIBParser.GetPath(&attributePathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aAttributeStatusIBParser.GetErrorStatus(&StatusIBParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildAttributeStatuses(nlTestSuite * apSuite, AttributeStatuses::Builder & aAttributeStatusesBuilder)
{
    AttributeStatusIB::Builder aAttributeStatusIBBuilder = aAttributeStatusesBuilder.CreateAttributeStatus();
    NL_TEST_ASSERT(apSuite, aAttributeStatusesBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeStatusIB(apSuite, aAttributeStatusIBBuilder);

    aAttributeStatusesBuilder.EndOfAttributeStatuses();
    NL_TEST_ASSERT(apSuite, aAttributeStatusesBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeStatuses(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeStatuses::Parser attributeStatusParser;

    err = attributeStatusParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = attributeStatusParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildAttributeDataIB(nlTestSuite * apSuite, AttributeDataIB::Builder & aAttributeDataIBBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    AttributePathIB::Builder attributePathBuilder = aAttributeDataIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, aAttributeDataIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePathIB(apSuite, attributePathBuilder);

    // Construct attribute data
    {
        chip::TLV::TLVWriter * pWriter = aAttributeDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(AttributeDataIB::Tag::kData)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }
    aAttributeDataIBBuilder.DataVersion(2);
    err = aAttributeDataIBBuilder.GetError();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    aAttributeDataIBBuilder.EndOfAttributeDataIB();
    NL_TEST_ASSERT(apSuite, aAttributeDataIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeDataIB(nlTestSuite * apSuite, AttributeDataIB::Parser & aAttributeDataIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathIB::Parser attributePathParser;
    chip::DataVersion version = 0;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aAttributeDataIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aAttributeDataIBParser.GetPath(&attributePathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aAttributeDataIBParser.GetDataVersion(&version);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && version == 2);

    {
        chip::TLV::TLVReader reader;
        bool val = false;
        chip::TLV::TLVType container;
        aAttributeDataIBParser.GetData(&reader);
        err = reader.EnterContainer(container);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = reader.Next();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = reader.Get(val);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && val);

        err = reader.ExitContainer(container);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }
}

void BuildAttributeDataIBs(nlTestSuite * apSuite, AttributeDataIBs::Builder & aAttributeDataIBsBuilder)
{
    AttributeDataIB::Builder AttributeDataIBBuilder = aAttributeDataIBsBuilder.CreateAttributeDataIBBuilder();
    NL_TEST_ASSERT(apSuite, aAttributeDataIBsBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataIB(apSuite, AttributeDataIBBuilder);

    aAttributeDataIBsBuilder.EndOfAttributeDataIBs();
    NL_TEST_ASSERT(apSuite, aAttributeDataIBsBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeDataIBs(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeDataIBs::Parser AttributeDataIBsParser;

    err = AttributeDataIBsParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = AttributeDataIBsParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildAttributeReportIB(nlTestSuite * apSuite, AttributeReportIB::Builder & aAttributeReportIBBuilder)
{
    AttributeDataIB::Builder attributeDataIBBuilder = aAttributeReportIBBuilder.CreateAttributeData();
    NL_TEST_ASSERT(apSuite, aAttributeReportIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataIB(apSuite, attributeDataIBBuilder);

    aAttributeReportIBBuilder.EndOfAttributeReportIB();
    NL_TEST_ASSERT(apSuite, aAttributeReportIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeReportIB(nlTestSuite * apSuite, AttributeReportIB::Parser & aAttributeReportIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeStatusIB::Parser attributeStatusParser;
    AttributeDataIB::Parser attributeDataParser;

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aAttributeReportIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aAttributeReportIBParser.GetAttributeData(&attributeDataParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildAttributeReportIBs(nlTestSuite * apSuite, AttributeReportIBs::Builder & aAttributeReportIBsBuilder)
{
    AttributeReportIB::Builder attributeReportIBBuilder = aAttributeReportIBsBuilder.CreateAttributeReport();
    NL_TEST_ASSERT(apSuite, aAttributeReportIBsBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeReportIB(apSuite, attributeReportIBBuilder);

    aAttributeReportIBsBuilder.EndOfAttributeReportIBs();
    NL_TEST_ASSERT(apSuite, aAttributeReportIBsBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeReportIBs(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeReportIBs::Parser attributeReportIBsParser;

    err = attributeReportIBsParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = attributeReportIBsParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildAttributeDataVersionList(nlTestSuite * apSuite, AttributeDataVersionList::Builder & aAttributeDataVersionListBuilder)
{
    aAttributeDataVersionListBuilder.AddVersion(1);

    aAttributeDataVersionListBuilder.EndOfAttributeDataVersionList();
    NL_TEST_ASSERT(apSuite, aAttributeDataVersionListBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseAttributeDataVersionList(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DataVersion version;
    AttributeDataVersionList::Parser attributeDataVersionListParser;

    err = attributeDataVersionListParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = attributeDataVersionListParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    attributeDataVersionListParser.GetVersion(&version);
}

void BuildCommandDataIB(nlTestSuite * apSuite, CommandDataIB::Builder & aCommandDataIBBuilder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    CommandPathIB::Builder commandPathBuilder = aCommandDataIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, aCommandDataIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandPath(apSuite, commandPathBuilder);

    // Construct command data
    {
        chip::TLV::TLVWriter * pWriter = aCommandDataIBBuilder.GetWriter();
        chip::TLV::TLVType dummyType   = chip::TLV::kTLVType_NotSpecified;
        err = pWriter->StartContainer(chip::TLV::ContextTag(chip::to_underlying(CommandDataIB::Tag::kData)),
                                      chip::TLV::kTLVType_Structure, dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->PutBoolean(chip::TLV::ContextTag(1), true);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = pWriter->EndContainer(dummyType);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }

    aCommandDataIBBuilder.EndOfCommandDataIB();
    NL_TEST_ASSERT(apSuite, aCommandDataIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseCommandDataIB(nlTestSuite * apSuite, CommandDataIB::Parser & aCommandDataIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPathParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aCommandDataIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aCommandDataIBParser.GetPath(&commandPathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        chip::TLV::TLVReader reader;
        bool val = false;
        chip::TLV::TLVType container;
        aCommandDataIBParser.GetData(&reader);
        err = reader.EnterContainer(container);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = reader.Next();
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        err = reader.Get(val);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && val);

        err = reader.ExitContainer(container);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    }
}

void BuildCommandStatusIB(nlTestSuite * apSuite, CommandStatusIB::Builder & aCommandStatusIBBuilder)
{
    CommandPathIB::Builder commandPathBuilder = aCommandStatusIBBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, aCommandStatusIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandPath(apSuite, commandPathBuilder);

    StatusIB::Builder statusIBBuilder = aCommandStatusIBBuilder.CreateErrorStatus();
    NL_TEST_ASSERT(apSuite, statusIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildStatusIB(apSuite, statusIBBuilder);

    aCommandStatusIBBuilder.EndOfCommandStatusIB();
    NL_TEST_ASSERT(apSuite, aCommandStatusIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseCommandStatusIB(nlTestSuite * apSuite, CommandStatusIB::Parser & aCommandStatusIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPathParser;
    StatusIB::Parser statusParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aCommandStatusIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aCommandStatusIBParser.GetPath(&commandPathParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = aCommandStatusIBParser.GetErrorStatus(&statusParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildWrongInvokeResponseIB(nlTestSuite * apSuite, InvokeResponseIB::Builder & aInvokeResponseIBBuilder)
{
    CommandDataIB::Builder commandDataBuilder = aInvokeResponseIBBuilder.CreateCommand();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandDataIB(apSuite, commandDataBuilder);
    CommandStatusIB::Builder commandStatusBuilder = aInvokeResponseIBBuilder.CreateStatus();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandStatusIB(apSuite, commandStatusBuilder);
    aInvokeResponseIBBuilder.EndOfInvokeResponseIB();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
}

void BuildInvokeResponseIBWithCommandDataIB(nlTestSuite * apSuite, InvokeResponseIB::Builder & aInvokeResponseIBBuilder)
{
    CommandDataIB::Builder commandDataBuilder = aInvokeResponseIBBuilder.CreateCommand();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandDataIB(apSuite, commandDataBuilder);
    aInvokeResponseIBBuilder.EndOfInvokeResponseIB();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
}

void BuildInvokeResponseIBWithCommandStatusIB(nlTestSuite * apSuite, InvokeResponseIB::Builder & aInvokeResponseIBBuilder)
{
    CommandStatusIB::Builder commandStatusBuilder = aInvokeResponseIBBuilder.CreateStatus();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandStatusIB(apSuite, commandStatusBuilder);
    aInvokeResponseIBBuilder.EndOfInvokeResponseIB();
    NL_TEST_ASSERT(apSuite, aInvokeResponseIBBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseInvokeResponseIBWithCommandDataIB(nlTestSuite * apSuite, InvokeResponseIB::Parser & aInvokeResponseIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataIB::Parser commandDataParser;
    CommandStatusIB::Parser statusIBParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aInvokeResponseIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aInvokeResponseIBParser.GetCommand(&commandDataParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseInvokeResponseIBWithCommandStatusIB(nlTestSuite * apSuite, InvokeResponseIB::Parser & aInvokeResponseIBParser)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataIB::Parser commandDataParser;
    CommandStatusIB::Parser statusIBParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = aInvokeResponseIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = aInvokeResponseIBParser.GetStatus(&statusIBParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void ParseWrongInvokeResponseIB(nlTestSuite * apSuite, InvokeResponseIB::Parser & aInvokeResponseIBParser)
{
    CommandDataIB::Parser commandDataParser;
    CommandStatusIB::Parser statusIBParser;
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    CHIP_ERROR err = aInvokeResponseIBParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err != CHIP_NO_ERROR);
#endif
}

void BuildInvokeRequests(nlTestSuite * apSuite, InvokeRequests::Builder & aInvokeRequestsBuilder)
{
    CommandDataIB::Builder aCommandDataIBBuilder = aInvokeRequestsBuilder.CreateCommandData();
    NL_TEST_ASSERT(apSuite, aInvokeRequestsBuilder.GetError() == CHIP_NO_ERROR);
    BuildCommandDataIB(apSuite, aCommandDataIBBuilder);

    aInvokeRequestsBuilder.EndOfInvokeRequests();
    NL_TEST_ASSERT(apSuite, aInvokeRequestsBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseInvokeRequests(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeRequests::Parser invokeRequestsParser;
    err = invokeRequestsParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeRequestsParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildInvokeResponses(nlTestSuite * apSuite, InvokeResponses::Builder & aInvokeResponsesBuilder)
{
    InvokeResponseIB::Builder invokeResponseIBBuilder = aInvokeResponsesBuilder.CreateInvokeResponse();
    NL_TEST_ASSERT(apSuite, aInvokeResponsesBuilder.GetError() == CHIP_NO_ERROR);
    BuildInvokeResponseIBWithCommandDataIB(apSuite, invokeResponseIBBuilder);

    aInvokeResponsesBuilder.EndOfInvokeResponses();
    NL_TEST_ASSERT(apSuite, aInvokeResponsesBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseInvokeResponses(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponses::Parser invokeResponsesParser;
    err = invokeResponsesParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeResponsesParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildInvokeRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeRequestMessage::Builder invokeRequestMessageBuilder;
    err = invokeRequestMessageBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    invokeRequestMessageBuilder.SuppressResponse(true);
    invokeRequestMessageBuilder.TimedRequest(true);
    InvokeRequests::Builder invokeRequestsBuilder = invokeRequestMessageBuilder.CreateInvokeRequests();
    NL_TEST_ASSERT(apSuite, invokeRequestsBuilder.GetError() == CHIP_NO_ERROR);

    BuildInvokeRequests(apSuite, invokeRequestsBuilder);

    invokeRequestMessageBuilder.EndOfInvokeRequestMessage();
    NL_TEST_ASSERT(apSuite, invokeRequestMessageBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseInvokeRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeRequestMessage::Parser invokeRequestMessageParser;
    err = invokeRequestMessageParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    bool suppressResponse = false;
    bool timedRequest     = false;
    invokeRequestMessageParser.GetSuppressResponse(&suppressResponse);
    invokeRequestMessageParser.GetTimedRequest(&timedRequest);
    NL_TEST_ASSERT(apSuite, suppressResponse == true);
    NL_TEST_ASSERT(apSuite, timedRequest == true);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeRequestMessageParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildInvokeResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseMessage::Builder invokeResponseMessageBuilder;
    err = invokeResponseMessageBuilder.Init(&aWriter);

    invokeResponseMessageBuilder.SuppressResponse(true);
    InvokeResponses::Builder invokeResponsesBuilder = invokeResponseMessageBuilder.CreateInvokeResponses();
    NL_TEST_ASSERT(apSuite, invokeResponseMessageBuilder.GetError() == CHIP_NO_ERROR);

    BuildInvokeResponses(apSuite, invokeResponsesBuilder);

    invokeResponseMessageBuilder.EndOfInvokeResponseMessage();
    NL_TEST_ASSERT(apSuite, invokeResponseMessageBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseInvokeResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseMessage::Parser invokeResponseMessageParser;
    err = invokeResponseMessageParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    bool suppressResponse = false;
    invokeResponseMessageParser.GetSuppressResponse(&suppressResponse);
    NL_TEST_ASSERT(apSuite, suppressResponse == true);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeResponseMessageParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
}

void BuildReportDataMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportDataMessage::Builder reportDataMessageBuilder;

    err = reportDataMessageBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    reportDataMessageBuilder.SuppressResponse(true).SubscriptionId(2);
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    AttributeReportIBs::Builder AttributeReportIBs = reportDataMessageBuilder.CreateAttributeReportIBs();
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeReportIBs(apSuite, AttributeReportIBs);

    EventReports::Builder EventReports = reportDataMessageBuilder.CreateEventReports();
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventReports(apSuite, EventReports);

    reportDataMessageBuilder.MoreChunkedMessages(true);
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);

    reportDataMessageBuilder.EndOfReportDataMessage();
    NL_TEST_ASSERT(apSuite, reportDataMessageBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseReportDataMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReportDataMessage::Parser reportDataParser;

    bool suppressResponse   = false;
    uint64_t subscriptionId = 0;
    AttributeReportIBs::Parser attributeReportIBsParser;
    EventReports::Parser eventReportsParser;
    bool moreChunkedMessages = false;
    reportDataParser.Init(aReader);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = reportDataParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = reportDataParser.GetSuppressResponse(&suppressResponse);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && suppressResponse);

    err = reportDataParser.GetSubscriptionId(&subscriptionId);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && subscriptionId == 2);

    err = reportDataParser.GetAttributeReportIBs(&attributeReportIBsParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = reportDataParser.GetEventReports(&eventReportsParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = reportDataParser.GetMoreChunkedMessages(&moreChunkedMessages);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && moreChunkedMessages);
}

void BuildReadRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReadRequestMessage::Builder readRequestBuilder;

    err = readRequestBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathIBs::Builder AttributePathIBs = readRequestBuilder.CreateAttributeRequests();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePathList(apSuite, AttributePathIBs);

    EventPaths::Builder eventPathList = readRequestBuilder.CreateEventRequests();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPaths(apSuite, eventPathList);

    EventFilters::Builder eventFilters = readRequestBuilder.CreateEventFilters();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventFilters(apSuite, eventFilters);

    readRequestBuilder.IsFabricFiltered(true);
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);

    readRequestBuilder.EndOfReadRequestMessage();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseReadRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReadRequestMessage::Parser readRequestParser;
    AttributePathIBs::Parser attributePathListParser;
    EventPaths::Parser eventPathListParser;
    EventFilters::Parser eventFiltersParser;
    bool isFabricFiltered = false;

    err = readRequestParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = readRequestParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = readRequestParser.GetAttributeRequests(&attributePathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetEventRequests(&eventPathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetEventFilters(&eventFiltersParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = readRequestParser.GetIsFabricFiltered(&isFabricFiltered);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && isFabricFiltered);
}

void BuildWriteRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WriteRequestMessage::Builder writeRequestBuilder;

    err = writeRequestBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    writeRequestBuilder.SuppressResponse(true);
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);

    writeRequestBuilder.TimedRequest(true);
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);

    AttributeDataIBs::Builder AttributeDataIBs = writeRequestBuilder.CreateWriteRequests();
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataIBs(apSuite, AttributeDataIBs);

    writeRequestBuilder.MoreChunkedMessages(true);
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);

    writeRequestBuilder.IsFabricFiltered(true);
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);

    writeRequestBuilder.EndOfWriteRequestMessage();
    NL_TEST_ASSERT(apSuite, writeRequestBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseWriteRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    WriteRequestMessage::Parser writeRequestParser;
    bool suppressResponse = false;
    bool timeRequest      = false;
    AttributeDataIBs::Parser writeRequests;
    bool moreChunkedMessages = false;
    bool isFabricFiltered    = false;

    err = writeRequestParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = writeRequestParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = writeRequestParser.GetSuppressResponse(&suppressResponse);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && suppressResponse);

    err = writeRequestParser.GetTimedRequest(&timeRequest);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && timeRequest);

    err = writeRequestParser.GetWriteRequests(&writeRequests);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = writeRequestParser.GetMoreChunkedMessages(&moreChunkedMessages);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && moreChunkedMessages);

    err = writeRequestParser.GetIsFabricFiltered(&isFabricFiltered);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR && isFabricFiltered);
}

void BuildWriteResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WriteResponseMessage::Builder writeResponseBuilder;

    err = writeResponseBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributeStatuses::Builder attributeStatuses = writeResponseBuilder.CreateWriteResponses();
    NL_TEST_ASSERT(apSuite, writeResponseBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeStatuses(apSuite, attributeStatuses);

    writeResponseBuilder.EndOfWriteResponseMessage();
    NL_TEST_ASSERT(apSuite, writeResponseBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseWriteResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    WriteResponseMessage::Parser writeResponseParser;
    AttributeStatuses::Parser attributeStatusesParser;
    err = writeResponseParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = writeResponseParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = writeResponseParser.GetWriteResponses(&attributeStatusesParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void BuildSubscribeRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SubscribeRequestMessage::Builder subscribeRequestBuilder;

    err = subscribeRequestBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathIBs::Builder attributePathIBs = subscribeRequestBuilder.CreateAttributeRequests();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributePathList(apSuite, attributePathIBs);

    EventPaths::Builder eventPathList = subscribeRequestBuilder.CreateEventRequests();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventPaths(apSuite, eventPathList);

    EventFilters::Builder eventFilters = subscribeRequestBuilder.CreateEventFilters();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
    BuildEventFilters(apSuite, eventFilters);

    subscribeRequestBuilder.MinIntervalFloorSeconds(2);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.MaxIntervalCeilingSeconds(3);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.KeepSubscriptions(true);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.IsProxy(true);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.IsFabricFiltered(true);
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);

    subscribeRequestBuilder.EndOfSubscribeRequestMessage();
    NL_TEST_ASSERT(apSuite, subscribeRequestBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseSubscribeRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SubscribeRequestMessage::Parser subscribeRequestParser;
    AttributePathIBs::Parser attributePathListParser;
    EventPaths::Parser eventPathListParser;
    EventFilters::Parser eventFiltersParser;
    uint16_t MinIntervalFloorSeconds   = 0;
    uint16_t MaxIntervalCeilingSeconds = 0;
    bool keepExistingSubscription      = false;
    bool isProxy                       = false;
    bool isFabricFiltered              = false;

    err = subscribeRequestParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = subscribeRequestParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = subscribeRequestParser.GetAttributeRequests(&attributePathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetEventRequests(&eventPathListParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetEventFilters(&eventFiltersParser);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetMinIntervalFloorSeconds(&MinIntervalFloorSeconds);
    NL_TEST_ASSERT(apSuite, MinIntervalFloorSeconds == 2 && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetMaxIntervalCeilingSeconds(&MaxIntervalCeilingSeconds);
    NL_TEST_ASSERT(apSuite, MaxIntervalCeilingSeconds == 3 && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetKeepSubscriptions(&keepExistingSubscription);
    NL_TEST_ASSERT(apSuite, keepExistingSubscription && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetIsProxy(&isProxy);
    NL_TEST_ASSERT(apSuite, isProxy && err == CHIP_NO_ERROR);

    err = subscribeRequestParser.GetIsFabricFiltered(&isFabricFiltered);
    NL_TEST_ASSERT(apSuite, isFabricFiltered && err == CHIP_NO_ERROR);
}

void BuildSubscribeResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SubscribeResponseMessage::Builder subscribeResponseBuilder;

    err = subscribeResponseBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    subscribeResponseBuilder.SubscriptionId(1);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.MinIntervalFloorSeconds(1);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.MaxIntervalCeilingSeconds(2);
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);

    subscribeResponseBuilder.EndOfSubscribeResponseMessage();
    NL_TEST_ASSERT(apSuite, subscribeResponseBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseSubscribeResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SubscribeResponseMessage::Parser subscribeResponseParser;
    uint64_t subscriptionId            = 0;
    uint16_t minIntervalFloorSeconds   = 0;
    uint16_t maxIntervalCeilingSeconds = 0;
    err                                = subscribeResponseParser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = subscribeResponseParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = subscribeResponseParser.GetSubscriptionId(&subscriptionId);
    NL_TEST_ASSERT(apSuite, subscriptionId == 1 && err == CHIP_NO_ERROR);

    err = subscribeResponseParser.GetMinIntervalFloorSeconds(&minIntervalFloorSeconds);
    NL_TEST_ASSERT(apSuite, minIntervalFloorSeconds == 1 && err == CHIP_NO_ERROR);

    err = subscribeResponseParser.GetMaxIntervalCeilingSeconds(&maxIntervalCeilingSeconds);
    NL_TEST_ASSERT(apSuite, maxIntervalCeilingSeconds == 2 && err == CHIP_NO_ERROR);
}

void BuildTimedRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TimedRequestMessage::Builder TimedRequestMessageBuilder;

    err = TimedRequestMessageBuilder.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    TimedRequestMessageBuilder.TimeoutMs(1);
    NL_TEST_ASSERT(apSuite, TimedRequestMessageBuilder.GetError() == CHIP_NO_ERROR);
}

void ParseTimedRequestMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    TimedRequestMessage::Parser TimedRequestMessagearser;
    uint16_t timeout = 0;

    err = TimedRequestMessagearser.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = TimedRequestMessagearser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    err = TimedRequestMessagearser.GetTimeoutMs(&timeout);
    NL_TEST_ASSERT(apSuite, timeout == 1 && err == CHIP_NO_ERROR);
}

void EventFilterTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventFilterIB::Builder eventFilterBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventFilterBuilder.Init(&writer);
    BuildEventFilterIB(apSuite, eventFilterBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventFilterIB(apSuite, reader);
}

void EventFiltersTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    EventFilters::Builder eventFiltersBuilder;
    EventFilters::Parser eventFiltersParser;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    err = eventFiltersBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    BuildEventFilters(apSuite, eventFiltersBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventFilters(apSuite, reader);
}

void AttributePathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributePathIB::Builder attributePathBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributePathBuilder.Init(&writer);
    BuildAttributePathIB(apSuite, attributePathBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ParseAttributePathIB(apSuite, reader);
}

void AttributePathListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    AttributePathIBs::Builder attributePathListBuilder;

    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    err = attributePathListBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    BuildAttributePathList(apSuite, attributePathListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributePathList(apSuite, reader);
}

void EventPathTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventPathIB::Parser eventPathParser;
    EventPathIB::Builder eventPathBuilder;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventPathBuilder.Init(&writer);
    BuildEventPath(apSuite, eventPathBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    eventPathParser.Init(reader);
    ParseEventPath(apSuite, eventPathParser);
}

void EventPathsTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    EventPaths::Builder eventPathListBuilder;

    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));

    err = eventPathListBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    BuildEventPaths(apSuite, eventPathListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventPaths(apSuite, reader);
}

void CommandPathIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    CommandPathIB::Builder commandPathBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    err = commandPathBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    BuildCommandPath(apSuite, commandPathBuilder);

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    ParseCommandPath(apSuite, reader);
}

void EventDataIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventDataIB::Builder eventDataIBBuilder;
    EventDataIB::Parser eventDataIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventDataIBBuilder.Init(&writer);
    BuildEventDataIB(apSuite, eventDataIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    eventDataIBParser.Init(reader);
    ParseEventDataIB(apSuite, eventDataIBParser);
}

void EventReportIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventReportIB::Builder eventReportIBBuilder;
    EventReportIB::Parser eventReportIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventReportIBBuilder.Init(&writer);
    BuildEventReportIB(apSuite, eventReportIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    eventReportIBParser.Init(reader);
    ParseEventReportIB(apSuite, eventReportIBParser);
}

void EventReportsTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    EventReports::Builder eventReportsBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventReportsBuilder.Init(&writer);
    BuildEventReports(apSuite, eventReportsBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseEventReports(apSuite, reader);
}

void AttributeReportIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeReportIB::Builder attributeReportIBBuilder;
    AttributeReportIB::Parser attributeReportIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributeReportIBBuilder.Init(&writer);
    BuildAttributeReportIB(apSuite, attributeReportIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeReportIBParser.Init(reader);
    ParseAttributeReportIB(apSuite, attributeReportIBParser);
}

void AttributeReportIBsTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    AttributeReportIBs::Builder attributeReportIBsBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributeReportIBsBuilder.Init(&writer);
    BuildAttributeReportIBs(apSuite, attributeReportIBsBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributeReportIBs(apSuite, reader);
}

void StatusIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusIB::Builder statusIBBuilder;
    StatusIB::Parser StatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    statusIBBuilder.Init(&writer);
    BuildStatusIB(apSuite, statusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    StatusIBParser.Init(reader);
    ParseStatusIB(apSuite, StatusIBParser);
}

void EventStatusIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventStatusIB::Builder eventStatusIBBuilder;
    EventStatusIB::Parser eventStatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    eventStatusIBBuilder.Init(&writer);
    BuildEventStatusIB(apSuite, eventStatusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    eventStatusIBParser.Init(reader);
    ParseEventStatusIB(apSuite, eventStatusIBParser);
}

void AttributeStatusIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeStatusIB::Builder attributeStatusIBBuilder;
    AttributeStatusIB::Parser attributeStatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    attributeStatusIBBuilder.Init(&writer);
    BuildAttributeStatusIB(apSuite, attributeStatusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    attributeStatusIBParser.Init(reader);
    ParseAttributeStatusIB(apSuite, attributeStatusIBParser);
}

void AttributeStatusesTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeStatuses::Builder attributeStatusesBuilder;
    err = attributeStatusesBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    BuildAttributeStatuses(apSuite, attributeStatusesBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributeStatuses(apSuite, reader);
}

void AttributeDataIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    AttributeDataIB::Builder AttributeDataIBBuilder;
    AttributeDataIB::Parser AttributeDataIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeDataIBBuilder.Init(&writer);
    BuildAttributeDataIB(apSuite, AttributeDataIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributeDataIBParser.Init(reader);
    ParseAttributeDataIB(apSuite, AttributeDataIBParser);
}

void AttributeDataIBsTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeDataIBs::Builder AttributeDataIBsBuilder;
    AttributeDataIBsBuilder.Init(&writer);
    BuildAttributeDataIBs(apSuite, AttributeDataIBsBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributeDataIBs(apSuite, reader);
}

void AttributeDataVersionListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeDataVersionList::Builder attributeDataVersionListBuilder;
    attributeDataVersionListBuilder.Init(&writer);
    BuildAttributeDataVersionList(apSuite, attributeDataVersionListBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseAttributeDataVersionList(apSuite, reader);
}

void CommandDataIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandDataIB::Builder commandDataIBBuilder;
    CommandDataIB::Parser commandDataIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    commandDataIBBuilder.Init(&writer);
    BuildCommandDataIB(apSuite, commandDataIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandDataIBParser.Init(reader);
    ParseCommandDataIB(apSuite, commandDataIBParser);
}

void CommandStatusIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandStatusIB::Builder commandStatusIBBuilder;
    CommandStatusIB::Parser commandStatusIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    commandStatusIBBuilder.Init(&writer);
    BuildCommandStatusIB(apSuite, commandStatusIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    commandStatusIBParser.Init(reader);
    ParseCommandStatusIB(apSuite, commandStatusIBParser);
}

void InvokeResponseIBWithCommandDataIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseIB::Builder invokeResponseIBBuilder;
    InvokeResponseIB::Parser invokeResponseIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponseIBBuilder.Init(&writer);
    BuildInvokeResponseIBWithCommandDataIB(apSuite, invokeResponseIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    invokeResponseIBParser.Init(reader);
    ParseInvokeResponseIBWithCommandDataIB(apSuite, invokeResponseIBParser);
}

void InvokeResponseIBWithCommandStatusIBTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseIB::Builder invokeResponseIBBuilder;
    InvokeResponseIB::Parser invokeResponseIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponseIBBuilder.Init(&writer);
    BuildInvokeResponseIBWithCommandStatusIB(apSuite, invokeResponseIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    invokeResponseIBParser.Init(reader);
    ParseInvokeResponseIBWithCommandStatusIB(apSuite, invokeResponseIBParser);
}

void InvokeResponseIBWithMalformDataTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeResponseIB::Builder invokeResponseIBBuilder;
    InvokeResponseIB::Parser invokeResponseIBParser;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponseIBBuilder.Init(&writer);
    BuildWrongInvokeResponseIB(apSuite, invokeResponseIBBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    invokeResponseIBParser.Init(reader);
    ParseWrongInvokeResponseIB(apSuite, invokeResponseIBParser);
}

void InvokeRequestsTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    InvokeRequests::Builder invokeRequestsBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeRequestsBuilder.Init(&writer);
    BuildInvokeRequests(apSuite, invokeRequestsBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseInvokeRequests(apSuite, reader);
}

void InvokeResponsesTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    InvokeResponses::Builder invokeResponsesBuilder;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    invokeResponsesBuilder.Init(&writer);
    BuildInvokeResponses(apSuite, invokeResponsesBuilder);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseInvokeResponses(apSuite, reader);
}

void InvokeInvokeRequestMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildInvokeRequestMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseInvokeRequestMessage(apSuite, reader);
}

void InvokeInvokeResponseMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildInvokeResponseMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseInvokeResponseMessage(apSuite, reader);
}

void ReportDataMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildReportDataMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseReportDataMessage(apSuite, reader);
}

void ReadRequestMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildReadRequestMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseReadRequestMessage(apSuite, reader);
}

void WriteRequestMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildWriteRequestMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseWriteRequestMessage(apSuite, reader);
}

void WriteResponseMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildWriteResponseMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseWriteResponseMessage(apSuite, reader);
}

void SubscribeRequestMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildSubscribeRequestMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseSubscribeRequestMessage(apSuite, reader);
}

void SubscribeResponseMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildSubscribeResponseMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseSubscribeResponseMessage(apSuite, reader);
}

void TimedRequestMessageTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildTimedRequestMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseTimedRequestMessage(apSuite, reader);
}

void CheckPointRollbackTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    size_t NumDataElement = 0;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    AttributeDataIBs::Parser AttributeDataIBsParser;
    chip::TLV::TLVWriter checkpoint;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    AttributeDataIBs::Builder AttributeDataIBsBuilder;
    AttributeDataIBsBuilder.Init(&writer);

    // encode one attribute element
    AttributeDataIB::Builder AttributeDataIBBuilder1 = AttributeDataIBsBuilder.CreateAttributeDataIBBuilder();
    NL_TEST_ASSERT(apSuite, AttributeDataIBsBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataIB(apSuite, AttributeDataIBBuilder1);
    // checkpoint
    AttributeDataIBsBuilder.Checkpoint(checkpoint);
    // encode another attribute element
    AttributeDataIB::Builder AttributeDataIBBuilder2 = AttributeDataIBsBuilder.CreateAttributeDataIBBuilder();
    NL_TEST_ASSERT(apSuite, AttributeDataIBsBuilder.GetError() == CHIP_NO_ERROR);
    BuildAttributeDataIB(apSuite, AttributeDataIBBuilder2);
    // rollback to previous checkpoint
    AttributeDataIBsBuilder.Rollback(checkpoint);

    AttributeDataIBsBuilder.EndOfAttributeDataIBs();
    NL_TEST_ASSERT(apSuite, AttributeDataIBsBuilder.GetError() == CHIP_NO_ERROR);

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    DebugPrettyPrint(buf);

    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = AttributeDataIBsParser.Init(reader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = AttributeDataIBsParser.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif
    while (CHIP_NO_ERROR == (err = AttributeDataIBsParser.Next()))
    {
        ++NumDataElement;
    }

    NL_TEST_ASSERT(apSuite, NumDataElement == 1);
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("EventFilterTest", EventFilterTest),
                NL_TEST_DEF("EventFiltersTest", EventFiltersTest),
                NL_TEST_DEF("AttributePathTest", AttributePathTest),
                NL_TEST_DEF("AttributePathListTest", AttributePathListTest),
                NL_TEST_DEF("AttributeStatusIBTest", AttributeStatusIBTest),
                NL_TEST_DEF("AttributeStatusesTest", AttributeStatusesTest),
                NL_TEST_DEF("AttributeDataIBTest", AttributeDataIBTest),
                NL_TEST_DEF("AttributeDataIBsTest", AttributeDataIBsTest),
                NL_TEST_DEF("AttributeReportIBTest", AttributeReportIBTest),
                NL_TEST_DEF("AttributeReportIBsTest", AttributeReportIBsTest),
                NL_TEST_DEF("EventPathTest", EventPathTest),
                NL_TEST_DEF("EventPathsTest", EventPathsTest),
                NL_TEST_DEF("EventDataIBTest", EventDataIBTest),
                NL_TEST_DEF("EventReportIBTest", EventReportIBTest),
                NL_TEST_DEF("EventReportsTest", EventReportsTest),
                NL_TEST_DEF("StatusIBTest", StatusIBTest),
                NL_TEST_DEF("EventStatusIBTest", EventStatusIBTest),
                NL_TEST_DEF("AttributeDataVersionListTest", AttributeDataVersionListTest),
                NL_TEST_DEF("CommandPathIBTest", CommandPathIBTest),
                NL_TEST_DEF("CommandDataIBTest", CommandDataIBTest),
                NL_TEST_DEF("CommandStatusIBTest", CommandStatusIBTest),
                NL_TEST_DEF("InvokeResponseIBWithCommandDataIBTest", InvokeResponseIBWithCommandDataIBTest),
                NL_TEST_DEF("InvokeResponseIBWithCommandStatusIBTest", InvokeResponseIBWithCommandStatusIBTest),
                NL_TEST_DEF("InvokeResponseIBWithMalformDataTest", InvokeResponseIBWithMalformDataTest),
                NL_TEST_DEF("InvokeRequestsTest", InvokeRequestsTest),
                NL_TEST_DEF("InvokeResponsesTest", InvokeResponsesTest),
                NL_TEST_DEF("InvokeInvokeRequestMessageTest", InvokeInvokeRequestMessageTest),
                NL_TEST_DEF("InvokeInvokeResponseMessageTest", InvokeInvokeResponseMessageTest),
                NL_TEST_DEF("ReportDataMessageTest", ReportDataMessageTest),
                NL_TEST_DEF("ReadRequestMessageTest", ReadRequestMessageTest),
                NL_TEST_DEF("WriteRequestMessageTest", WriteRequestMessageTest),
                NL_TEST_DEF("WriteResponseMessageTest", WriteResponseMessageTest),
                NL_TEST_DEF("SubscribeRequestMessageTest", SubscribeRequestMessageTest),
                NL_TEST_DEF("SubscribeResponseMessageTest", SubscribeResponseMessageTest),
                NL_TEST_DEF("TimedRequestMessageTest", TimedRequestMessageTest),
                NL_TEST_DEF("CheckPointRollbackTest", CheckPointRollbackTest),
                NL_TEST_SENTINEL()
        };
// clang-format on
} // namespace

/**
 *  Set up the test suite.
 */
static int TestSetup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestMessageDef()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "MessageDef",
        &sTests[0],
        TestSetup,
        TestTeardown,
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestMessageDef)
