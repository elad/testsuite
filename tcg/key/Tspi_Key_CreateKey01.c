/*
 *
 *   Copyright (C) International Business Machines  Corp., 2004-2006
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	Tspi_Key_CreateKey01.c
 *
 * DESCRIPTION
 *	This test will verify Tspi_Key_CreateKey.
 *	The purpose of this test case is to get TSS_SUCCESS to be
 *		returned. This is easily accomplished by following
 *		the algorithm described below.
 *
 * ALGORITHM
 *	Setup:
 *		Create Context
 *		Connect Context
 *		Create Object
 *		Load SRK By UUID
 *		Get Policy Object
 *		Set Secret
 *
 *	Test:	Call CreateKey. If it is not a success
 *		Make sure that it returns the proper return codes
 *
 *	Cleanup:
 *		Close hKey object
 *		Close context
 *		Print error/success message
 *
 * USAGE:	First parameter is --options
 *			-v or --version
 *		Second Parameter is the version of the test case to be run.
 *		This test case is currently only implemented for 1.1 and 1.2
 *
 * HISTORY
 *	Author:	Kathy Robertson
 *	Date:	June 2004
 *	Email:	klrobert@us.ibm.com
 *	Kent Yoder, shpedoikal@gmail.com, 10/11/2004
 *	  Removed unneeded code
 *	EJR, ejratl@gmail.com, 8/10/2006, 1.2 updates
 *
 * RESTRICTIONS
 *	None.
 */

#include "common.h"



int main(int argc, char **argv)
{
	char version;

	version = parseArgs(argc, argv);
	if (version)
		main_v1_1();
	else
		print_wrongVersion();
}

main_v1_1(void)
{

	char *nameOfFunction = "Tspi_Key_CreateKey01";
	TSS_HCONTEXT hContext;
	TSS_HTPM hTPM;
	TSS_FLAG initFlags;
	TSS_HKEY hKey;
	TSS_HKEY hSRK;
	TSS_RESULT result;
	TSS_HPOLICY srkUsagePolicy, keyUsagePolicy;


	initFlags = TSS_KEY_TYPE_SIGNING | TSS_KEY_SIZE_2048 |
	    TSS_KEY_VOLATILE | TSS_KEY_AUTHORIZATION |
	    TSS_KEY_NOT_MIGRATABLE;

	print_begin_test(nameOfFunction);

	//Create Context
	result = Tspi_Context_Create(&hContext);
	if (result != TSS_SUCCESS) {
		print_error("Tspi_Context_Create", result);
		print_error_exit(nameOfFunction, err_string(result));
		exit(result);
	}
	//Connect Context
	result = Tspi_Context_Connect(hContext, get_server(GLOBALSERVER));
	if (result != TSS_SUCCESS) {
		print_error("Tspi_Context_Connect", result);
		print_error_exit(nameOfFunction, err_string(result));
		Tspi_Context_Close(hContext);
		exit(result);
	}
	//Create Object
	result =
	    Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_RSAKEY,
				      initFlags, &hKey);
	if (result != TSS_SUCCESS) {
		print_error("Tspi_Context_CreateObject", result);
		print_error_exit(nameOfFunction, err_string(result));
		Tspi_Context_Close(hContext);
		exit(result);
	}
	//Load Key By UUID
	result = Tspi_Context_LoadKeyByUUID(hContext,
					    TSS_PS_TYPE_SYSTEM, SRK_UUID,
					    &hSRK);
	if (result != TSS_SUCCESS) {
		print_error("Tspi_Context_LoadKeyByUUID", result);
		print_error_exit(nameOfFunction, err_string(result));
		Tspi_Context_CloseObject(hContext, hKey);
		Tspi_Context_Close(hContext);
		exit(result);
	}
#ifndef TESTSUITE_NOAUTH_SRK
	//Get Policy Object
	result =
	    Tspi_GetPolicyObject(hSRK, TSS_POLICY_USAGE, &srkUsagePolicy);
	if (result != TSS_SUCCESS) {
		print_error("Tspi_GetPolicyObject", result);
		print_error_exit(nameOfFunction, err_string(result));
		Tspi_Context_CloseObject(hContext, hKey);
		Tspi_Context_Close(hContext);
		exit(result);
	}
	//Set Secret
	result =
	    Tspi_Policy_SetSecret(srkUsagePolicy,
				  TESTSUITE_SRK_SECRET_MODE,
				  TESTSUITE_SRK_SECRET_LEN,
				  TESTSUITE_SRK_SECRET);
	if (result != TSS_SUCCESS) {
		print_error("Tspi_Policy_SetSecret", result);
		print_error_exit(nameOfFunction, err_string(result));
		Tspi_Context_CloseObject(hContext, hKey);
		Tspi_Context_Close(hContext);
		exit(result);
	}
#endif
	//Create Policy Object
	result =
	    Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_POLICY, TSS_POLICY_USAGE,
				      &keyUsagePolicy);
	if (result != TSS_SUCCESS) {
		print_error("Tspi_Context_CreateObject", result);
		print_error_exit(nameOfFunction, err_string(result));
		Tspi_Context_CloseObject(hContext, hKey);
		Tspi_Context_Close(hContext);
		exit(result);
	}
	//Set Secret
	result =
	    Tspi_Policy_SetSecret(keyUsagePolicy,
				  TESTSUITE_KEY_SECRET_MODE,
				  TESTSUITE_KEY_SECRET_LEN,
				  TESTSUITE_KEY_SECRET);
	if (result != TSS_SUCCESS) {
		print_error("Tspi_Policy_SetSecret", result);
		print_error_exit(nameOfFunction, err_string(result));
		Tspi_Context_CloseObject(hContext, hKey);
		Tspi_Context_Close(hContext);
		exit(result);
	}
	//Assign policy to key
	result = Tspi_Policy_AssignToObject(keyUsagePolicy, hKey);
	if (result != TSS_SUCCESS) {
		print_error("Tspi_Policy_AssignToObject", result);
		print_error_exit(nameOfFunction, err_string(result));
		Tspi_Context_Close(hContext);
		exit(result);
	}
	//Create Key
	result = Tspi_Key_CreateKey(hKey, hSRK, 0);
	if (result != TSS_SUCCESS) {
		if (!checkNonAPI(result)) {
			print_error(nameOfFunction, result);
			print_end_test(nameOfFunction);
			Tspi_Context_CloseObject(hContext, hKey);
			Tspi_Context_Close(hContext);
			exit(result);
		} else {
			print_error_nonapi(nameOfFunction, result);
			print_end_test(nameOfFunction);
			Tspi_Context_CloseObject(hContext, hKey);
			Tspi_Context_Close(hContext);
			exit(result);
		}
	} else {
		print_success(nameOfFunction, result);
		print_end_test(nameOfFunction);
		Tspi_Context_CloseObject(hContext, hKey);
		Tspi_Context_Close(hContext);
		exit(0);
	}
}
