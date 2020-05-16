/*  Copyright (c) 2019, Karsten Ohme
 *  This file is part of GlobalPlatform.
 *
 *  GlobalPlatform is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GlobalPlatform is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with GlobalPlatform.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <setjmp.h>
#include "globalplatform/globalplatform.h"
#include <stdio.h>
#include <string.h>
#include <cmocka.h>
#include "testUtil.h"

/**
 * Global card context for the test.
 */
static OPGP_CARD_CONTEXT cardContext;

/**
 * Global card info for the test.
 */
static OPGP_CARD_INFO cardInfo;

/**
 * GP 2.1.1 Security Info.
 */
static GP211_SECURITY_INFO securityInfo211;

static void delete_application(void **state) {
	OPGP_ERROR_STATUS status;

	BYTE hostChallenge[] = {0xB2, 0xBD, 0xE1, 0xA2, 0xDC, 0x66, 0xBD, 0x56};

	BYTE sMac[] = {0x58, 0x56, 0x33, 0x62, 0xEC, 0x5A, 0x45, 0x41, 0xAB, 0xCD, 0x32, 0xB3, 0x4B, 0x1E, 0xAE, 0x7D};
	BYTE sEnc[] = {0xF9, 0x95, 0xD0, 0xA0, 0x69, 0x33, 0x5C, 0x7D, 0xF4, 0x2E, 0x59, 0x03, 0x17, 0xFF, 0xEA, 0x6D};
	BYTE dek[] = {0x0A, 0x02, 0xA6, 0xD6, 0x87, 0x40, 0x6D, 0xCF, 0xA0, 0x9D, 0xC7, 0x0B, 0x3E, 0xDB, 0x7E, 0x38};

	BYTE sessionEnc[] = {0x9B, 0x84, 0x1B, 0xB6, 0x3D, 0x10, 0x87, 0x48, 0x4F, 0xF7, 0xF1, 0x68, 0x02, 0x01, 0x72, 0x43};
	BYTE sessionCMac[] = {0x81, 0x17, 0xFE, 0x5A, 0xDE, 0xD7, 0x4A, 0xC0, 0xDA, 0xAB, 0xE9, 0xFB, 0x81, 0xDB, 0xDB, 0x67};
	BYTE sessionRMac[] = {0x45, 0x21, 0xD7, 0x5E, 0x4D, 0x6F, 0x60, 0xDB, 0xFB, 0x8A, 0xB4, 0xEE, 0x25, 0x4E, 0xF6, 0xF1};

	BYTE initializeUpdateRequest[] = {0x80, 0x50, 0x00, 0x00, 0x08, 0xB2, 0xBD, 0xE1, 0xA2, 0xDC, 0x66, 0xBD, 0x56, 0x00};
	BYTE initializeUpdateResponse[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x03, 0x70, 0xC4, 0x09, 0x32, 0xA6, 0xFE, 0xFE, 0xAE, 0xB2, 0xE1,
			0x27, 0x18, 0x51, 0x70, 0xF4, 0x5F, 0xCA, 0x00, 0x00, 0x15, 0x90, 0x00};
	DWORD initializeUpdateResponseLength = sizeof(initializeUpdateResponse);

	BYTE extAuthRequest[] = {0x84, 0x82, 0x03, 0x00, 0x10, 0x59, 0xD5, 0x4D, 0x93, 0x24, 0x23, 0xD2, 0x9F, 0x63, 0x13, 0xC9, 0x70, 0x87, 0x5F, 0x8C, 0x2F};
	BYTE extAuthResponse[] = {0x90, 0x00};
	DWORD extAuthResponseLength = sizeof(extAuthResponse);

	BYTE deleteRequest[] = {0x84, 0xE4, 0x00, 0x80, 0x18, 0x29, 0xF0, 0x6A, 0xEC,
			0x31, 0x80, 0xB3, 0xEF, 0xE9, 0x1F, 0xF6, 0x62, 0x31, 0x0B, 0x94, 0x5D, 0xE0, 0xD5, 0xFE, 0x66, 0xAD, 0x67, 0xA5, 0xB6};
	BYTE deleteResponse[] = {0x00, 0x90, 0x00};
	DWORD deleteResponseLength = sizeof(deleteResponse);

	BYTE cardChallenge[] = {0xC4, 0x09, 0x32, 0xA6, 0xFE, 0xFE, 0xAE, 0xB2};
	BYTE cardCryptogram[] = {0xE1, 0x27, 0x18, 0x51, 0x70, 0xF4, 0x5F, 0xCA};
	BYTE sequenceCounter[] = {0x00, 0x00, 0x15};

	OPGP_AID appAid;
	BYTE aid[] = {0xA0, 0x01, 0x00, 0x01, 0x51, 0x41, 0x43, 0x4C};
	memcpy(appAid.AID, aid, sizeof(aid));
	appAid.AIDLength = sizeof(aid);

	GP211_RECEIPT_DATA receiptData;
	DWORD receiptDataLength;

	will_return(__wrap_RAND_bytes, hostChallenge);
	expect_value(__wrap_RAND_bytes, num, 8);

	expect_memory(send_APDU, capdu, initializeUpdateRequest, sizeof(initializeUpdateRequest));
	will_return(send_APDU, initializeUpdateResponse);
	will_return(send_APDU, &initializeUpdateResponseLength);

	expect_memory(send_APDU, capdu, extAuthRequest, sizeof(extAuthRequest));
	will_return(send_APDU, extAuthResponse);
	will_return(send_APDU, &extAuthResponseLength);

	memcpy(securityInfo211.invokingAid, (void *)GP231_ISD_AID, sizeof(GP231_ISD_AID));
	securityInfo211.invokingAidLength = sizeof(GP231_ISD_AID);

	status = GP211_mutual_authentication(cardContext, cardInfo, NULL, sEnc, sMac, dek, 0, 0,
			GP211_SCP03, GP211_SCP03_IMPL_i70, GP211_SCP03_SECURITY_LEVEL_C_DEC_C_MAC, 0, &securityInfo211);
	assert_int_equal(status.errorCode, OPGP_ERROR_STATUS_SUCCESS);
	assert_memory_equal(securityInfo211.encryptionSessionKey, sessionEnc, 16);
	assert_memory_equal(securityInfo211.C_MACSessionKey, sessionCMac, 16);
	assert_memory_equal(securityInfo211.R_MACSessionKey, sessionRMac, 16);

	expect_memory(send_APDU, capdu, deleteRequest, sizeof(deleteRequest));
	will_return(send_APDU, deleteResponse);
	will_return(send_APDU, &deleteResponseLength);

	status = GP211_delete_application(cardContext, cardInfo, &securityInfo211, &appAid, 1, &receiptData, &receiptDataLength);
	assert_int_equal(status.errorCode, OPGP_ERROR_STATUS_SUCCESS);
}

