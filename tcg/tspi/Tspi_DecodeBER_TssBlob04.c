/*
 *
 *   Copyright (C) International Business Machines  Corp., 2007
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
 *	Tspi_DecodeBER_TssBlob04.c
 *
 * DESCRIPTION
 *	This test will verify that Tspi_DecodeBER_TssBlob
 *		returns TSS_E_BAD_PARAMETER
 *
 * ALGORITHM
 *	Setup:
 *		Create Context
 *		Connect Context
 *		Create Key Object
 *		Encode Key Object
 *		Decode Key Object
 *
 *	Test:
 *		Call EncodeDER_TssBlob and DecodeBER_TssBlob with parameters that are not valid.
 *		Make sure that it returns the proper return codes
 *		Print results
 *
 *	Cleanup:
 *		Free memory related to hContext
 *		Close context
 *
 * USAGE
 *      First parameter is --options
 *                         -v or --version
 *      Second parameter is the version of the test case to be run
 *      This test case is currently only implemented for v1.2
 *
 * HISTORY
 *      Tom Lendacky, toml@us.ibm.com, 6/07.
 *
 * RESTRICTIONS
 *	None.
 */

#include <stdio.h>
#include "common.h"


int
main( int argc, char **argv )
{
	char		version;

	version = parseArgs( argc, argv );
	if (version >= TESTSUITE_TEST_TSS_1_2)
		main_v1_2(version);
	else
		print_NA();
}

int
main_v1_2( char version )
{
	char		*function = "Tspi_DecodeBER_TssBlob04";
	TSS_HCONTEXT	hContext;
	TSS_HKEY	hSRK;
	TSS_HPOLICY	hSrkPolicy;
	TSS_HKEY	hKey;
	BYTE*		rgbBlob;
	UINT32		ulBlobLength;
	BYTE*		rgbEncBlob = NULL;
	UINT32		ulEncBlobLength;
	UINT32		ulBlobType;
	BYTE*		rgbDecBlob = NULL;
	UINT32		ulDecBlobLength;
	TSS_RESULT	result;

	print_begin_test( function );

	result = connect_load_srk(&hContext, &hSRK);
	if ( result != TSS_SUCCESS )
	{
		print_error_exit( function, err_string(result) );
		Tspi_Context_FreeMemory( hContext, NULL );
		Tspi_Context_Close( hContext );
		exit( result );
	}

	result = create_key(hContext, TSS_KEY_TYPE_BIND, hSRK, &hKey);
	if ( result != TSS_SUCCESS )
	{
		print_error_exit( function, err_string(result) );
		Tspi_Context_FreeMemory( hContext, NULL );
		Tspi_Context_Close( hContext );
		exit( result );
	}

		//Call GetAttribData
	result = Tspi_GetAttribData(hKey,
			TSS_TSPATTRIB_KEY_BLOB,
			TSS_TSPATTRIB_KEYBLOB_BLOB,
			&ulBlobLength, &rgbBlob);
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_GetAttribData", result );
		print_error_exit( function, err_string(result) );
		Tspi_Context_FreeMemory( hContext, NULL );
		Tspi_Context_Close( hContext );
		exit( result );
	}

		// EncodeDER_TssBlob (32 bytes extra is enough for the encoding)
	ulEncBlobLength = ulBlobLength + 32;
	rgbEncBlob = calloc(1, ulEncBlobLength);
	result = Tspi_EncodeDER_TssBlob( ulBlobLength, rgbBlob, TSS_BLOB_TYPE_KEY, &ulEncBlobLength, rgbEncBlob );
	if ( result != TSS_SUCCESS )
	{
		print_error( "Tspi_EncodeDER_TssBlob", result );
		print_error_exit( function, err_string(result) );
		free(rgbEncBlob);
		Tspi_Context_FreeMemory( hContext, NULL );
		Tspi_Context_Close( hContext );
		exit( result );
	}

		// DecodeBER_TssBlob (pass in a buffer length but no buffer)
	ulDecBlobLength = 10;
	rgbDecBlob = NULL;
	result = Tspi_DecodeBER_TssBlob( ulEncBlobLength, rgbEncBlob, &ulBlobType, &ulDecBlobLength, rgbDecBlob );
	if ( TSS_ERROR_CODE(result) == TSS_E_BAD_PARAMETER ) {
		print_success( function, result );
	}
	else
	{
		if( !(checkNonAPI(result)) )
		{
			print_error( function, result );
		}
		else
		{
			print_error_nonapi( function, result );
		}

		print_error_exit( function, err_string(result) );
		free(rgbEncBlob);
		free(rgbDecBlob);
		Tspi_Context_FreeMemory( hContext, NULL );
		Tspi_Context_Close( hContext );
		exit( result );
	}

	print_end_test( function );
	free(rgbEncBlob);
	free(rgbDecBlob);
	Tspi_Context_FreeMemory( hContext, NULL );
	Tspi_Context_Close( hContext );
	exit( 0 );
}
