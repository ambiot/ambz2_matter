#!/usr/bin/env bash

#
# Copyright (c) 2021-2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Script that can be used to generate CHIP device attestation certificates
# for testing purposes.
# The script expects the path to the chip-cert tool binary as an input argument.
#
# Generates C-Style file with those certificates/keys to be use by the SDK tests:
#     ./credentials/test/gen-test-attestation-certs.sh ./out/debug/standalone/chip-cert CHIPAttCert_test_vectors
#
# In addition to the DER/PEM files this command also generates the following C-Style files:
#     src/credentials/tests/CHIPAttCert_test_vectors.cpp
#     src/credentials/tests/CHIPAttCert_test_vectors.h
#

set -e

if [ $# == 3 ]; then
    chip_dir=$1
    chip_cert_tool=$2
    output_cstyle_file=$3
else
    echo "Error: Please specify three input arguments"
    echo "First argument = the path to the connectedhomeip sdk"
    echo "Second argument = the path to the chip-cert tool binary"
    echo "Third argument = the name of the output C-Style file"
    exit
fi

dest_dir="$chip_dir/myattestation"

rm -rf "$dest_dir"
mkdir -p "$dest_dir"

cert_valid_from="2022-07-10 14:23:43"
cert_lifetime=4294967295

vid=1316
pid=1A01
format_version=1
device_type_id=0x0016
certificate_id="ZIG20142ZB330003-24"
security_level=0
security_info=0
version_num=0x2694
certification_type=0

# Generate PAA, PAI and DAC
{
    # Matter's test PAA (uncomment if you want to use Matter's test PAA)
    # paa_key_file="$chip_dir/credentials/test/attestation/Chip-Test-PAA-NoVID-Key"
    # paa_cert_file="$chip_dir/credentials/test/attestation/Chip-Test-PAA-NoVID-Cert"

    # Self generated PAA
    paa_key_file="$dest_dir/Chip-Test-PAA-NoVID-Key"
    paa_cert_file="$dest_dir/Chip-Test-PAA-NoVID-Cert"

    # If you are using Matter's test PAA, don't need to generate it, comment below line
    "$chip_cert_tool" gen-att-cert --type a --subject-cn "Matter Test PAA" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --out-key "$paa_key_file".pem --out "$paa_cert_file".pem

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-NoPID-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-NoPID-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem
}

# In addition to PEM format also create certificates in DER form.
for cert_file_pem in "$dest_dir"/*Cert.pem; do
    cert_file_der="${cert_file_pem/.pem/.der}"
    "$chip_cert_tool" convert-cert "$cert_file_pem" "$cert_file_der" --x509-der
done

# In addition to PEM format also create private key in DER form.
for key_file_pem in "$dest_dir"/*Key.pem; do
    key_file_der="${key_file_pem/.pem/.der}"
    "$chip_cert_tool" convert-key "$key_file_pem" "$key_file_der" --x509-der
done

# Print generated certificate, keys, and parameters in C-Style to use in the SDK if the output file is provided.

    copyright_note='/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
'

    cpp_includes='
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
'
    header_includes='
#pragma once
'

    namespaces_open='
namespace chip {
namespace TestCerts {
'
    namespaces_close='
} // namespace TestCerts
} // namespace chip
'

printf "$copyright_note" >"$dest_dir/$output_cstyle_file".cpp
printf "$copyright_note" >"$dest_dir/$output_cstyle_file".h
printf "$cpp_includes" >>"$dest_dir/$output_cstyle_file".cpp
printf "$header_includes" >>"$dest_dir/$output_cstyle_file".h
printf "$namespaces_open\n" >>"$dest_dir/$output_cstyle_file".cpp
printf "$namespaces_open\n" >>"$dest_dir/$output_cstyle_file".h
for cert_file_pem in "$dest_dir/"*Cert.pem; do
    params_prefix="${cert_file_pem/*Chip-Test/sTestCert}"
    params_prefix="${params_prefix//-/_}"
    params_prefix="${params_prefix/_Cert.pem/}"

    cert_file_der="${cert_file_pem/.pem/.der}"
    key_file_pem="${cert_file_pem/Cert.pem/Key.pem}"

    {
        printf "// \${chip_root}/$cert_file_pem\n\n"

        printf "constexpr uint8_t ${params_prefix}_Cert_Array[] = {\n"
        less -f "$cert_file_der" | od -t x1 -An | sed 's/\</0x/g' | sed 's/\>/,/g' | sed 's/^/   /g'
        printf "};\n\n"
        printf "extern const ByteSpan ${params_prefix}_Cert = ByteSpan(${params_prefix}_Cert_Array);\n\n"

        printf "constexpr uint8_t ${params_prefix}_SKID_Array[] = {\n"
        openssl x509 -text -noout -in "$cert_file_pem" | sed '0,/X509v3 Subject Key Identifier:/d' | sed '2,$d' | sed 's/:/ /g' | sed 's/\</0x/g' | sed 's/\>/,/g' | sed "s/^[ \t]*/    /"
        printf "};\n\n"
        printf "extern const ByteSpan ${params_prefix}_SKID = ByteSpan(${params_prefix}_SKID_Array);\n\n"

        printf "// \${chip_root}/$key_file_pem\n\n"

        printf "constexpr uint8_t ${params_prefix}_PublicKey_Array[] = {\n"
        openssl ec -text -noout -in "$key_file_pem" | sed '0,/pub:$/d' | sed '/ASN1 OID:/,$d' | sed 's/:/ /g' | sed 's/\</0x/g' | sed 's/\>/,/g' | sed "s/^[ \t]*/    /" | sed 's/ *$//'
        printf "};\n\n"
        printf "extern const ByteSpan ${params_prefix}_PublicKey = ByteSpan(${params_prefix}_PublicKey_Array);\n\n"

        printf "constexpr uint8_t ${params_prefix}_PrivateKey_Array[] = {\n"
        openssl ec -text -noout -in "$key_file_pem" | sed '0,/priv:$/d' | sed '/pub:/,$d' | sed 's/:/ /g' | sed 's/\</0x/g' | sed 's/\>/,/g' | sed "s/^[ \t]*/    /" | sed 's/ *$//'
        printf "};\n\n"
        printf "extern const ByteSpan ${params_prefix}_PrivateKey = ByteSpan(${params_prefix}_PrivateKey_Array);\n\n"
    } >>"$dest_dir/$output_cstyle_file".cpp

    {
        printf "extern const ByteSpan ${params_prefix}_Cert;\n"
        printf "extern const ByteSpan ${params_prefix}_SKID;\n"
        printf "extern const ByteSpan ${params_prefix}_PublicKey;\n"
        printf "extern const ByteSpan ${params_prefix}_PrivateKey;\n\n"
    } >>"$dest_dir/$output_cstyle_file".h

done
printf "$namespaces_close" >>"$dest_dir/$output_cstyle_file".cpp
printf "$namespaces_close" >>"$dest_dir/$output_cstyle_file".h

#cp "$dest_dir/$output_cstyle_file".cpp $dest_dir
#cp "$dest_dir/$output_cstyle_file".h $dest_dir

# Generate Credential Declaration
cd_signing_key="$chip_dir/credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem"
cd_signing_cert="$chip_dir/credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem"

"$chip_cert_tool" gen-cd --key "$cd_signing_key" --cert "$cd_signing_cert" --out "$dest_dir/Chip-Test-CD-$vid-$pid.der" --format-version "$format_version" --vendor-id "0x$vid" --product-id "0x$pid" --device-type-id "$device_type_id" --certificate-id "$certificate_id" --security-level "$security_level" --security-info "$security_info" --version-number "$version_num" --certification-type "$certification_type"
