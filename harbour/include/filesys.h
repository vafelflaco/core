/*
 * $Id$
 */

/*
   Harbour Project source code

   This module contains the Harbour declarations for FILE management.

   Copyright 1999 David G. Holm <dholm@jsd-llc.com>
   www - http://www.harbour-project.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version, with one exception:

   The exception is that if you link the Harbour Runtime Library (HRL)
   and/or the Harbour Virtual Machine (HVM) with other files to produce
   an executable, this does not by itself cause the resulting executable
   to be covered by the GNU General Public License. Your use of that
   executable is in no way restricted on account of linking the HRL
   and/or HVM code into it.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA (or visit
   their web site at http://www.gnu.org/).

   V 1.7    Victor Szel                 #include <x> changed to #include "x".
   V 1.6    David G. Holm               Added my email address.
   V 1.5    David G. Holm               Added copyright and license header,
                                        along with a complete version history.
   V 1.4    Victor Szel                 Undocumented change.
   V 1.3    Gonzalo A. Diethelm         Ensured that all Harbour functions
                                        are declared as HB_FUNCTION( void );
   V 1.2    David G. Holm               Corrected RCS Id keyword.
   V 1.1    David G. Holm               Committed to CVS.
   V 1.0    David G. Holm               Initial version.
*/

#ifndef HB_FILESYS_H_
#define HB_FILESYS_H_

#include "extend.h"
#include "fileio.ch"

typedef int    FHANDLE;

/* Filename support */
typedef struct
{
  char   szBuffer[ _POSIX_PATH_MAX + 3 ];
  char * szPath;
  char * szName;
  char * szExtension;
} HB_FNAME, * PHB_FNAME, * HB_FNAME_PTR;

extern BOOL     hb_fsChDir      ( BYTE * fpDirName );
extern USHORT   hb_fsChDrv      ( BYTE * nDrive );
extern void     hb_fsClose      ( FHANDLE hFileHandle );
extern void     hb_fsCommit     ( FHANDLE hFileHandle );
extern FHANDLE  hb_fsCreate     ( BYTE * fpFilename, USHORT uiAttribute );
extern BYTE *   hb_fsCurDir     ( USHORT uiDrive );
extern BYTE     hb_fsCurDrv     ( void );
extern void     hb_fsDelete     ( BYTE * fpFilename );
extern USHORT   hb_fsError      ( void );
extern FHANDLE  hb_fsExtOpen    ( BYTE * fpFilename, BYTE * fpDefExt,
                                  USHORT uiFlags, BYTE * fpPaths, PHB_ITEM pError );
extern USHORT   hb_fsIsDrv      ( BYTE nDrive );
extern BOOL     hb_fsLock       ( FHANDLE hFileHandle, ULONG ulStart,
                                  ULONG ulLength, USHORT uiMode );
extern BOOL     hb_fsMkDir      ( BYTE * fpDirName );
extern FHANDLE  hb_fsOpen       ( BYTE * fpFilename, USHORT uiFlags );
extern USHORT   hb_fsRead       ( FHANDLE hFileHandle, BYTE * fpBuff, USHORT uiCount );
extern BOOL     hb_fsRmDir      ( BYTE * fpDirName );
extern void     hb_fsRename     ( BYTE * fpOldName, BYTE * fpNewName );
extern ULONG    hb_fsSeek       ( FHANDLE hFileHandle, LONG lOffset, USHORT uiMode );
extern USHORT   hb_fsWrite      ( FHANDLE hFileHandle, BYTE * fpBuff, USHORT uiCount );

extern PHB_FNAME hb_fsFNameSplit ( char * szFilename ); /* Split given filename into path, name and extension */
extern char *    hb_fsFNameMerge ( char * szFileName, PHB_FNAME pFileName ); /* This function joins path, name and extension into a string with a filename */

#endif /* HB_FILESYS_H_ */
