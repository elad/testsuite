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
 *	Tspi_Context_RegisterKey02.c
 *
 * DESCRIPTION
 *	This test will return TSS_E_INVALID_HANDLE, because
 *		whContext (-1) is passed in as the first parameter of
 *		Context_RegisterKey.
 *
 * ALGORITHM
 *	Setup:
 *              Create Context
 *              Connect Context
 *              Load SRK by UUID
 *              Get Policy Object
 *              Set Secret
 *              Create Object (signing key)
 *              Create Key (signing key)
 *
 *	Test:
 *		Call Context_RegisterKey then if it does not succeed
 *		Make sure that it returns the proper return codes
 *		Print results
 *
 *	Cleanup:
 *		Unregister system key
 *		Free memory related to hContext
 *		Close context
 *
 * USAGE
 *      First parameter is --options
 *                         -v or --version
 *      Second parameter is the version of the test case to be run
 *      This test case is currently only implemented for v1.1 and 1.2
 *
 * HISTORY
 *	Megan Schneider, mschnei@us.ibm.com, 6/04.
 *
 * RESTRICTIONS
 *	None.
 */

#include <stdio.h>
#include <stdlib.h>

#include <trousers/tss.h>
#include "../common/common.h"


int
main( int argc, char **argv )
{
	char		*version;

	version = parseArgs(argc, argv);
		// if it is not version 1.1 or 1.2, print error
	if ((0 == strcmp(version, "1.1")) || (0 == strcmp(version, "1.2")))
		main_v1_1();
	else
		print_wrongVersion();
}

int
main_v1_1( void )
{
	char		*function = "Tspi_Context_RegisterKey02";
	TSS_HKEY	hSRK;
	TSS_UUID	migratableSignUUID =	{1,2,3,4,5,{6,7,8,9,10,2}};
	TSS_HKEY	hMSigningKey;
	TSS_HCONTEXT	hContext;
	TSS_HCONTEXT	whContext = -1;
	TSS_RESULT	result;
	TSS_HPOLICY	srkUsagePolicy;
	UINT32		exitCode = 0;

	print_begin_test( function );

		// Create Context
	result = Tspi_Context_Create( &hContext );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Context_Create", result );
		print_error_exit( function, err_string(result) );
		exit( result );
	}

		// Connect to Context
	result = Tspi_Context_Connect( hContext, get_server(GLOBALSERVER) );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Context_Connect", result );
		print_error_exit( function, err_string(result) );
		Tspi_Context_FreeMemory( hContext, NULL );
		Tspi_Context_Close( hContext );
		exit( result );
	}

		// Load SRK
	result = Tspi_Context_LoadKeyByUUID( hContext, TSS_PS_TYPE_SYSTEM,
						SRK_UUID, &hSRK );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Context_LoadKeyByUUID (hSRK)", result );
		print_error_exit( function, err_string(result) );
		Tspi_Context_FreeMemory( hContext, NULL );
		Tspi_Context_Close( hContext );
		exit( result );
	}

		// Get SRK Usage Policy
	result = Tspi_GetPolicyObject( hSRK, TSS_POLICY_USAGE,
					&srkUsagePolicy );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_GetPolicyObject", result );
		print_error_exit( function, err_string(result) );
		Tspi_Context_FreeMemory( hContext, NULL );
		Tspi_Context_Close( hContext );
		exit( result );
	}

		// set secret
	result = Tspi_Policy_SetSecret( srkUsagePolicy, TESTSUITE_SRK_SECRET_MODE,
					TESTSUITE_SRK_SECRET_LEN, TESTSUITE_SRK_SECRET );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Policy_SetSecret", result );
		print_error_exit( function, err_string(result) );
		Tspi_Context_FreeMemory( hContext, NULL );
		Tspi_Context_Close( hContext );
		exit( result );
	}

		// create object
	result = Tspi_Context_CreateObject( hContext, TSS_OBJECT_TYPE_RSAKEY,
						TSS_KEY_SIZE_2048 |
						TSS_KEY_TYPE_SIGNING |
						TSS_KEY_MIGRATABLE,
						&hMSigningKey );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Context_CreateObject (signing key)",
				result );
		print_error_exit( function, err_string(result) );
		Tspi_Context_FreeMemory( hContext, NULL );
		Tspi_Context_Close( hContext );
		exit( result );
	}

		// create signing key
	result = Tspi_Key_CreateKey( hMSigningKey, hSRK, 0 );
	if ( (result != TSS_SUCCESS) )
	{
		print_error( "Tspi_Key_CreateKey (signing key)", result );
		print_error_exit( function, err_string(result) );
		Tspi_Context_FreeMemory( hContext, NULL );
		Tspi_Context_Close( hContext );
		exit( result );
	}

		// register key
	result = Tspi_Context_RegisterKey( whContext, hMSigningKey,
						TSS_PS_TYPE_SYSTEM,
						migratableSignUUID,
						TSS_PS_TYPE_SYSTEM,
						SRK_UUID );
	if ( (TSS_ERROR_CODE(result) != TSS_E_INVALID_HANDLE) &&
		(TSS_ERROR_CODE(result) != TSS_E_KEY_ALREADY_REGISTERED) )
	{
		if( !(checkNonAPI(result)) )
		{
			print_error( function, result );
			exitCode = result;
		}
		else
		{
			print_error_nonapi( function, result );
			exitCode = result;
		}
	}
	else
	{
		print_success( function, result );
	}

	print_end_test( function );
	Tspi_Context_UnregisterKey( hContext, TSS_PS_TYPE_SYSTEM,
					migratableSignUUID,
					&hMSigningKey );
	Tspi_Context_FreeMemory( hContext, NULL );
	Tspi_Context_Close( hContext );
	exit( exitCode );
}