// TODO: R-MAC does not work
static void send_apdu_rmac_rencryption(void **state) {
	OPGP_ERROR_STATUS status;

	BYTE hostChallenge[8];
	DWORD hostChallengeLen = 8;
	BYTE initializeUpdateRequest[APDU_COMMAND_LEN], extAuthRequest[APDU_COMMAND_LEN];

	DWORD initializeUpdateRequestLen, extAuthRequestLen;
	initializeUpdateRequestLen = extAuthRequestLen = APDU_COMMAND_LEN;

	BYTE initializeUpdateResponse[APDU_RESPONSE_LEN];
	DWORD initializeUpdateResponseLen = APDU_RESPONSE_LEN;

	BYTE extAuthResponse[] = {0x90, 0x00};
	DWORD extAuthResponseLen = sizeof(extAuthResponse);

	securityInfo211.invokingAidLength = 16;
	hex_to_byte_array("A00000003000020007", securityInfo211.invokingAid, &securityInfo211.invokingAidLength);

	OPGP_CSTRING commands[] = {
				"84400000B80422E602DD973125102DDE05875CA115C04D811A40593DB9D9480927CC9A249BD118FB6C674A86BF70D0CD898153A438DAC4CD302B74876D718481FCDB9D39944453BEBD28832300C99FB8035D2DFF3E3CB3F5DAD568330127929207A179226EB5C6C52341A297CB2C8F25E481F1CE6EF6EADD2410E70EB268AF48D29CB20CFCB8448DE83235ADA96A5EEED379C0701924707DA111DBB9039ACF8E19F1CE62AA4EF26D7C871E842ED815245F7F2D9A9A87D9BDA54B188DCB",
				"84400000B803C6D93C9A2EBE310E2215CEAC64C9CB8A03A89657272AE001E398FB7A7F42E02210672D0927DBE2BF6F80AA322D948C2743EFBA75EC51FD9BC5E6B4306371916CC50B0F341177FB99091F560A37C88D1A387443792E128A3A04118D0EA9F8EC989012FE4E097440411852373B4470851AC573A0B15262E506B34F771DCC53A24E91FAD37FE67454FAAFEFCC3E2F7141DCD7174D88933629E632C1CFDD5D1D30017D0CA14FE32A022F064C93EB4769F5A497BE036C5A563F",
				"84400000B8982E6E5F120796589037A751431F835CCF17C5A13CB4D7F5E0948CF48538FEDCD6F2F0A54F813E8BA755DFE22F7B847BF6A8BBDB1FEEC531B44D01B92BD55EEAA5B55B770AF293E4995E14E3076A40A89ED21A6C2F3772839E6E9A31FAB4B493AB31FE1D951AD11D9924DF15C1C33A5F3C90D168E2A78437ED40601B5289C56F2F020C78744C8C22141A2FCFFADB4B4FDE504EFABA6DD9D5EAD7E0F7D57B58F97C28DACD4C9E93052A0F16EFDF2A41DDFE373917C351B9B8"
	};
	OPGP_CSTRING responses[] = {
				"20F075017DE9F58A1E3D7567CBE71140AC0EBFA376881449A0665992BB997C6A145AC6BE3B642931C2652F62B85532B45689EB3E37602E6276B72BABE6A0FF00BEB02473EF22792A37F70D17F7715F103528CC6122880FC86AE75E5F085F1B37492FF33DD148162DAFA203D973F3A4579F8B547FACB14034641FC817BBDF119A9EB0F24DED4F9CE3640B46189EE2138B405446FA4F154D4F9600B288299642BEF5A387B71922A0CAD4D77D9596E8F417A6DB42CFE2C1081E9000",
				"EFA214CD7DE37F299690265ECDBF210361CBC87EAA227C7A5FC1C984A8DFFAF82B6C1856E57C399FA46AA13F9A7150F76207BCBF7421EFE6424D1FCFEF090F63CDF6C1F21F951E286D6A237843B4E1AD070D7FB16C883D052338F19D5C5E96135398F6251E5D48984EBE528E42A93CBD880D67D6631BCCEF13C3D54572C77CEAD84D26F72A4A66FA13DDCFA6C1BF8F5CF4AB151D722B3DBE86841F30BA15E4B90834B122257913FAA6F09D0E540E114817C1ED9F9D2F7AD39000",
				"0E625EA4492C2253DF239C271E0DEA22DDB136461742E6AD02FA9710588655A17B9C9C115076E00EF858C84662827AF5B90CE2374A575A7208E708BB3D01F655A996BF10F6077F61FBD71927E352215CE9348429FDAB03A3E6EEC5C71556A3C42B98240D224314638608CFDCFE5847BB2811948956F9204BC69085A034F33982E75BEBDEB71B59665FCEA9EDD27A85ABE908171D68E2FA639E2ABA02A27CD21E3A37EE831A1C1C1CF6ECD1B4C744EBD250EA42072B5300789000"
	};

	hex_to_byte_array("80500000082C8130E574247B1B00", initializeUpdateRequest, &initializeUpdateRequestLen);
	hex_to_byte_array("00010203040506070809000370A5874C57119B976BF94879E36F29E0390000019000", initializeUpdateResponse, &initializeUpdateResponseLen);
	hex_to_byte_array("8482330010A75F1CD48F3B93DFA57E690937921F92", extAuthRequest, &extAuthRequestLen);
	hex_to_byte_array("2C8130E574247B1B", hostChallenge, &hostChallengeLen);

	GP211_APPLICATION_DATA applicationData[10];
	GP211_EXECUTABLE_MODULES_DATA executablesData[10];
	DWORD dataLength = 10;

	will_return(__wrap_RAND_bytes, hostChallenge);
	expect_value(__wrap_RAND_bytes, num, 8);

	expect_memory(send_APDU, capdu, initializeUpdateRequest, initializeUpdateRequestLen);
	will_return(send_APDU, initializeUpdateResponse);
	will_return(send_APDU, &initializeUpdateResponseLen);

	expect_memory(send_APDU, capdu, extAuthRequest, extAuthRequestLen);
	will_return(send_APDU, extAuthResponse);
	will_return(send_APDU, &extAuthResponseLen);

	status = GP211_mutual_authentication(cardContext, cardInfo, NULL, (PBYTE)OPGP_VISA_DEFAULT_KEY, (PBYTE)OPGP_VISA_DEFAULT_KEY, (PBYTE)OPGP_VISA_DEFAULT_KEY, 0, 0,
			GP211_SCP03, GP211_SCP03_IMPL_i70, GP211_SCP03_SECURITY_LEVEL_C_DEC_R_ENC_C_MAC_R_MAC, 0, &securityInfo211);
	assert_int_equal(status.errorCode, OPGP_ERROR_STATUS_SUCCESS);

	// the macro looses one 9000 response ????
	//ENQUEUE_COMMANDS(commands, responses, 6)
	BYTE commandRequest[APDU_COMMAND_LEN];
	DWORD commandRequestLen = APDU_COMMAND_LEN;
	BYTE commandResponse[APDU_RESPONSE_LEN];
	DWORD commandResponseLen = APDU_RESPONSE_LEN;
	for (int i=0; i<3; i++) {
		commandRequestLen = APDU_COMMAND_LEN;
		commandResponseLen = APDU_RESPONSE_LEN;
		hex_to_byte_array(*(commands + i), commandRequest, &commandRequestLen);
		hex_to_byte_array(*(responses + i), commandResponse, &commandResponseLen);
		expect_memory(send_APDU, capdu, commandRequest, commandRequestLen);
		will_return(send_APDU, commandResponse);
		will_return(send_APDU, &commandResponseLen);
	}

	commandRequestLen = APDU_COMMAND_LEN;
	commandResponseLen = APDU_RESPONSE_LEN;
	hex_to_byte_array("80400000a548656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c64", commandRequest, &commandRequestLen);
	status = GP211_send_APDU(cardContext, cardInfo, &securityInfo211, commandRequest, commandRequestLen, commandResponse, &commandResponseLen);
	assert_int_equal(status.errorCode, OPGP_ERROR_STATUS_SUCCESS);

	commandRequestLen = APDU_COMMAND_LEN;
	commandResponseLen = APDU_RESPONSE_LEN;
	hex_to_byte_array("80400000a548656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c64", commandRequest, &commandRequestLen);
	status = GP211_send_APDU(cardContext, cardInfo, &securityInfo211, commandRequest, commandRequestLen, commandResponse, &commandResponseLen);
	assert_int_equal(status.errorCode, OPGP_ERROR_STATUS_SUCCESS);

	commandRequestLen = APDU_COMMAND_LEN;
	commandResponseLen = APDU_RESPONSE_LEN;
	hex_to_byte_array("80400000a548656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c64", commandRequest, &commandRequestLen);
	status = GP211_send_APDU(cardContext, cardInfo, &securityInfo211, commandRequest, commandRequestLen, commandResponse, &commandResponseLen);
	assert_int_equal(status.errorCode, OPGP_ERROR_STATUS_SUCCESS);
}

