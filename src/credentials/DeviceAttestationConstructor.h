/*
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

#include <credentials/DeviceAttestationVendorReserved.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace Credentials {

/**
 *  @brief Take the attestation elements buffer and return each component seperately.
 *         All output data stays valid while attestationElements buffer is valid.
 *
 *  @param[in]    attestationElements ByteSpan containg source of Attestation Elements data.
 *  @param[out]   certificationDeclaration
 *  @param[out]   attestationNonce
 *  @param[out]   timestamp
 *  @param[out]   firmwareInfo ByteSpan containing Firmware Information data if present within attestationElements.
 *                             Empty ByteSpan if not present in attestationElements.
 *  @param[out]   VendorReserved  Placeholder to for client to examine VendorReserved elements later
 */
CHIP_ERROR DeconstructAttestationElements(const ByteSpan & attestationElements, ByteSpan & certificationDeclaration,
                                          ByteSpan & attestationNonce, uint32_t & timestamp, ByteSpan & firmwareInfo,
                                          DeviceAttestationVendorReservedDeconstructor & vendorReserved);

/**
 *  @brief Take each component separately and form the Attestation Elements buffer.
 *
 *  @param[in]  certificationDeclaration Valid Certification Declaration data.
 *  @param[in]  attestationNonce Attestation Nonce - 32 octets required.
 *  @param[in]  timestamp Timestamp data in epoch time format.
 *  @param[in]  firmwareInfo Optional Firmware Information data - Can be empty.
 *  @param[in]  VendorReserved    Prefilled-in vendor reserved elements to be put into DA elements.
 *  @param[out] attestationElements Buffer used to write all AttestationElements data, formed with all the data fields above.
 *                                  Provided buffer needs to be capable to handle all data fields + tags.
 */
CHIP_ERROR ConstructAttestationElements(const ByteSpan & certificationDeclaration, const ByteSpan & attestationNonce,
                                        uint32_t timestamp, const ByteSpan & firmwareInfo,
                                        DeviceAttestationVendorReservedConstructor & vendorReserved,
                                        MutableByteSpan & attestationElements);

/***
 *  @brief Count the number of VendorReservedElements in a DeviceAttestation blob
 *
 *  @param[in]   attestationElements ByeSpan conitaining source of Attestation Elements data
 *  @param[out]
 *  @returns CHIP_NO_ERROR on success
 */
CHIP_ERROR CountVendorReservedElementsInDA(const ByteSpan & attestationElements, size_t & numElements);

} // namespace Credentials
} // namespace chip
