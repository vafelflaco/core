/*
 * $Id$
 */

/*
 * Harbour Project source code:
 * Tracing functions.
 *
 * Copyright 1999 Gonzalo Diethelm <gonzalo.diethelm@iname.com>
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hbapi.h"
#include "hbtrace.h"

char * hb_tr_file_ = "";
int    hb_tr_line_ = 0;
int    hb_tr_level_ = 0;

static int s_enabled = 1;
static FILE * s_fp = NULL;
static char * s_slevel[ HB_TR_LAST ] =
{
   "HB_TR_ALWAYS",
   "HB_TR_FATAL",
   "HB_TR_ERROR",
   "HB_TR_WARNING",
   "HB_TR_INFO",
   "HB_TR_DEBUG"
};


int hb_tracestate( int new_state )
{
   int old_state = s_enabled;

   if( new_state == 0 ||
       new_state == 1 )
      s_enabled = new_state;

   return old_state;
}

int hb_tracelevel( int new_level )
{
   int old_level = hb_tr_level_;

   if( new_level >= HB_TR_ALWAYS &&
       new_level <  HB_TR_LAST )
      hb_tr_level_ = new_level;

   return old_level;
}

int hb_tr_level( void )
{
   static int s_level = -1;

   if( s_level == -1 )
   {
      char * out;
      char * env;

      s_level = HB_TR_DEFAULT;

      out = getenv( "HB_TR_OUTPUT" );
      if( out != NULL && out[ 0 ] != '\0' )
      {
         s_fp = fopen( out, "w" );

         if( s_fp == NULL )
            s_fp = stderr;
      }
      else
         s_fp = stderr;

      env = getenv( "HB_TR_LEVEL" );
      if( env != NULL && env[ 0 ] != '\0' )
      {
         int i;

         for( i = 0; i < HB_TR_LAST; ++i )
         {
            /* 17/04/2000 - maurilio.longo@libero.it
               SET HB_TR_LEVEL=hb_tr_debug is valid under OS/2 and environment variable value returned is lower case */
            if( hb_stricmp( env, s_slevel[ i ] ) == 0 )
            {
               s_level = i;
               break;
            }
         }
      }
   }

   return s_level;
}

void hb_tr_trace( char * fmt, ... )
{
   /*
    * If tracing is disabled, do nothing.
    */
   if( s_enabled )
   {
      int i;
      va_list ap;

      /*
       * Clean up the file, so that instead of showing
       *
       *   ../../../foo/bar/baz.c
       *
       * we just show
       *
       *   foo/bar/baz.c
       */
      for( i = 0; hb_tr_file_[ i ] != '\0'; ++i )
      {
         if( hb_tr_file_[ i ] != '.' &&
             hb_tr_file_[ i ] != '/' &&
             hb_tr_file_[ i ] != '\\' )
            break;
      }

      /*
       * Print file and line.
       */
      fprintf( s_fp, "%s:%d: %s ",
               hb_tr_file_ + i, hb_tr_line_, s_slevel[ hb_tr_level_ ] );

      /*
       * Print the name and arguments for the function.
       */
      va_start( ap, fmt );
      vfprintf( s_fp, fmt, ap );
      va_end( ap );

      /*
       * Print a new-line.
       */
      fprintf( s_fp, "\n" );

      /*
       * Reset file and line.
       */
      hb_tr_level_ = -1;
      /* NOTE: resetting file name/line numer will cause that we will unable
       * to report the location of code that allocated unreleased memory blocks
       * See hb_xalloc/hb_xgrab in source/rtl/fm.c
       */
      if( hb_tr_level() < HB_TR_DEBUG )
      {
         hb_tr_file_ = "";
         hb_tr_line_ = -1;
      }
   }
}