//TRACE (2020-05-14 17:46:51,738) (com.simless.iso7816.apduchannel:com.simless.iso7816.JCardSimAPDUChannel.singleTransmit(JCardSimAPDUChannel.java:119)) - --> :80500000082C8130E574247B1B00
//TRACE (2020-05-14 17:46:51,741) (com.simless.iso7816.apduchannel:com.simless.iso7816.JCardSimAPDUChannel.singleTransmit(JCardSimAPDUChannel.java:121)) - <-- :00010203040506070809000370A5874C57119B976BF94879E36F29E0390000019000
//TRACE (2020-05-14 17:46:51,751) (com.simless.globalplatform.scp.SCP03:com.simless.globalplatform.scp.SCP03.initializeUpdateResponse(SCP03.java:266)) - Card challenge: a5874c57119b976b
//TRACE (2020-05-14 17:46:51,751) (com.simless.globalplatform.scp.SCP03:com.simless.globalplatform.scp.SCP03.initializeUpdateResponse(SCP03.java:272)) - Card cryptogram: f94879e36f29e039
//TRACE (2020-05-14 17:46:51,751) (com.simless.iso7816.apduchannel:com.simless.iso7816.JCardSimAPDUChannel.singleTransmit(JCardSimAPDUChannel.java:119)) - --> :8482330010A75F1CD48F3B93DFA57E690937921F92
//TRACE (2020-05-14 17:46:51,752) (com.simless.iso7816.apduchannel:com.simless.iso7816.JCardSimAPDUChannel.singleTransmit(JCardSimAPDUChannel.java:121)) - <-- :9000
//TRACE (2020-05-14 17:46:51,752) (com.simless.globalplatform.scp.SCP03:com.simless.globalplatform.scp.SCP03.wrap(SCP03.java:375)) - Unwrapped command: 80400000a548656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c64
//TRACE (2020-05-14 17:46:51,754) (com.simless.iso7816.apduchannel:com.simless.iso7816.JCardSimAPDUChannel.singleTransmit(JCardSimAPDUChannel.java:119)) - --> :84400000B80422E602DD973125102DDE05875CA115C04D811A40593DB9D9480927CC9A249BD118FB6C674A86BF70D0CD898153A438DAC4CD302B74876D718481FCDB9D39944453BEBD28832300C99FB8035D2DFF3E3CB3F5DAD568330127929207A179226EB5C6C52341A297CB2C8F25E481F1CE6EF6EADD2410E70EB268AF48D29CB20CFCB8448DE83235ADA96A5EEED379C0701924707DA111DBB9039ACF8E19F1CE62AA4EF26D7C871E842ED815245F7F2D9A9A87D9BDA54B188DCB
//TRACE (2020-05-14 17:46:51,755) (com.simless.iso7816.apduchannel:com.simless.iso7816.JCardSimAPDUChannel.singleTransmit(JCardSimAPDUChannel.java:121)) - <-- :20F075017DE9F58A1E3D7567CBE71140AC0EBFA376881449A0665992BB997C6A145AC6BE3B642931C2652F62B85532B45689EB3E37602E6276B72BABE6A0FF00BEB02473EF22792A37F70D17F7715F103528CC6122880FC86AE75E5F085F1B37492FF33DD148162DAFA203D973F3A4579F8B547FACB14034641FC817BBDF119A9EB0F24DED4F9CE3640B46189EE2138B405446FA4F154D4F9600B288299642BEF5A387B71922A0CAD4D77D9596E8F417A6DB42CFE2C1081E9000
//TRACE (2020-05-14 17:46:51,756) (com.simless.globalplatform.scp.SCP03:com.simless.globalplatform.scp.SCP03.wrap(SCP03.java:375)) - Unwrapped command: 80400000a548656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c64
//TRACE (2020-05-14 17:46:51,756) (com.simless.iso7816.apduchannel:com.simless.iso7816.JCardSimAPDUChannel.singleTransmit(JCardSimAPDUChannel.java:119)) - --> :84400000B803C6D93C9A2EBE310E2215CEAC64C9CB8A03A89657272AE001E398FB7A7F42E02210672D0927DBE2BF6F80AA322D948C2743EFBA75EC51FD9BC5E6B4306371916CC50B0F341177FB99091F560A37C88D1A387443792E128A3A04118D0EA9F8EC989012FE4E097440411852373B4470851AC573A0B15262E506B34F771DCC53A24E91FAD37FE67454FAAFEFCC3E2F7141DCD7174D88933629E632C1CFDD5D1D30017D0CA14FE32A022F064C93EB4769F5A497BE036C5A563F
//TRACE (2020-05-14 17:46:51,757) (com.simless.iso7816.apduchannel:com.simless.iso7816.JCardSimAPDUChannel.singleTransmit(JCardSimAPDUChannel.java:121)) - <-- :EFA214CD7DE37F299690265ECDBF210361CBC87EAA227C7A5FC1C984A8DFFAF82B6C1856E57C399FA46AA13F9A7150F76207BCBF7421EFE6424D1FCFEF090F63CDF6C1F21F951E286D6A237843B4E1AD070D7FB16C883D052338F19D5C5E96135398F6251E5D48984EBE528E42A93CBD880D67D6631BCCEF13C3D54572C77CEAD84D26F72A4A66FA13DDCFA6C1BF8F5CF4AB151D722B3DBE86841F30BA15E4B90834B122257913FAA6F09D0E540E114817C1ED9F9D2F7AD39000
//TRACE (2020-05-14 17:46:51,758) (com.simless.globalplatform.scp.SCP03:com.simless.globalplatform.scp.SCP03.wrap(SCP03.java:375)) - Unwrapped command: 80400000a548656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c6448656c6c6f20576f726c64
//TRACE (2020-05-14 17:46:51,759) (com.simless.iso7816.apduchannel:com.simless.iso7816.JCardSimAPDUChannel.singleTransmit(JCardSimAPDUChannel.java:119)) - --> :84400000B8982E6E5F120796589037A751431F835CCF17C5A13CB4D7F5E0948CF48538FEDCD6F2F0A54F813E8BA755DFE22F7B847BF6A8BBDB1FEEC531B44D01B92BD55EEAA5B55B770AF293E4995E14E3076A40A89ED21A6C2F3772839E6E9A31FAB4B493AB31FE1D951AD11D9924DF15C1C33A5F3C90D168E2A78437ED40601B5289C56F2F020C78744C8C22141A2FCFFADB4B4FDE504EFABA6DD9D5EAD7E0F7D57B58F97C28DACD4C9E93052A0F16EFDF2A41DDFE373917C351B9B8
//TRACE (2020-05-14 17:46:51,759) (com.simless.iso7816.apduchannel:com.simless.iso7816.JCardSimAPDUChannel.singleTransmit(JCardSimAPDUChannel.java:121)) - <-- :0E625EA4492C2253DF239C271E0DEA22DDB136461742E6AD02FA9710588655A17B9C9C115076E00EF858C84662827AF5B90CE2374A575A7208E708BB3D01F655A996BF10F6077F61FBD71927E352215CE9348429FDAB03A3E6EEC5C71556A3C42B98240D224314638608CFDCFE5847BB2811948956F9204BC69085A034F33982E75BEBDEB71B59665FCEA9EDD27A85ABE908171D68E2FA639E2ABA02A27CD21E3A37EE831A1C1C1CF6ECD1B4C744EBD250EA42072B5300789000

