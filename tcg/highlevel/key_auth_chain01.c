/*
 *
 *   Copyright (C) International Business Machines  Corp., 2004, 2005
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
 *
 * DESCRIPTION
 *	This test creates 3 keys in a hierarchy, the middle of which requires auth.
 *	This is meant to test the TSS_LOADKEY_INFO functionality. The TCS should
 *	try to load key 2 first and notice that its parent needs loading. When it
 *	tries to load key 1, it should see that there's auth data there and return
 *	a TSS_LOADKEY_INFO struct to the TSP. The TSP should transparently handle
 *	this structure to get...
 *
 *	SRK
 *	 \
 *	  hKey0 (no auth)
 *	   \
 *	    hKey1 (auth)
 *	     \
 *	      hKey2 (no auth)
 *
 * ALGORITHM
 *	Setup:
 *		Create Context
 *		Connect Context
 *		(the following code repeats for the keys)
 *		Create Key Object
 *		Get SRK Handle
 *		Get Policy Object
 *		Set Secret
 *		Create Key Object
 *		Create Key
 *		Register Key
 *		(end of repeating code)
 *
 *      Creates RSA keys with the following attributes:
 *      KEY     initFlags
 *      0       TSS_KEY_TYPE_STORAGE | TSS_KEY_NO_AUTHORIZATION
 *      1       TSS_KEY_TYPE_STORAGE | TSS_KEY_AUTHORIZATION
 *      2       TSS_KEY_TYPE_STORAGE | TSS_KEY_NO_AUTHORIZATION
 *
 *	Test:
 *
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *
 * USAGE
 *      First parameter is --options
 *                         -v or --version
 *      Second parameter is the version of the test case to be run
 *      This test case is currently only implemented for v1.1
 *
 * HISTORY
 *
 * RESTRICTIONS
 *	None.
 */

#include <stdio.h>
#include <trousers/tss.h>

#include "../common/common.h"

TSS_UUID uuid_0 = { 9, 9, 9, 9, 9, { 8, 8, 8, 8, 8, 8 } };
TSS_UUID uuid_1 = { 8, 8, 8, 8, 8, { 7, 7, 7, 7, 7, 7 } };
TSS_UUID uuid_2 = { 7, 7, 7, 7, 7, { 6, 6, 6, 6, 6, 6 } };


int
main( int argc, char **argv )
{
	char		*version;

	version = parseArgs( argc, argv );
		// if it is not version 1.1, print error
	if( strcmp(version, "1.1") )
		print_wrongVersion();
	else
		main_v1_1();
}

int
main_v1_1( void )
{
	char		*function = "key_auth_chain01";
	TSS_HCONTEXT    hContext;
	TSS_HKEY        hSRK;
	TSS_HKEY	hKey0, hKey1, hKey2;
	TSS_RESULT	result;
	TSS_HPOLICY	srkUsagePolicy, keyUsagePolicy;
	int temp;

	print_begin_test( function );

	srand(time(0));

		// Create Context
	result = Tspi_Context_Create( &hContext );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Context_Create", result );
		print_error_exit( function, err_string(result) );
		exit( result );
	}

		// Connect to Context
	result = Tspi_Context_Connect( hContext, NULL );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Context_Connect", result );
		goto done;
	}

#if 1
	Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
	Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_1, &hKey1);
	Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_2, &hKey2);
