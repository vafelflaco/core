/*
 * $Id$
 */

/*
 * Harbour Project source code:
 * Preprocessor runtime library callable version
 *
 * Copyright 1999 Felipe G. Coury
 * www - http://www.harbour-project.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version, with one exception:
 *
 * The exception is that if you link the Harbour Runtime Library (HRL)
 * and/or the Harbour Virtual Machine (HVM) with other files to produce
 * an executable, this does not by itself cause the resulting executable
 * to be covered by the GNU General Public License. Your use of that
 * executable is in no way restricted on account of linking the HRL
 * and/or HVM code into it.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA (or visit
 * their web site at http://www.gnu.org/).
 *
 */

#include <stdio.h>
#include <setjmp.h>

#include "hbpp.h"
#include "extend.h"
#include "itemapi.h"
#include "hberrors.h"

PATHNAMES * _pIncludePath = NULL;
PHB_FNAME _pFileName = NULL;

jmp_buf s_env;

/* TODO: Extend the function to allow directives
         and external include files              */

/* TODO: This function should return an error code. The preprocessed sting
 * should be returned  by a reference.
 */
HARBOUR HB___PREPROCESS( void )
{
  if( ISCHAR( 1 ) )
    {
      char * pText = ( char * ) hb_xgrab( STR_SIZE );
      char * pOut = ( char * ) hb_xgrab( STR_SIZE );
      char * ptr = pText;

      int slen;

      slen = HB_MIN_( hb_parclen( 1 ), STR_SIZE - 1 );
      memcpy( pText, hb_parc( 1 ), slen );
      pText[ slen ] = 0; /* Preprocessor expects null-terminated string */
      memset( pOut, 0, STR_SIZE );

      SKIPTABSPACES( ptr );

      if( setjmp( s_env ) == 0 )
        {
          int resParse;

          if( ( resParse = ParseExpression( ptr, pOut ) ) > 0 )
            {
              /* Some error here? */
            }
          hb_retc( pText ); /* Preprocessor returns parsed line in input buffer */
        }
      else
        {
          /* an error occured during parsing.
           * The longjmp was used in GenError()
           */
          hb_retc( "ERROR" );
        }

      hb_xfree( pText );
      hb_xfree( pOut );
    }
  else
    hb_retc( "" );
}

void GenError( char * _szErrors[], char cPrefix, int iError, char * szError1, char * szError2 )
{
  HB_TRACE(("GenError(%p, %c, %d, %s, %s)",
            _szErrors, cPrefix, iError, szError1, szError2));

  /* TODO: The internal buffers allocated by the preprocessor should be
   * deallocated here
   */
  printf( "Error %c%i  ", cPrefix, iError );
  printf( _szErrors[ iError - 1 ], szError1, szError2 );
  printf( hb_consoleGetNewLine() );
  printf( hb_consoleGetNewLine() );

  longjmp( s_env, iError );
}

void GenWarning( char* _szWarnings[], char cPrefix, int iWarning, char * szWarning1, char * szWarning2)
{
  HB_TRACE(("GenWarning(%p, %c, %d, %s, %s)",
            _szWarnings, cPrefix, iWarning, szWarning1, szWarning2));

  /* NOTE:
   *    All warnings are simply ignored
   */
  HB_SYMBOL_UNUSED( _szWarnings );
  HB_SYMBOL_UNUSED( cPrefix );
  HB_SYMBOL_UNUSED( iWarning );
  HB_SYMBOL_UNUSED( szWarning1 );
  HB_SYMBOL_UNUSED( szWarning2 );
}