static void mutual_auth(void **state) {
	OPGP_ERROR_STATUS status;

	BYTE hostChallenge[] = {0x9B, 0xD6, 0xBF, 0x87, 0x8F, 0xB8, 0xE9, 0x91};

	BYTE sMac[] = {0x58, 0x56, 0x33, 0x62, 0xEC, 0x5A, 0x45, 0x41, 0xAB, 0xCD, 0x32, 0xB3, 0x4B, 0x1E, 0xAE, 0x7D};
	BYTE sEnc[] = {0xF9, 0x95, 0xD0, 0xA0, 0x69, 0x33, 0x5C, 0x7D, 0xF4, 0x2E, 0x59, 0x03, 0x17, 0xFF, 0xEA, 0x6D};
	BYTE dek[] = {0x0A, 0x02, 0xA6, 0xD6, 0x87, 0x40, 0x6D, 0xCF, 0xA0, 0x9D, 0xC7, 0x0B, 0x3E, 0xDB, 0x7E, 0x38};

	BYTE sessionEnc[] = {0xD8, 0x3E, 0xE3, 0x8C, 0x99, 0x54, 0xC8, 0x07, 0x89, 0x87, 0xA5, 0xE9, 0xEE, 0x6A, 0xB1, 0x3C};
	BYTE sessionCMac[] = {0x6F, 0xF3, 0x77, 0x16, 0xE0, 0x41, 0x30, 0x65, 0xE8, 0xDF, 0xD0, 0x8B, 0xF1, 0xE9, 0xEC, 0x5E};
	BYTE sessionRMac[] = {0x02, 0x54, 0xC7, 0x86, 0xE5, 0x7A, 0xCA, 0x89, 0x82, 0x67, 0x0C, 0x1C, 0x1A, 0x05, 0xFF, 0x12};

	BYTE initializeUpdateRequest[] = {0x80, 0x50, 0x00, 0x00, 0x08, 0x9B, 0xD6, 0xBF, 0x87, 0x8F, 0xB8, 0xE9, 0x91, 0x00};
	BYTE initializeUpdateResponse[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x03, 0x70, 0x3C,
			0x80, 0xC2, 0xCC, 0x87, 0xEB, 0x3A, 0x35, 0xE4, 0xED, 0xBA, 0x35, 0xE6, 0x29, 0xC3, 0x36, 0x00, 0x00, 0x1E,
			0x90, 0x00};
	DWORD initializeUpdateResponseLength = sizeof(initializeUpdateResponse);

	BYTE extAuthRequest[] = {0x84, 0x82, 0x03, 0x00, 0x10, 0x23, 0xEB, 0xFE, 0xDC, 0x57, 0x9D, 0x22, 0xCD, 0xCD, 0xB6, 0xA2,
			0x5A, 0x5F, 0xF7, 0x89, 0x1F};
	BYTE extAuthResponse[] = {0x90, 0x00};
	DWORD extAuthResponseLength = sizeof(extAuthResponse);

	will_return(__wrap_RAND_bytes, hostChallenge);
	expect_value(__wrap_RAND_bytes, num, 8);

	expect_memory(send_APDU, capdu, initializeUpdateRequest, sizeof(initializeUpdateRequest));
	will_return(send_APDU, initializeUpdateResponse);
	will_return(send_APDU, &initializeUpdateResponseLength);

	expect_memory(send_APDU, capdu, extAuthRequest, sizeof(extAuthRequest));
	will_return(send_APDU, extAuthResponse);
	will_return(send_APDU, &extAuthResponseLength);

	memcpy(securityInfo211.invokingAid, (void *)GP231_ISD_AID, sizeof(GP231_ISD_AID));
	securityInfo211.invokingAidLength = sizeof(GP231_ISD_AID);

	status = GP211_mutual_authentication(cardContext, cardInfo, NULL, sEnc, sMac, dek, 0, 0,
			GP211_SCP03, GP211_SCP03_IMPL_i70, GP211_SCP03_SECURITY_LEVEL_C_DEC_C_MAC, 0, &securityInfo211);
	assert_int_equal(status.errorCode, OPGP_ERROR_STATUS_SUCCESS);
	assert_memory_equal(securityInfo211.encryptionSessionKey, sessionEnc, 16);
	assert_memory_equal(securityInfo211.C_MACSessionKey, sessionCMac, 16);
	assert_memory_equal(securityInfo211.R_MACSessionKey, sessionRMac, 16);
}

