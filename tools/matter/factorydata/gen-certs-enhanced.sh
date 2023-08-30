#!/usr/bin/env bash

#
# Copyright (c) 2021-2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#	http://www.apache.org/licenses/LICENSE-2.0
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
#	./credentials/test/gen-test-attestation-certs.sh ./out/debug/standalone/chip-cert CHIPAttCert_test_vectors
#
# In addition to the DER/PEM files this command also generates the following C-Style files:
#	src/credentials/tests/CHIPAttCert_test_vectors.cpp
#	src/credentials/tests/CHIPAttCert_test_vectors.h
#

set -e

chip_dir=$1
chip_cert_tool=$2
vid=$3
pid=$4
paa_cert=$5
paa_key=$6
endproduct_vid=$7
endproduct_pid=$8

if [ $# -le 3 ]; then
	printf "Please specify input arguments\n"
	printf "./gen-certs.sh chip_dir chip_cert_tool vid pid <paa_cert> <paa_key> <endproduct_vid> <endproduct_pid>\n"
	exit
fi

dest_dir="$PWD/myattestation"

if [ -d "$dest_dir" ]; then
	while true; do
		read -p 'Do you want to remove directory (y/n)? ' removed
		case $removed in
			[Yy]* )
			printf "\nRemoving and creating new folder $dest_dir\n\n"
			rm -rf $dest_dir; 
			mkdir $dest_dir
			break;;
			[Nn]* ) exit;;
			* ) printf "Please answer yes or no.\n\n";;
		esac
	done
else
	printf "Creating directory $dest_dir\n\n"
	mkdir $dest_dir
fi

cert_valid_from="2023-08-14 14:23:43"
cert_lifetime=4294967295
format_version=1
device_type_id=0x0016
certificate_id="ZIG20142ZB330003-24"
security_level=0
security_info=0
version_num=0x2694
certification_type=1

# Generate PAA, PAI and DAC
{
	##### 1. PAA Certificates and Keys

	if [ -z $paa_cert ] && [ -z $paa_key ]; then
		printf "No existing PAA. Generating new PAA\n\n"
		paa_key_file="$dest_dir/Chip-Test-PAA-$vid-Key"
		paa_cert_file="$dest_dir/Chip-Test-PAA-$vid-Cert"

		"$chip_cert_tool" gen-att-cert --type a --subject-cn "Matter Test PAA" --subject-vid "$vid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --out-key "$paa_key_file".pem --out "$paa_cert_file".pem
	else
		printf "Using existing PAA\n\n"
		paa_key_file="${paa_key::-4}"
		paa_cert_file="${paa_cert::-4}"
	fi

	##### 2. PAI Certificates and Keys

	while true; do
		read -p 'Is PAI vid-scoped (y/n)? ' scoped
		case $scoped in
			[Yy]* ) scoped=1; break;;
			[Nn]* ) scoped=0; break;;
			* ) printf "Please answer yes or no.\n\n";;
		esac
	done
	
	if [ $scoped == 1 ]; then
		printf "\nGenerating vid-scoped PAI of vid $vid\n\n"
		pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Key"
		pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Cert"
	else
		printf "\nGenerating non-vid-scoped PAI of vid $vid and pid $pid\n\n"
		pai_key_file="$dest_dir/Chip-Test-PAI-$vid-NoPID-Key"
		pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-NoPID-Cert"
	fi

	"$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

	##### 3. DAC Certificates and Keys

	printf "Generating DAC of vid $vid and pid $pid\n\n"
	
	read -p 'How many DAC do you want to generate? ' number

	for (( i=1; i<$number; i++ ))
	do
		dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-Key-$i"
		dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-Cert-$i"

		"$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

		"$chip_cert_tool" validate-att-cert --dac "$dac_cert_file".pem --pai "$pai_cert_file".pem --paa "$paa_cert_file".pem
	done
}

printf "Converting Certificates and Keys from PEM to DER\n\n"
#In addition to PEM format also create certificates in DER form.
for cert_file_pem in "$dest_dir"/*Cert*.pem; do
	cert_file_der="${cert_file_pem/.pem/.der}"
	"$chip_cert_tool" convert-cert "$cert_file_pem" "$cert_file_der" --x509-der
done

# In addition to PEM format also create private key in DER form.
for key_file_pem in "$dest_dir"/*Key*.pem; do
	key_file_der="${key_file_pem/.pem/.der}"
	"$chip_cert_tool" convert-key "$key_file_pem" "$key_file_der" --x509-der
done

###### 4. Generate Credential Declaration
cd_signing_key="$chip_dir/credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem"
cd_signing_cert="$chip_dir/credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem"

if [ -z $endproduct_vid ] && [ -z $endproduct_pid ]; then
	printf "Generating CD without dac_origin_vid and dac_origin_pid\n"
	"$chip_cert_tool" gen-cd --key "$cd_signing_key" --cert "$cd_signing_cert" --out "$dest_dir/Chip-Test-CD-$vid-$pid.der" --format-version "$format_version" --vendor-id "0x$vid" --product-id "0x$pid" --device-type-id "$device_type_id" --certificate-id "$certificate_id" --security-level "$security_level" --security-info "$security_info" --version-number "$version_num" --certification-type "$certification_type"
else
	printf "\nGenerating CD with dac_origin_vid $endproduct_vid and dac_origin_pid $endproduct_pid\n"
	"$chip_cert_tool" gen-cd --key "$cd_signing_key" --cert "$cd_signing_cert" --out "$dest_dir/Chip-Test-CD-$endproduct_vid-$endproduct_pid-WithDACOrigin.der" --format-version "$format_version" --vendor-id "$endproduct_vid" --product-id "$endproduct_pid" --device-type-id "$device_type_id" --certificate-id "$certificate_id" --security-level "$security_level" --security-info "$security_info" --version-number "$version_num" --certification-type "$certification_type" --dac-origin-vendor-id "$vid" --dac-origin-product-id "$pid"
fi