#endif
		//Load Key By UUID
	result = Tspi_Context_LoadKeyByUUID( hContext, TSS_PS_TYPE_SYSTEM,
						SRK_UUID, &hSRK );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Context_LoadKeyByUUID (hSRK)", result );
		goto done;
	}

		//Get Policy Object
	result = Tspi_GetPolicyObject( hSRK, TSS_POLICY_USAGE,
					&srkUsagePolicy );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_GetPolicyObject", result );
		goto done;
	}

		//Set Secret
	result = Tspi_Policy_SetSecret( srkUsagePolicy, TESTSUITE_SRK_SECRET_MODE,
				TESTSUITE_SRK_SECRET_LEN, TESTSUITE_SRK_SECRET );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Policy_SetSecret", result );
		goto done;
	}

	/* ######## Start Key 0 ######## */
	result = Tspi_Context_CreateObject( hContext, TSS_OBJECT_TYPE_RSAKEY,
						TSS_KEY_SIZE_2048 |
						TSS_KEY_TYPE_STORAGE |
						TSS_KEY_NO_AUTHORIZATION,
						&hKey0 );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Context_CreateObject (key 0)", result );
		goto done;
	}

	result = Tspi_Key_CreateKey( hKey0, hSRK, 0 );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Key_CreateKey (key 0)", result );
		goto done;
	}

	result = Tspi_Context_RegisterKey( hContext, hKey0,
						TSS_PS_TYPE_SYSTEM,
						uuid_0,
						TSS_PS_TYPE_SYSTEM,
						SRK_UUID );
	if ( (result != TSS_SUCCESS) )
	{
		print_error( "Tspi_Context_RegisterKey (uuid_0)", result );
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
		goto done;
	}
	fprintf( stderr, "\t\tKey 0 Finished\n" );
	/* ######## End Key 0 ######## */
	/* ######## Start Key 1 ######## */
	result = Tspi_Context_CreateObject( hContext,
						TSS_OBJECT_TYPE_RSAKEY,
						TSS_KEY_TYPE_STORAGE |
						TSS_KEY_AUTHORIZATION,
						&hKey1 );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Context_CreateObject (key 1)", result );
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
		goto done;
	}

	result = Tspi_Context_LoadKeyByUUID( hContext, TSS_PS_TYPE_SYSTEM,
                                                uuid_0, &hKey0 );
        if ( result != TSS_SUCCESS )
        {
                print_error( "Tspi_Context_LoadKeyByUUID (hKey0)", result );
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
		goto done;
        }

	result = Tspi_GetPolicyObject( hKey1, TSS_POLICY_USAGE,
					&keyUsagePolicy );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_GetPolicyObject", result );
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
		goto done;
	}

		//Set Secret
	result = Tspi_Policy_SetSecret( keyUsagePolicy, TESTSUITE_KEY_SECRET_MODE,
					TESTSUITE_KEY_SECRET_LEN, TESTSUITE_KEY_SECRET );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Policy_SetSecret", result );
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
		goto done;
	}

	result = Tspi_Key_CreateKey( hKey1, hKey0, 0 );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Key_CreateKey (key 1)", result );
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
		goto done;
	}

	result = Tspi_Context_RegisterKey( hContext, hKey1,
						TSS_PS_TYPE_SYSTEM,
						uuid_1,
						TSS_PS_TYPE_SYSTEM,
						uuid_0 );
	if ( (result != TSS_SUCCESS) )
	{
		print_error( "Tspi_Context_RegisterKey (uuid_1)", result );
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
		goto done;
	}
	fprintf( stderr, "\t\tKey 1 Finished\n" );
	/* ######## End Key 1 ######## */

	/* ######## Start Key 2 ######## */
	result = Tspi_Context_CreateObject( hContext, TSS_OBJECT_TYPE_RSAKEY,
						TSS_KEY_TYPE_BIND |
						TSS_KEY_NO_AUTHORIZATION,
						&hKey2 );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Context_CreateObject (key 2)", result );
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_1, &hKey1);
		goto done;
	}

	result = Tspi_Key_LoadKey( hKey1, hKey0 );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Context_LoadKeyByUUID (hKey1)", result );
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_1, &hKey1);
		goto done;
	}

	result = Tspi_Key_CreateKey( hKey2, hKey1, 0 );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_Key_CreateKey (key 2)", result );
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_1, &hKey1);
		goto done;
	}

	result = Tspi_Context_RegisterKey( hContext, hKey2,
						TSS_PS_TYPE_SYSTEM,
						uuid_2,
						TSS_PS_TYPE_SYSTEM,
						uuid_1 );
	if ( (result != TSS_SUCCESS) )
	{
		print_error( "Tspi_Context_RegisterKey (uuid_2)", result );
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_0, &hKey0);
		Tspi_Context_UnregisterKey(hContext, TSS_PS_TYPE_SYSTEM, uuid_1, &hKey1);
		goto done;
	}
	fprintf( stderr, "\t\tKey 2 Finished\n" );
	/* ######## End Key 2 ######## */

	print_success( function, result );
	print_end_test( function );
	Tspi_Context_FreeMemory( hContext, NULL );
	Tspi_Context_Close( hContext );
	exit( 0 );

done:
	print_error_exit( function, err_string(result) );
	Tspi_Context_FreeMemory( hContext, NULL );
	Tspi_Context_Close( hContext );
	exit( result );
}