static void get_status(void **state) {
	OPGP_ERROR_STATUS status;

	BYTE hostChallenge[] = {0xA6, 0xD4, 0x0A, 0xEC, 0x55, 0xFE, 0x79, 0x86};

	BYTE sMac[] = {0x58, 0x56, 0x33, 0x62, 0xEC, 0x5A, 0x45, 0x41, 0xAB, 0xCD, 0x32, 0xB3, 0x4B, 0x1E, 0xAE, 0x7D};
	BYTE sEnc[] = {0xF9, 0x95, 0xD0, 0xA0, 0x69, 0x33, 0x5C, 0x7D, 0xF4, 0x2E, 0x59, 0x03, 0x17, 0xFF, 0xEA, 0x6D};
	BYTE dek[] = {0x0A, 0x02, 0xA6, 0xD6, 0x87, 0x40, 0x6D, 0xCF, 0xA0, 0x9D, 0xC7, 0x0B, 0x3E, 0xDB, 0x7E, 0x38};

	BYTE initializeUpdateRequest[] = {0x80, 0x50, 0x00, 0x00, 0x08, 0xA6, 0xD4, 0x0A, 0xEC, 0x55, 0xFE, 0x79, 0x86, 0x00};
	BYTE initializeUpdateResponse[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x03, 0x70, 0xB6,
			0xE3, 0x62, 0xD0, 0xF9, 0x0F, 0x81, 0x01, 0xC3, 0x4B, 0x7B, 0x21, 0xA7, 0x45, 0x53, 0x20, 0x00, 0x00, 0x25, 0x90, 0x00};
	DWORD initializeUpdateResponseLength = sizeof(initializeUpdateResponse);

	BYTE extAuthRequest[] = {0x84, 0x82, 0x03, 0x00, 0x10, 0x3D, 0x95, 0x8B, 0x36, 0xBB, 0x38, 0xAE, 0x58, 0x96, 0xA5,
			0x59, 0x34, 0x5F, 0x81, 0x47, 0x1A};
	BYTE extAuthResponse[] = {0x90, 0x00};
	DWORD extAuthResponseLength = sizeof(extAuthResponse);

	BYTE getStatusRequest[] = {0x84, 0xf2, 0x40, 0x02, 0x18, 0xd1, 0x8c, 0x9a, 0x7b, 0xb2, 0xb1, 0xb1, 0xc7, 0xed, 0x1b,
			0x24, 0xe1, 0xb1, 0x30, 0x7c, 0xb9, 0xd7, 0x1a, 0x66, 0xab, 0xc5, 0x5c, 0xbc, 0x3a, 0x00};
	BYTE getStatusResponse[] = {0xe3, 0x3e, 0x4f, 0x10, 0xa0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0xff, 0xff, 0xff, 0xff,
			0x89, 0x00, 0x00, 0x10, 0x00, 0x9f, 0x70, 0x01, 0x3f, 0xc5, 0x03, 0x80, 0xc0, 0x00, 0xc4, 0x07, 0xa0, 0x00,
			0x00, 0x01, 0x51, 0x53, 0x50, 0xce, 0x02, 0x01, 0x00, 0xcc, 0x10, 0xa0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0xff,
			0xff, 0xff, 0xff, 0x89, 0x00, 0x00, 0x01, 0x00, 0xea, 0x02, 0x80, 0x00, 0xe3, 0x39, 0x4f, 0x10, 0xa0, 0x00, 0x00,
			0x00, 0x87, 0x10, 0x02, 0xff, 0x01, 0xff, 0x41, 0x89, 0x00, 0x00, 0x01, 0x00, 0x9f, 0x70, 0x02, 0x07, 0x00, 0xc5,
			0x03, 0x00, 0x00, 0x00, 0xc4, 0x0c, 0xa0, 0x00, 0x00, 0x00, 0x95, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x00, 0x01, 0xce,
			0x02, 0x01, 0x00, 0xcc, 0x05, 0xa0, 0x00, 0x00, 0x01, 0x51, 0xea, 0x02, 0x80, 0x00, 0xe3, 0x30, 0x4f, 0x09, 0xa0,
			0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0x00, 0x01, 0x9f, 0x70, 0x02, 0x07, 0x00, 0xc5, 0x03, 0x00, 0x00, 0x00, 0xc4,
			0x07, 0xa0, 0x00, 0x00, 0x01, 0x51, 0x53, 0x50, 0xce, 0x02, 0x01, 0x00, 0xcc, 0x08, 0xa0, 0x00, 0x00, 0x01, 0x51,
			0x00, 0x00, 0x00, 0xea, 0x02, 0x80, 0x00, 0xe3, 0x30, 0x4f, 0x09, 0xa0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0x00,
			0x02, 0x9f, 0x70, 0x02, 0x07, 0x00, 0xc5, 0x03, 0x00, 0x00, 0x00, 0xc4, 0x07, 0xa0, 0x00, 0x00, 0x01, 0x51, 0x53,
			0x50, 0xce, 0x02, 0x01, 0x00, 0xcc, 0x08, 0xa0, 0x00, 0x00, 0x01, 0x51, 0x00, 0x00, 0x00, 0xea, 0x02, 0x80, 0x00, 0x90, 0x00};
	DWORD getStatusResponseLength = sizeof(getStatusResponse);
	GP211_APPLICATION_DATA applicationData[10];
	GP211_EXECUTABLE_MODULES_DATA executablesData[10];
	DWORD dataLength = 10;

	will_return(__wrap_RAND_bytes, hostChallenge);
	expect_value(__wrap_RAND_bytes, num, 8);

	expect_memory(send_APDU, capdu, initializeUpdateRequest, sizeof(initializeUpdateRequest));
	will_return(send_APDU, initializeUpdateResponse);
	will_return(send_APDU, &initializeUpdateResponseLength);

	expect_memory(send_APDU, capdu, extAuthRequest, sizeof(extAuthRequest));
	will_return(send_APDU, extAuthResponse);
	will_return(send_APDU, &extAuthResponseLength);

	memcpy(securityInfo211.invokingAid, (void *)GP231_ISD_AID, sizeof(GP231_ISD_AID));
	securityInfo211.invokingAidLength = sizeof(GP231_ISD_AID);

	status = GP211_mutual_authentication(cardContext, cardInfo, NULL, sEnc, sMac, dek, 0, 0,
			GP211_SCP03, GP211_SCP03_IMPL_i70, GP211_SCP03_SECURITY_LEVEL_C_DEC_C_MAC, 0, &securityInfo211);
	assert_int_equal(status.errorCode, OPGP_ERROR_STATUS_SUCCESS);

	// 80f24002024f0000
	expect_memory(send_APDU, capdu, getStatusRequest, sizeof(getStatusRequest));
	will_return(send_APDU, getStatusResponse);
	will_return(send_APDU, &getStatusResponseLength);

	status = GP211_get_status(cardContext, cardInfo, &securityInfo211, GP211_STATUS_APPLICATIONS, GP211_STATUS_FORMAT_NEW, applicationData,
			executablesData, &dataLength);
	assert_int_equal(status.errorCode, OPGP_ERROR_STATUS_SUCCESS);
	assert_int_equal(dataLength, 4);
	assert_int_equal(applicationData[3].aid.AIDLength, 9);
	assert_int_equal(applicationData[3].lifeCycleState, 7);
	assert_int_equal(applicationData[3].privileges, 0);
}

static int setup(void **state) {
	cardContext.connectionFunctions.sendAPDU = &send_APDU;
	return 0;
}

int main(void) {
	const struct CMUnitTest tests[] = {
			cmocka_unit_test(delete_application),
			cmocka_unit_test(mutual_auth),
			cmocka_unit_test(get_status)
			//cmocka_unit_test(send_apdu_rmac_rencryption)
	};
	return cmocka_run_group_tests_name("SCP03", tests, setup, NULL);
}
