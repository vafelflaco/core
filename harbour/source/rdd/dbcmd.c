/*
 * $Id$
 */

/*
 * Harbour Project source code:
 * Base RDD module
 *
 * Copyright 1999 Bruno Cantero <bruno@issnet.net>
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

/*
 * The following parts are Copyright of the individual authors.
 * www - http://www.harbour-project.org
 *
 * Copyright 1999 Luiz Rafael Culik <culik@sl.conex.net>
 *    DB*() documentation
 *    ORD*() documentation
 *    RDD*() documentation
 * See doc/license.txt for licensing terms.
 *
 */

#include <ctype.h>
#include "extend.h"
#include "itemapi.h"
#include "errorapi.h"
#include "rddapi.h"
#include "set.h"
#include "ctoharb.h"
#include "rddsys.ch"
#include "set.ch"
#include "langapi.h"

#define HARBOUR_MAX_RDD_DRIVERNAME_LENGTH       32
#define HARBOUR_MAX_RDD_ALIAS_LENGTH            32
#define HARBOUR_MAX_RDD_FIELDNAME_LENGTH        32

typedef struct _RDDNODE
{
   char     szName[ HARBOUR_MAX_RDD_DRIVERNAME_LENGTH + 1 ];
   USHORT   uiType;            /* Type of RDD */
   RDDFUNCS pTable;            /* Table of functions */
   USHORT   uiAreaSize;        /* Size of the WorkArea */
   struct _RDDNODE * pNext;    /* Next RDD in the list */
} RDDNODE;

typedef RDDNODE * LPRDDNODE;

typedef struct _AREANODE
{
   void *             pArea;   /* WorkAreas with different sizes */
   struct _AREANODE * pPrev;   /* Prev WorkArea in the list */
   struct _AREANODE * pNext;   /* Next WorkArea in the list */
} AREANODE;

typedef AREANODE * LPAREANODE;

extern HARBOUR HB__DBF( void );
extern HARBOUR HB__SDF( void );
extern HARBOUR HB__DELIM( void );
extern HARBOUR HB_RDDSYS( void );

HARBOUR HB_AFIELDS( void );
HARBOUR HB_ALIAS( void );
HARBOUR HB_BOF( void );
HARBOUR HB_DBAPPEND( void );
HARBOUR HB_DBCLEARFILTER( void );
HARBOUR HB_DBCLOSEALL( void );
HARBOUR HB_DBCLOSEAREA( void );
HARBOUR HB_DBCOMMIT( void );
HARBOUR HB_DBCOMMITALL( void );
HARBOUR HB___DBCONTINUE( void );
HARBOUR HB_DBCREATE( void );
HARBOUR HB_DBDELETE( void );
HARBOUR HB_DBEVAL( void );
HARBOUR HB_DBF( void );
HARBOUR HB_DBFILTER( void );
HARBOUR HB_DBGOBOTTOM( void );
HARBOUR HB_DBGOTO( void );
HARBOUR HB_DBGOTOP( void );
HARBOUR HB___DBLOCATE( void );
HARBOUR HB___DBPACK( void );
HARBOUR HB_DBRECALL( void );
HARBOUR HB_DBRLOCK( void );
HARBOUR HB_DBRLOCKLIST( void );
HARBOUR HB_DBRUNLOCK( void );
HARBOUR HB_DBSEEK( void );
HARBOUR HB_DBSELECTAREA( void );
HARBOUR HB_DBSETDRIVER( void );
HARBOUR HB_DBSETFILTER( void );
HARBOUR HB___DBSETFOUND( void );
HARBOUR HB___DBSETLOCATE( void );
HARBOUR HB_DBSKIP( void );
HARBOUR HB_DBSTRUCT( void );
HARBOUR HB_DBTABLEEXT( void );
HARBOUR HB_DBUNLOCK( void );
HARBOUR HB_DBUNLOCKALL( void );
HARBOUR HB_DBUSEAREA( void );
HARBOUR HB___DBZAP( void );
HARBOUR HB_DELETED( void );
HARBOUR HB_EOF( void );
HARBOUR HB_FCOUNT( void );
HARBOUR HB_FIELDGET( void );
HARBOUR HB_FIELDNAME( void );
HARBOUR HB_FIELDPOS( void );
HARBOUR HB_FIELDPUT( void );
HARBOUR HB_FLOCK( void );
HARBOUR HB_FOUND( void );
HARBOUR HB_HEADER( void );
HARBOUR HB_INDEXORD( void );
HARBOUR HB_LASTREC( void );
HARBOUR HB_LOCK( void );
HARBOUR HB_LUPDATE( void );
HARBOUR HB_NETERR( void );
HARBOUR HB_ORDBAGEXT( void );
HARBOUR HB_ORDBAGNAME( void );
HARBOUR HB_ORDCONDSET( void );
HARBOUR HB_ORDCREATE( void );
HARBOUR HB_ORDDESTROY( void );
HARBOUR HB_ORDFOR( void );
HARBOUR HB_ORDKEY( void );
HARBOUR HB_ORDLISTADD( void );
HARBOUR HB_ORDLISTCLEAR( void );
HARBOUR HB_ORDLISTREBUILD( void );
HARBOUR HB_ORDNAME( void );
HARBOUR HB_ORDNUMBER( void );
HARBOUR HB_ORDSETFOCUS( void );
HARBOUR HB_RDDLIST( void );
HARBOUR HB_RDDNAME( void );
HARBOUR HB_RDDREGISTER( void );
HARBOUR HB_RDDSETDEFAULT( void );
HARBOUR HB_RECCOUNT( void );
HARBOUR HB_RECNO( void );
HARBOUR HB_RECSIZE( void );
HARBOUR HB_RLOCK( void );
HARBOUR HB_SELECT( void );
HARBOUR HB_USED( void );
HARBOUR HB___RDDSETDEFAULT( void );

static char * szDefDriver = NULL;    /* Default RDD name */
static USHORT uiCurrArea = 1;        /* Selectd area */
static LPRDDNODE pRddList = NULL;    /* Registered RDD's */
static BOOL bNetError = FALSE;       /* Error on Networked environments */

static LPAREANODE pWorkAreas = NULL; /* WorkAreas */
static LPAREANODE pCurrArea = NULL;  /* Pointer to a selectd and valid area */

/*
 * -- BASIC RDD METHODS --
 */

static ERRCODE defAddField( AREAP pArea, LPDBFIELDINFO pFieldInfo )
{
   LPFIELD pField;
   ULONG ulLen;

   HB_TRACE(HB_TR_DEBUG, ("defAddField(%p, %p)", pArea, pFieldInfo));

   /* Validate the name of field */
   ulLen = strlen( ( char * ) pFieldInfo->atomName );
   hb_strLTrim( ( char * ) pFieldInfo->atomName, &ulLen );
   if( !ulLen )
      return FAILURE;

   pField = pArea->lpFields + pArea->uiFieldCount;
   if( pArea->uiFieldCount > 0 )
   {
      ( ( LPFIELD ) ( pField - 1 ) )->lpfNext = pField;
      if( ( ( LPFIELD ) ( pField - 1 ) )->uiType == 'C' )
         pField->uiOffset = ( ( LPFIELD ) ( pField - 1 ) )->uiOffset +
                            ( ( LPFIELD ) ( pField - 1 ) )->uiLen +
                            ( ( USHORT ) ( ( LPFIELD ) ( pField - 1 ) )->uiDec << 8 );
      else
         pField->uiOffset = ( ( LPFIELD ) ( pField - 1 ) )->uiOffset +
                            ( ( LPFIELD ) ( pField - 1 ) )->uiLen;
   }
   else
      pField->uiOffset = 1;
   pField->sym = ( void * ) hb_dynsymGet( ( char * ) pFieldInfo->atomName );
   pField->uiType = pFieldInfo->uiType;
   pField->uiTypeExtended = pFieldInfo->typeExtended;
   pField->uiLen = pFieldInfo->uiLen;
   pField->uiDec = pFieldInfo->uiDec;
   pField->uiArea = pArea->uiArea;
   pArea->uiFieldCount++;
   return SUCCESS;
}

static ERRCODE defAlias( AREAP pArea, BYTE * szAlias )
{
   HB_TRACE(HB_TR_DEBUG, ("defAlias(%p, %p)", pArea, szAlias));

   strncpy( ( char * ) szAlias,
            ( ( PHB_DYNS ) pArea->atomAlias )->pSymbol->szName,
            HARBOUR_MAX_RDD_ALIAS_LENGTH );
   return SUCCESS;
}

static ERRCODE defBof( AREAP pArea, BOOL * pBof )
{
   HB_TRACE(HB_TR_DEBUG, ("defBof(%p, %p)", pArea, pBof));

   * pBof = pArea->fBof;
   return SUCCESS;
}

static ERRCODE defClearFilter( AREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("defClearFilter(%p)", pArea));

   if( pArea->dbfi.fFilter )
   {
      hb_itemRelease( pArea->dbfi.itmCobExpr );
      hb_itemRelease( pArea->dbfi.abFilterText );
      pArea->dbfi.fFilter = FALSE;
   }
   return SUCCESS;
}

static ERRCODE defClearLocate( AREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("defClearLocate(%p)", pArea));

   if( pArea->dbsi.itmCobFor )
   {
      hb_itemRelease( pArea->dbsi.itmCobFor );
      pArea->dbsi.itmCobFor = NULL;
   }
   if( pArea->dbsi.lpstrFor )
   {
      hb_itemRelease( pArea->dbsi.lpstrFor );
      pArea->dbsi.lpstrFor = NULL;
   }
   if( pArea->dbsi.itmCobWhile )
   {
      hb_itemRelease( pArea->dbsi.itmCobWhile );
      pArea->dbsi.itmCobWhile = NULL;
   }
   if( pArea->dbsi.lpstrWhile )
   {
      hb_itemRelease( pArea->dbsi.lpstrWhile );
      pArea->dbsi.lpstrWhile = NULL;
   }
   if( pArea->dbsi.lNext )
   {
      hb_itemRelease( pArea->dbsi.lNext );
      pArea->dbsi.lNext = NULL;
   }
   if( pArea->dbsi.itmRecID )
   {
      hb_itemRelease( pArea->dbsi.itmRecID );
      pArea->dbsi.itmRecID = NULL;
   }
   if( pArea->dbsi.fRest )
   {
      hb_itemRelease( pArea->dbsi.fRest );
      pArea->dbsi.fRest = NULL;
   }
   return SUCCESS;
}

static ERRCODE defClose( AREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("defClose(%p)", pArea));

   SELF_CLEARFILTER( pArea );
   SELF_CLEARLOCATE( pArea );
   ( ( PHB_DYNS ) pArea->atomAlias )->hArea = 0;
   return SUCCESS;
}

static ERRCODE defCompile( AREAP pArea, BYTE * szExpr )
{
   HB_MACRO_PTR pMacro;

   HB_TRACE(HB_TR_DEBUG, ("defCompile(%p, %p)", pArea, szExpr));

   pMacro = hb_macroCompile( ( char * ) szExpr );
   if( pMacro )
   {
      pArea->valResult = hb_itemPutPtr( pArea->valResult, ( void * ) pMacro );
      return SUCCESS;
   }
   else
      return FAILURE;
}

static ERRCODE defCreateFields( AREAP pArea, PHB_ITEM pStruct )
{
   USHORT uiCount, uiItems;
   PHB_ITEM pFieldDesc;
   DBFIELDINFO pFieldInfo;
   long lLong;

   HB_TRACE(HB_TR_DEBUG, ("defCreateFields(%p, %p)", pArea, pStruct));

   uiItems = hb_arrayLen( pStruct );
   SELF_SETFIELDEXTENT( pArea, uiItems );
   pFieldInfo.typeExtended = 0;
   for( uiCount = 0; uiCount < uiItems; uiCount++ )
   {
      pFieldDesc = hb_arrayGetItemPtr( pStruct, uiCount + 1 );
      pFieldInfo.uiType = toupper( hb_arrayGetCPtr( pFieldDesc, 2 )[ 0 ] );
      pFieldInfo.atomName = ( BYTE * ) hb_arrayGetCPtr( pFieldDesc, 1 );
      lLong = hb_arrayGetNL( pFieldDesc, 3 );
      if( lLong < 0 )
         lLong = 0;
      pFieldInfo.uiLen = ( USHORT ) lLong;
      lLong = hb_arrayGetNL( pFieldDesc, 4 );
      if( lLong < 0 )
         lLong = 0;
      pFieldInfo.uiDec = ( USHORT ) lLong;
      if( SELF_ADDFIELD( pArea, &pFieldInfo ) == FAILURE )
         return FAILURE;
   }
   return SUCCESS;
}

static ERRCODE defEof( AREAP pArea, BOOL * pEof )
{
   HB_TRACE(HB_TR_DEBUG, ("defEof(%p, %p)", pArea, pEof));

   * pEof = pArea->fEof;
   return SUCCESS;
}

static ERRCODE defError( AREAP pArea, PHB_ITEM pError )
{
   char * szRddName;

   HB_TRACE(HB_TR_DEBUG, ("defError(%p, %p)", pArea, pError));

   szRddName = ( char * ) hb_xgrab( HARBOUR_MAX_RDD_DRIVERNAME_LENGTH + 1 );
   SELF_SYSNAME( pArea, ( BYTE * ) szRddName );
   hb_errPutSeverity( pError, ES_ERROR );
   hb_errPutSubSystem( pError, szRddName );
   hb_xfree( szRddName );
   return hb_errLaunch( pError );
}

static ERRCODE defEval( AREAP pArea, LPDBEVALINFO pEvalInfo )
{
   BOOL bEof, bFor, bWhile;
   ULONG ulNext;

   HB_TRACE(HB_TR_DEBUG, ("defEval(%p, %p)", pArea, pEvalInfo));

   if( pEvalInfo->dbsci.itmRecID )
   {
      SELF_GOTOID( pArea, pEvalInfo->dbsci.itmRecID );
      SELF_EOF( pArea, &bEof );
      if( !bEof )
      {
         if( pEvalInfo->dbsci.itmCobWhile )
         {
            hb_vmPushSymbol( &hb_symEval );
            hb_vmPush( pEvalInfo->dbsci.itmCobWhile );
            hb_vmDo( 0 );
            bWhile = hb_itemGetL( &hb_stack.Return );
         }
         else
            bWhile = TRUE;

         if( pEvalInfo->dbsci.itmCobFor )
         {
            hb_vmPushSymbol( &hb_symEval );
            hb_vmPush( pEvalInfo->dbsci.itmCobFor );
            hb_vmDo( 0 );
            bFor = hb_itemGetL( &hb_stack.Return );
         }
         else
            bFor = TRUE;

         if( bWhile && bFor )
         {
            hb_vmPushSymbol( &hb_symEval );
            hb_vmPush( pEvalInfo->itmBlock );
            hb_vmDo( 0 );
         }
      }
      return SUCCESS;
   }

   if( !pEvalInfo->dbsci.fRest || !hb_itemGetL( pEvalInfo->dbsci.fRest ) )
      SELF_GOTOP( pArea );

   if( pEvalInfo->dbsci.lNext )
      ulNext = hb_itemGetNL( pEvalInfo->dbsci.lNext );

   SELF_EOF( pArea, &bEof );
   while( !bEof )
   {
      if( pEvalInfo->dbsci.lNext && ulNext-- < 1 )
         break;

      if( pEvalInfo->dbsci.itmCobWhile )
      {
         hb_vmPushSymbol( &hb_symEval );
         hb_vmPush( pEvalInfo->dbsci.itmCobWhile );
         hb_vmDo( 0 );
         bWhile = hb_itemGetL( &hb_stack.Return );
         if( !bWhile )
            break;
      }
      else
         bWhile = TRUE;

      if( pEvalInfo->dbsci.itmCobFor )
      {
         hb_vmPushSymbol( &hb_symEval );
         hb_vmPush( pEvalInfo->dbsci.itmCobFor );
         hb_vmDo( 0 );
         bFor = hb_itemGetL( &hb_stack.Return );
      }
      else
         bFor = TRUE;

      if( bFor && bWhile )
      {
         hb_vmPushSymbol( &hb_symEval );
         hb_vmPush( pEvalInfo->itmBlock );
         hb_vmDo( 0 );
      }
      SELF_SKIP( pArea, 1 );
      SELF_EOF( pArea, &bEof );
   }

   return SUCCESS;
}

static ERRCODE defEvalBlock( AREAP pArea, PHB_ITEM pBlock )
{
   PHB_ITEM pError;

   HB_TRACE(HB_TR_DEBUG, ("defEvalBlock(%p, %p)", pArea, pBlock));

   if( !pBlock && !IS_BLOCK( pBlock ) )
   {
      pError = hb_errNew();
      hb_errPutGenCode( pError, EG_NOMETHOD );
      hb_errPutDescription( pError, hb_langDGetErrorDesc( EG_NOMETHOD ) );
      SELF_ERROR( pArea, pError );
      hb_errRelease( pError );
      return FAILURE;
   }

   hb_vmPushSymbol( &hb_symEval );
   hb_vmPush( pBlock );
   hb_vmDo( 0 );
   if( !pArea->valResult )
      pArea->valResult = hb_itemNew( NULL );
   hb_itemCopy( pArea->valResult, &hb_stack.Return );

   return SUCCESS;
}

static ERRCODE defFieldCount( AREAP pArea, USHORT * uiFields )
{
   HB_TRACE(HB_TR_DEBUG, ("defFieldCount(%p, %p)", pArea, uiFields));

   * uiFields = pArea->uiFieldCount;
   return SUCCESS;
}

static ERRCODE defFieldInfo( AREAP pArea, USHORT uiIndex, USHORT uiType, PHB_ITEM pItem )
{
   LPFIELD pField;
   char szType[ 2 ];

   HB_TRACE(HB_TR_DEBUG, ("defFieldInfo(%p, %hu, %hu, %p)", pArea, uiIndex, uiType, pItem));

   if( uiIndex > pArea->uiFieldCount )
      return FAILURE;

   pField = pArea->lpFields + uiIndex - 1;
   switch( uiType )
   {
      case DBS_NAME:
         hb_itemPutC( pItem, ( ( PHB_DYNS ) pField->sym )->pSymbol->szName );
         break;

      case DBS_TYPE:
         szType[ 0 ] = pField->uiType;
         szType[ 1 ] = '\0';
         hb_itemPutC( pItem, szType );
         break;

      case DBS_LEN:
         hb_itemPutNL( pItem, pField->uiLen );
         break;

      case DBS_DEC:
         hb_itemPutNL( pItem, pField->uiDec );
         break;

      default:
         return FAILURE;

   }
   return SUCCESS;
}


static ERRCODE defFieldName( AREAP pArea, USHORT uiIndex, void * szName )
{
   LPFIELD pField;

   HB_TRACE(HB_TR_DEBUG, ("defFieldName(%p, %hu, %p)", pArea, uiIndex, szName));

   if( uiIndex > pArea->uiFieldCount )
      return FAILURE;

   pField = pArea->lpFields + uiIndex - 1;
   strncpy( ( char * ) szName, ( ( PHB_DYNS ) pField->sym )->pSymbol->szName,
            HARBOUR_MAX_RDD_FIELDNAME_LENGTH );
   return SUCCESS;
}

static ERRCODE defFilterText( AREAP pArea, PHB_ITEM pFilter )
{
   HB_TRACE(HB_TR_DEBUG, ("defFilterText(%p, %p)", pArea, pFilter));

   if( pArea->dbfi.fFilter )
      hb_itemCopy( pFilter, pArea->dbfi.abFilterText );
   return SUCCESS;
}

static ERRCODE defFound( AREAP pArea, BOOL * pFound )
{
   HB_TRACE(HB_TR_DEBUG, ("defFound(%p, %p)", pArea, pFound));

   * pFound = pArea->fFound;
   return SUCCESS;
}

static ERRCODE defNewArea( AREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("defNewArea(%p)", pArea));

   pArea->lpDataInfo = ( LPFILEINFO ) hb_xgrab( sizeof( FILEINFO ) );
   memset( pArea->lpDataInfo, 0, sizeof( FILEINFO ) );
   pArea->lpDataInfo->hFile = FS_ERROR;
   pArea->lpExtendInfo = ( LPDBEXTENDINFO ) hb_xgrab( sizeof( DBEXTENDINFO ) );
   memset( pArea->lpExtendInfo, 0, sizeof( DBEXTENDINFO ) );
   return SUCCESS;
}

static ERRCODE defOpen( AREAP pArea, LPDBOPENINFO pOpenInfo )
{
   HB_TRACE(HB_TR_DEBUG, ("defOpen(%p, %p)", pArea, pOpenInfo));

   pArea->atomAlias = hb_dynsymGet( ( char * ) pOpenInfo->atomAlias );
   if( ( ( PHB_DYNS ) pArea->atomAlias )->hArea )
   {
      hb_errRT_DBCMD( EG_DUPALIAS, 1011, NULL, ( char * ) pOpenInfo->atomAlias );
      return FAILURE;
   }

   ( ( PHB_DYNS ) pArea->atomAlias )->hArea = pOpenInfo->uiArea;
   pArea->lpExtendInfo->fExclusive = !pOpenInfo->fShared;
   pArea->lpExtendInfo->fReadOnly = pOpenInfo->fReadonly;

   return SUCCESS;
}

static ERRCODE defOrderCondition( AREAP pArea, LPDBORDERCONDINFO pOrderInfo )
{
   if( pArea->lpdbOrdCondInfo )
   {
      if( pArea->lpdbOrdCondInfo->abFor )
         hb_xfree( pArea->lpdbOrdCondInfo->abFor );
      if( pArea->lpdbOrdCondInfo->itmCobFor )
         hb_itemRelease( pArea->lpdbOrdCondInfo->itmCobFor );
      if( pArea->lpdbOrdCondInfo->itmCobWhile )
         hb_itemRelease( pArea->lpdbOrdCondInfo->itmCobWhile );
      if( pArea->lpdbOrdCondInfo->itmCobEval )
         hb_itemRelease( pArea->lpdbOrdCondInfo->itmCobEval );
      hb_xfree( pArea->lpdbOrdCondInfo );
   }
   pArea->lpdbOrdCondInfo = pOrderInfo;

   return SUCCESS;
}

static ERRCODE defRelease( AREAP pArea )
{
   LPFILEINFO pFileInfo;

   HB_TRACE(HB_TR_DEBUG, ("defRelease(%p)", pArea));

   SELF_ORDSETCOND( pArea, NULL );

   if( pArea->valResult )
      hb_itemRelease( pArea->valResult );

   if( pArea->lpFields )
   {
      hb_xfree( pArea->lpFields );
      pArea->uiFieldCount = 0;
   }

   while( pArea->lpDataInfo )
   {
      pFileInfo = pArea->lpDataInfo;
      pArea->lpDataInfo = pArea->lpDataInfo->pNext;
      if( pFileInfo->szFileName )
         hb_xfree( pFileInfo->szFileName );
      hb_xfree( pFileInfo );
   }

   if( pArea->lpExtendInfo )
   {
      if( pArea->lpExtendInfo->bRecord )
         hb_xfree( pArea->lpExtendInfo->bRecord );
      hb_xfree( pArea->lpExtendInfo );
   }

   return SUCCESS;
}

static ERRCODE defSetFieldExtent( AREAP pArea, USHORT uiFieldExtent )
{
   HB_TRACE(HB_TR_DEBUG, ("defSetFieldExtent(%p, %hu)", pArea, uiFieldExtent));

   pArea->uiFieldExtent = uiFieldExtent;
   pArea->lpFields = ( LPFIELD ) hb_xgrab( uiFieldExtent * sizeof( FIELD ) );
   memset( pArea->lpFields, 0, uiFieldExtent * sizeof( FIELD ) );
   return SUCCESS;
}

static ERRCODE defSetFilter( AREAP pArea, LPDBFILTERINFO pFilterInfo )
{
   HB_TRACE(HB_TR_DEBUG, ("defSetFilter(%p, %p)", pArea, pFilterInfo));

   if( pArea->dbfi.fFilter )
   {
      hb_itemCopy( pArea->dbfi.itmCobExpr, pFilterInfo->itmCobExpr );
      hb_itemCopy( pArea->dbfi.abFilterText, pFilterInfo->abFilterText );
   }
   else
   {
      pArea->dbfi.itmCobExpr = hb_itemNew( NULL );
      hb_itemCopy( pArea->dbfi.itmCobExpr, pFilterInfo->itmCobExpr );
      pArea->dbfi.abFilterText = hb_itemNew( NULL );
      hb_itemCopy( pArea->dbfi.abFilterText, pFilterInfo->abFilterText );
      pArea->dbfi.fFilter = TRUE;
   }
   return SUCCESS;
}

static ERRCODE defSetLocate( AREAP pArea, LPDBSCOPEINFO pScopeInfo )
{
   HB_TRACE(HB_TR_DEBUG, ("defSetLocate(%p, %p)", pArea, pScopeInfo));

   if( pArea->dbsi.itmCobFor )
      hb_itemRelease( pArea->dbsi.itmCobFor );
   pArea->dbsi.itmCobFor = pScopeInfo->itmCobFor;
   if( pArea->dbsi.itmCobWhile )
      hb_itemRelease( pArea->dbsi.itmCobWhile );
   pArea->dbsi.itmCobWhile = pScopeInfo->itmCobWhile;
   if( pArea->dbsi.lNext )
      hb_itemRelease( pArea->dbsi.lNext );
   pArea->dbsi.lNext = pScopeInfo->lNext;
   if( pArea->dbsi.itmRecID )
      hb_itemRelease( pArea->dbsi.itmRecID );
   pArea->dbsi.itmRecID = pScopeInfo->itmRecID;
   if( pArea->dbsi.fRest )
      hb_itemRelease( pArea->dbsi.fRest );
   pArea->dbsi.fRest = pScopeInfo->fRest;
   return SUCCESS;
}

static ERRCODE defSkip( AREAP pArea, LONG lToSkip )
{
   BOOL bExit;

   HB_TRACE(HB_TR_DEBUG, ("defSkip(%p, %ld)", pArea, lToSkip));

   if( pArea->dbfi.fFilter || hb_set.HB_SET_DELETED )
   {
      if( lToSkip > 0 )
      {
         while( lToSkip > 0 )
         {
            SELF_SKIPRAW( pArea, 1 );
            SELF_SKIPFILTER( pArea, 1 );

            SELF_EOF( pArea, &bExit );
            if( bExit )
               return SUCCESS;

            lToSkip--;
         }
      }
      else if( lToSkip < 0 )
      {
         while( lToSkip < 0 )
         {
            SELF_SKIPRAW( pArea, -1 );
            SELF_SKIPFILTER( pArea, -1 );

            SELF_BOF( pArea, &bExit );
            if( bExit )
               return SELF_SKIPFILTER( pArea, 1 );

            lToSkip++;
         }
      }
      else
      {
         SELF_SKIPRAW( pArea, 0 );
         SELF_SKIPFILTER( pArea, 1 );

         SELF_EOF( pArea, &bExit );
         if( bExit )
            return SUCCESS;
      }
   }

   return SELF_SKIPRAW( pArea, lToSkip );
}

static ERRCODE defSkipFilter( AREAP pArea, LONG lUpDown )
{
   BOOL bExit, bDeleted;

   HB_TRACE(HB_TR_DEBUG, ("defSkipFilter(%p, %ld)", pArea, lUpDown));

   if( lUpDown > 0 )
   {
      while( 1 )
      {
         SELF_EOF( pArea, &bExit );
         if( bExit )
            return SUCCESS;

         /* SET DELETED */
         if( hb_set.HB_SET_DELETED )
         {
            SELF_DELETED( pArea, &bDeleted );
            if( bDeleted )
            {
               SELF_SKIPRAW( pArea, 1 );
               continue;
            }
         }

         /* SET FILTER TO */
         if( pArea->dbfi.fFilter )
         {
            hb_vmPushSymbol( &hb_symEval );
            hb_vmPush( pArea->dbfi.itmCobExpr );
            hb_vmDo( 0 );
            if( IS_LOGICAL( &hb_stack.Return ) &&
                !hb_stack.Return.item.asLogical.value )
            {
               SELF_SKIPRAW( pArea, 1 );
               continue;
            }
         }
         return SUCCESS;
      }
   }
   else if( lUpDown < 0 )
   {
      while( 1 )
      {
         SELF_BOF( pArea, &bExit );
         if( bExit )
            return SELF_SKIPFILTER( pArea, 1 );

         /* SET DELETED */
         if( hb_set.HB_SET_DELETED )
         {
            SELF_DELETED( pArea, &bDeleted );
            if( bDeleted )
            {
               SELF_SKIPRAW( pArea, -1 );
               continue;
            }
         }

         /* SET FILTER TO */
         if( pArea->dbfi.fFilter )
         {
            hb_vmPushSymbol( &hb_symEval );
            hb_vmPush( pArea->dbfi.itmCobExpr );
            hb_vmDo( 0 );
            if( IS_LOGICAL( &hb_stack.Return ) &&
                !hb_stack.Return.item.asLogical.value )
            {
               SELF_SKIPRAW( pArea, 1 );
               continue;
            }
         }
         return SUCCESS;
      }
   }
   return SUCCESS;
}

static ERRCODE defSkipRaw( AREAP pArea, LONG lToSkip )
{
   HB_TRACE(HB_TR_DEBUG, ("defSkipRaw(%p, %ld)", pArea, lToSkip));

   return SELF_GOTO( pArea, pArea->lpExtendInfo->ulRecNo + lToSkip );
}

static ERRCODE defStructSize( AREAP pArea, USHORT * uiSize )
{
   HB_TRACE(HB_TR_DEBUG, ("defStrucSize(%p, %p)", pArea, uiSize));

   HB_SYMBOL_UNUSED( pArea );
   HB_SYMBOL_UNUSED( uiSize );

   return SUCCESS;
}

static ERRCODE defSysName( AREAP pArea, BYTE * pBuffer )
{
   USHORT uiCount;
   LPRDDNODE pRddNode;

   HB_TRACE(HB_TR_DEBUG, ("defSysName(%p, %p)", pArea, pBuffer));

   pRddNode = pRddList;
   for( uiCount = 0; uiCount < pArea->rddID; uiCount++ )
      pRddNode = pRddNode->pNext;
   strncpy( ( char * ) pBuffer, pRddNode->szName, HARBOUR_MAX_RDD_DRIVERNAME_LENGTH );
   return SUCCESS;
}

static ERRCODE defUnSupported( AREAP pArea )
{
   PHB_ITEM pError;

   HB_TRACE(HB_TR_DEBUG, ("defUnSupported(%p)", pArea));

   HB_SYMBOL_UNUSED( pArea );

   pError = hb_errNew();
   hb_errPutGenCode( pError, EG_UNSUPPORTED );
   hb_errPutDescription( pError, hb_langDGetErrorDesc( EG_UNSUPPORTED ) );
   SELF_ERROR( pArea, pError );
   hb_errRelease( pError );
   return FAILURE;
}

static RDDFUNCS defTable = { defBof,
                             defEof,
                             defFound,
                             defUnSupported,
                             ( DBENTRYP_UL ) defUnSupported,
                             ( DBENTRYP_I ) defUnSupported,
                             defUnSupported,
                             ( DBENTRYP_BIB ) defUnSupported,
                             defSkip,
                             defSkipFilter,
                             defSkipRaw,
                             defAddField,
                             ( DBENTRYP_B ) defUnSupported,
                             defCreateFields,
                             defUnSupported,
                             ( DBENTRYP_BP ) defUnSupported,
                             defFieldCount,
                             ( DBENTRYP_VF ) defUnSupported,
                             defFieldInfo,
                             defFieldName,
                             defUnSupported,
                             ( DBENTRYP_PP ) defUnSupported,
                             ( DBENTRYP_SI ) defUnSupported,
                             ( DBENTRYP_SVL ) defUnSupported,
                             defUnSupported,
                             defUnSupported,
                             ( DBENTRYP_P ) defUnSupported,
                             ( DBENTRYP_SI ) defUnSupported,
                             defUnSupported,
                             ( DBENTRYP_ULP ) defUnSupported,
                             ( DBENTRYP_ISI ) defUnSupported,
                             ( DBENTRYP_I ) defUnSupported,
                             defSetFieldExtent,
                             defAlias,
                             defClose,
                             ( DBENTRYP_VP ) defUnSupported,
                             ( DBENTRYP_SI ) defUnSupported,
                             defNewArea,
                             defOpen,
                             defRelease,
                             defStructSize,
                             defSysName,
                             defEval,
                             defUnSupported,
                             defUnSupported,
                             ( DBENTRYP_OI ) defUnSupported,
                             defUnSupported,
                             ( DBENTRYP_OI ) defUnSupported,
                             defUnSupported,
                             defOrderCondition,
                             ( DBENTRYP_VOC ) defUnSupported,
                             ( DBENTRYP_OI ) defUnSupported,
                             ( DBENTRYP_OII ) defUnSupported,
                             defClearFilter,
                             defClearLocate,
                             defFilterText,
                             defSetFilter,
                             defSetLocate,
                             defCompile,
                             defError,
                             defEvalBlock,
                             ( DBENTRYP_VSP ) defUnSupported,
                             ( DBENTRYP_VL ) defUnSupported,
                             ( DBENTRYP_UL ) defUnSupported,
                             defUnSupported,
                             ( DBENTRYP_VP ) defUnSupported,
                             ( DBENTRYP_SVP ) defUnSupported,
                             ( DBENTRYP_VP ) defUnSupported,
                             ( DBENTRYP_SVP ) defUnSupported,
                             defUnSupported,
                             defUnSupported,
                             ( DBENTRYP_SVP ) defUnSupported
                           };


static void hb_rddCheck( void )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_rddCheck()"));

   if( !szDefDriver )
   {
      szDefDriver = ( char * ) hb_xgrab( 1 );
      szDefDriver[ 0 ] = '\0';

      /* Force link the built-in RDD's */
      HB__DBF();
      HB__SDF();
      HB__DELIM();
      HB_RDDSYS();
   }
}

static void hb_rddCloseAll( void )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_rddCloseAll()"));

   pCurrArea = pWorkAreas;
   while( pWorkAreas )
   {
      pCurrArea = pWorkAreas;
      pWorkAreas = pWorkAreas->pNext;
      SELF_CLOSE( ( AREAP ) pCurrArea->pArea );
      SELF_RELEASE( ( AREAP ) pCurrArea->pArea );
      hb_xfree( pCurrArea->pArea );
      hb_xfree( pCurrArea );
   }
   uiCurrArea = 1;
   pCurrArea = NULL;
   pWorkAreas = NULL;
}

static LPRDDNODE hb_rddFindNode( char * szDriver, USHORT * uiIndex )
{
   LPRDDNODE pRddNode;
   USHORT uiCount;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddFindNode(%s, %p)", szDriver, uiIndex));

   uiCount = 0;
   pRddNode = pRddList;
   while( pRddNode )
   {
      if( strcmp( pRddNode->szName, szDriver ) == 0 ) /* Matched RDD */
      {
         if( uiIndex )
            * uiIndex = uiCount;
         return pRddNode;
      }
      pRddNode = pRddNode->pNext;
      uiCount++;
   }
   if( uiIndex )
      * uiIndex = 0;
   return NULL;
}

static int hb_rddRegister( char * szDriver, USHORT uiType )
{
   LPRDDNODE pRddNode, pRddNewNode;
   PHB_DYNS pGetFuncTable;
   char * szGetFuncTable;
   USHORT uiFunctions;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddRegister(%s, %hu)", szDriver, uiType));

   if( hb_rddFindNode( szDriver, 0 ) )    /* Duplicated RDD */
      return 1;

   szGetFuncTable = ( char * ) hb_xgrab( strlen( szDriver ) + 14 );
   strcpy( szGetFuncTable, szDriver );
   strcat( szGetFuncTable, "_GETFUNCTABLE" );
   pGetFuncTable = hb_dynsymFindName( szGetFuncTable );
   hb_xfree( szGetFuncTable );
   if( !pGetFuncTable )
      return 2;              /* Not valid RDD */

   /* Create a new RDD node */
   pRddNewNode = ( LPRDDNODE ) hb_xgrab( sizeof( RDDNODE ) );
   memset( pRddNewNode, 0, sizeof( RDDNODE ) );

   /* Fill the new RDD node */
   strncpy( pRddNewNode->szName, szDriver, HARBOUR_MAX_RDD_DRIVERNAME_LENGTH );
   pRddNewNode->uiType = uiType;

   /* Call <szDriver>_GETFUNCTABLE() */
   hb_vmPushSymbol( pGetFuncTable->pSymbol );
   hb_vmPushNil();
   hb_vmPushLong( ( long ) &uiFunctions );
   hb_vmPushLong( ( long ) &pRddNewNode->pTable );
   hb_vmDo( 2 );
   if ( hb_parni( -1 ) != SUCCESS )
   {
      hb_xfree( pRddNewNode );         /* Delete de new RDD node */
      return 3;                        /* Invalid FUNCTABLE */
   }

   if( !pRddList )                     /* First RDD node */
      pRddList = pRddNewNode;
   else
   {
      pRddNode = pRddList;
      while( pRddNode->pNext )
         pRddNode = pRddNode->pNext;   /* Locate the last RDD node */
      pRddNode->pNext = pRddNewNode;   /* Add the new RDD node */
   }
   return 0;                           /* Ok */
}

static USHORT hb_rddSelect( char * szAlias )
{
   PHB_DYNS pSymAlias;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddSelect(%s)", szAlias));

   pSymAlias = hb_dynsymFindName( szAlias );
   if( pSymAlias && pSymAlias->hArea )
      return pSymAlias->hArea;
   else
      return 0;
}

static void hb_rddSelectFirstAvailable( void )
{
   LPAREANODE pAreaNode;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddSelectFirstAvailable()"));

   uiCurrArea = 1;
   pAreaNode = pWorkAreas;
   while( pAreaNode )
   {
      if( ( ( AREAP ) pAreaNode->pArea )->uiArea > uiCurrArea )
         break;
      else if( ( ( AREAP ) pAreaNode->pArea )->uiArea == uiCurrArea )
         uiCurrArea++;
      pAreaNode = pAreaNode->pNext;
   }
   pCurrArea = NULL;   /* Selected WorkArea must be created */
}

ERRCODE hb_rddInherit( PRDDFUNCS pTable, PRDDFUNCS pSubTable, PRDDFUNCS pSuperTable, BYTE * szDrvName )
{
   char * szSuperName;
   LPRDDNODE pRddNode;
   USHORT uiCount;
   DBENTRYP_V * pFunction, * pSubFunction;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddInherit(%p, %p, %p, %s)", pTable, pSubTable, pSuperTable, szDrvName));

   if( !pTable )
      return FAILURE;

   /* Copy the pSuperTable into pTable */
   if( !szDrvName || !( uiCount = strlen( ( const char * ) szDrvName ) ) )
   {
      memcpy( pTable, &defTable, sizeof( RDDFUNCS ) );
      memcpy( pSuperTable, &defTable, sizeof( RDDFUNCS ) );
   }
   else
   {
      szSuperName = ( char * ) hb_xgrab( uiCount + 1 );
      hb_strncpyUpper( szSuperName, ( char * ) szDrvName, uiCount );
      pRddNode = hb_rddFindNode( szSuperName, 0 );
      hb_xfree( szSuperName );
      if( !pRddNode )
      {
         return FAILURE;
      }
      memcpy( pTable, &pRddNode->pTable, sizeof( RDDFUNCS ) );
      memcpy( pSuperTable, &pRddNode->pTable, sizeof( RDDFUNCS ) );
   }

   /* Copy the non NULL entries from pSubTable into pTable */
   pFunction = ( DBENTRYP_V * ) pTable;
   pSubFunction = ( DBENTRYP_V * ) pSubTable;
   for( uiCount = 0; uiCount < RDDFUNCSCOUNT; uiCount++ )
   {
      if( * pSubFunction )
         * pFunction = * pSubFunction;
      pFunction += 1;
      pSubFunction += 1;
   }
   return SUCCESS;
}

/*
 *  Function for getting current workarea pointer
 */

void * hb_rddGetCurrentWorkAreaPointer( void )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_rddGetCurrentWorkAreaPointer()"));

   return pCurrArea->pArea;
}

/*
 * -- FUNCTIONS ACCESSED FROM VIRTUAL MACHINE --
 */

int  hb_rddGetCurrentWorkAreaNumber( void )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_rddGetCurrentWorkAreaNumber()"));

   return uiCurrArea;
}

ERRCODE hb_rddSelectWorkAreaNumber( int iArea )
{
   LPAREANODE pAreaNode;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddSelectWorkAreaNumber(%d)", iArea));

   uiCurrArea = iArea;

   pAreaNode = pWorkAreas;
   while( pAreaNode )
   {
      if( ( ( AREAP ) pAreaNode->pArea )->uiArea == uiCurrArea )
      {
         pCurrArea = pAreaNode; /* Select a valid WorkArea */
         return SUCCESS;
      }
      pAreaNode = pAreaNode->pNext;
   }
   pCurrArea = NULL;               /* Selected WorkArea is closed */
   return FAILURE;
}

ERRCODE hb_rddSelectWorkAreaSymbol( PHB_SYMB pSymAlias )
{
   ERRCODE bResult;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddSelectWorkAreaSymbol(%p)", pSymAlias));

   if( pSymAlias->pDynSym->hArea )
      bResult = hb_rddSelectWorkAreaNumber( pSymAlias->pDynSym->hArea );
   else
   {
      char * szName = pSymAlias->pDynSym->pSymbol->szName;

      if( strlen( szName ) == 1 && toupper( szName[ 0 ] ) >= 'A' && toupper( szName[ 0 ] ) <= 'K' )
         bResult = hb_rddSelectWorkAreaNumber( toupper( szName[ 0 ] ) - 'A' + 1 );
      else
      {
         /* generate an error with retry possibility
          * (user created error handler can open a missing database)
          */
         USHORT uiAction = E_RETRY;
         HB_ITEM_PTR pError;

         pError = hb_errRT_New( ES_ERROR, NULL, EG_NOALIAS, 1002,
                                NULL, pSymAlias->szName, 0, EF_CANRETRY );

         bResult = FAILURE;
         while( uiAction == E_RETRY )
         {
            uiAction = hb_errLaunch( pError );
            if( uiAction == E_RETRY )
               if( pSymAlias->pDynSym->hArea )
               {
                  bResult = hb_rddSelectWorkAreaNumber( pSymAlias->pDynSym->hArea );
                  uiAction = E_DEFAULT;
               }
         }
         hb_errRelease( pError );
      }
   }
   return bResult;
}

ERRCODE hb_rddSelectWorkAreaAlias( char * szName )
{
   ERRCODE bResult;
   ULONG ulLen;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddSelectWorkAreaAlias(%s)", szName));

   ulLen = strlen( szName );
   if( ulLen >= 1 && toupper( szName[ 0 ] ) > '0' && toupper( szName[ 0 ] ) <= '9' )
      bResult = hb_rddSelectWorkAreaNumber( atoi( szName ) );
   else if( ulLen == 1 && toupper( szName[ 0 ] ) >= 'A' && toupper( szName[ 0 ] ) <= 'K' )
      bResult = hb_rddSelectWorkAreaNumber( toupper( szName[ 0 ] ) - 'A' + 1 );
   else
   {
      PHB_DYNS pSymArea;

      pSymArea = hb_dynsymFindName( szName );
      if( pSymArea && pSymArea->hArea )
         bResult = hb_rddSelectWorkAreaNumber( pSymArea->hArea );
      else
      {
         /* generate an error with retry possibility
          * (user created error handler can open a missing database)
          */
         USHORT uiAction = E_RETRY;
         HB_ITEM_PTR pError;

         pError = hb_errRT_New( ES_ERROR, NULL, EG_NOALIAS, 1002,
                                NULL, szName, 0, EF_CANRETRY );

         bResult = FAILURE;
         while( uiAction == E_RETRY )
         {
            uiAction = hb_errLaunch( pError );
            if( uiAction == E_RETRY )
            {
               pSymArea = hb_dynsymFindName( szName );
               if( pSymArea && pSymArea->hArea )
               {
                  bResult = hb_rddSelectWorkAreaNumber( pSymArea->hArea );
                  uiAction = E_DEFAULT;
               }
            }
         }
         hb_errRelease( pError );
      }
   }

   return bResult;
}

ERRCODE hb_rddGetFieldValue( HB_ITEM_PTR pItem, PHB_SYMB pFieldSymbol )
{
   ERRCODE bSuccess;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddGetFieldValue(%p, %p)", pItem, pFieldSymbol));

   bSuccess = hb_rddFieldGet( pItem, pFieldSymbol );
   if( bSuccess == FAILURE )
   {
      /* generate an error with retry possibility
       * (user created error handler can make this field accessible)
       */
      USHORT uiAction = E_RETRY;
      HB_ITEM_PTR pError;

      pError = hb_errRT_New( ES_ERROR, NULL, EG_NOVAR, 1003,
                             NULL, pFieldSymbol->szName, 0, EF_CANRETRY );

      while( uiAction == E_RETRY )
      {
         uiAction = hb_errLaunch( pError );
         if( uiAction == E_RETRY )
         {
            bSuccess = hb_rddFieldGet( pItem, pFieldSymbol );
            if( bSuccess == SUCCESS )
               uiAction = E_DEFAULT;
         }
      }
      hb_errRelease( pError );
   }
   return bSuccess;
}

ERRCODE hb_rddPutFieldValue( HB_ITEM_PTR pItem, PHB_SYMB pFieldSymbol )
{
   ERRCODE bSuccess;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddPutFieldValue(%p, %p)", pItem, pFieldSymbol));

   bSuccess = hb_rddFieldPut( pItem, pFieldSymbol );
   if( bSuccess == FAILURE )
   {
      /* generate an error with retry possibility
       * (user created error handler can make this field accessible)
       */
      USHORT uiAction = E_RETRY;
      HB_ITEM_PTR pError;

      pError = hb_errRT_New( ES_ERROR, NULL, EG_NOVAR, 1003,
                             NULL, pFieldSymbol->szName, 0, EF_CANRETRY );

      while( uiAction == E_RETRY )
      {
         uiAction = hb_errLaunch( pError );
         if( uiAction == E_RETRY )
         {
            bSuccess = hb_rddFieldPut( pItem, pFieldSymbol );
            if( bSuccess == SUCCESS )
               uiAction = E_DEFAULT;
         }
      }
      hb_errRelease( pError );
   }
   return bSuccess;
}

ERRCODE hb_rddFieldPut( HB_ITEM_PTR pItem, PHB_SYMB pFieldSymbol )
{
   LPFIELD pField;
   USHORT uiField;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddFieldPut(%p, %p)", pItem, pFieldSymbol));

   if( pCurrArea )
   {
      uiField = 1;
      pField = ( ( AREAP ) pCurrArea->pArea )->lpFields;
      while( pField )
      {
         if( ( PHB_DYNS ) pField->sym == pFieldSymbol->pDynSym )
         {
            SELF_PUTVALUE( ( AREAP ) pCurrArea->pArea, uiField, pItem );
            return SUCCESS;
         }
         pField = pField->lpfNext;
         uiField++;
      }
   }
   return FAILURE;
}

ERRCODE hb_rddFieldGet( HB_ITEM_PTR pItem, PHB_SYMB pFieldSymbol )
{
   LPFIELD pField;
   USHORT uiField;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddFieldGet(%p, %p)", pItem, pFieldSymbol));

   if( pCurrArea )
   {
      uiField = 1;
      pField = ( ( AREAP ) pCurrArea->pArea )->lpFields;
      while( pField )
      {
         if( ( PHB_DYNS ) pField->sym == pFieldSymbol->pDynSym )
         {
            SELF_GETVALUE( ( AREAP ) pCurrArea->pArea, uiField, pItem );
            return SUCCESS;
         }
         pField = pField->lpfNext;
         uiField++;
      }
   }
   return FAILURE;
}

void hb_rddShutDown( void )
{
   LPRDDNODE pRddNode;

   HB_TRACE(HB_TR_DEBUG, ("hb_rddShutDown()"));

   hb_rddCloseAll();
   if( szDefDriver )
      hb_xfree( szDefDriver );
   while( pRddList )
   {
      pRddNode = pRddList;
      pRddList = pRddList->pNext;
      hb_xfree( pRddNode );
   }
}

/*
 * -- HARBOUR FUNCTIONS --
 */

/*  $DOC$
 *  $FUNCNAME$
 *     AFIELDS()
 *  $CATEGORY$
 *      DATA BASE
 *  $ONELINER$
 *      Fills referenced arrays with database field information
 *  $SYNTAX$
 *      AFields(<aNames>[,<aTypes>][,<aLen>][,<aDecs>]) --> <nFields>
 *  $ARGUMENTS$
 *      <aNames>  Array of field names
 *      <aTypes>  Array of field names
 *      <aLens>  Array of field names
 *      <aDecs>  Array of field names
 *  $RETURNS$
 *      <nFields> Number od fields in a database or work area
 *  $DESCRIPTION$
 *        This function will fill a series of arrays with field
 *      names,field types,field lenghts, and number of field
 *      decimal positions for the currently selected or designed
 *      database. Each array parallels the different descriptors
 *      of a file's structure.The first array will consist of the
 *      names of the fields in the current work area.All other arrays
 *      are optional and will be filled with the corrensponding data.
 *      This function will return zero if no parameters are specified
 *      or if no database is avaliable in the current work area.Otherwise,
 *      the number of fields or the lenght of the shortest array argument,
 *      witchever is smaller, will be returned.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *         LOCAL aNames:={},aTypes:={},aLens:={},aDecs:={},nFields:=0
 *
 *         USE Test
 *
 *         dbGoTop()
 *         nFields:=aFields(aNames,aTypes,aLens,aDecs)
 *
 *         ? "Number of fields", nFields
 *
 *         RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      AFIELDS() is fully CA-Clipper compliant.
 *  $SEEALSO$
 *  $END$
 */

HARBOUR HB_AFIELDS( void )
{
   PHB_ITEM pName, pType, pLen, pDec, pItem;
   USHORT uiFields, uiArrayLen, uiCount;

   if( !pCurrArea )
   {
      hb_retni( 0 );
      return;
   }

   pName = hb_param( 1, IT_ARRAY );
   pType = hb_param( 2, IT_ARRAY );
   pLen = hb_param( 3, IT_ARRAY );
   pDec = hb_param( 4, IT_ARRAY );
   if( !pName && !pType && !pLen && !pDec )
   {
      hb_retni( 0 );
      return;
   }

   pItem = hb_itemNew( NULL );
   SELF_FIELDCOUNT( ( AREAP ) pCurrArea->pArea, &uiFields );
   if( pName )
   {
      uiArrayLen = hb_arrayLen( pName );
      if( uiArrayLen > uiFields )
         uiArrayLen = uiFields;
      for( uiCount = 1; uiCount <= uiArrayLen; uiCount++ )
      {
         SELF_FIELDINFO( ( AREAP ) pCurrArea->pArea, uiCount, DBS_NAME, pItem );
         hb_arraySet( pName, uiCount, pItem );
      }
   }
   if( pType )
   {
      uiArrayLen = hb_arrayLen( pType );
      if( uiArrayLen > uiFields )
         uiArrayLen = uiFields;
      for( uiCount = 1; uiCount <= uiArrayLen; uiCount++ )
      {
         SELF_FIELDINFO( ( AREAP ) pCurrArea->pArea, uiCount, DBS_TYPE, pItem );
         hb_arraySet( pType, uiCount, pItem );
      }
   }
   if( pLen )
   {
      uiArrayLen = hb_arrayLen( pLen );
      if( uiArrayLen > uiFields )
         uiArrayLen = uiFields;
      for( uiCount = 1; uiCount <= uiArrayLen; uiCount++ )
      {
         SELF_FIELDINFO( ( AREAP ) pCurrArea->pArea, uiCount, DBS_LEN, pItem );
         hb_arraySet( pLen, uiCount, pItem );
      }
   }
   if( pDec )
   {
      uiArrayLen = hb_arrayLen( pDec );
      if( uiArrayLen > uiFields )
         uiArrayLen = uiFields;
      for( uiCount = 1; uiCount <= uiArrayLen; uiCount++ )
      {
         SELF_FIELDINFO( ( AREAP ) pCurrArea->pArea, uiCount, DBS_DEC, pItem );
         hb_arraySet( pDec, uiCount, pItem );
      }
   }

   hb_itemRelease( pItem );
   hb_retni( uiArrayLen );
}

/*  $DOC$
 *  $FUNCNAME$
 *     ALIAS()
 *  $CATEGORY$
 *      DATA BASE
 *  $ONELINER$
 *      Returns the alias name of a work area
 *  $SYNTAX$
 *      Alias([<nWorkArea>]) --> <cWorkArea>
 *  $ARGUMENTS$
 *      <nWorkArea> Number of a work area
 *  $RETURNS$
 *      <cWorkArea> Name of alias
 *  $DESCRIPTION$
 *      This function returns the alias of the work area indicated by <nWorkArea>
 *      If <nWorkArea> is not provided, the alias of the current work area is
 *      returned.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *
 *      USE Test
 *      select 0
 *      qOut( IF(Alias()=="","No Name",Alias()))
 *      Test->(qOut(Alias())
 *      qOut(Alias(1))
 *
 *      RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      ALIAS() is fully CA-Clipper compliant.
 *  $SEEALSO$
 *  $END$
 */

HARBOUR HB_ALIAS( void )
{
   USHORT uiArea;
   LPAREANODE pAreaNode;
   char * szAlias;

   uiArea = hb_parni( 1 );
   uiArea = uiArea ? uiArea : uiCurrArea;
   pAreaNode = pWorkAreas;
   while( pAreaNode )
   {
      if( ( ( AREAP ) pAreaNode->pArea )->uiArea == uiArea )
      {
         if( ( ( AREAP ) pAreaNode->pArea )->atomAlias &&
             ( ( PHB_DYNS ) ( ( AREAP ) pAreaNode->pArea )->atomAlias )->hArea )
         {
            szAlias = ( char * ) hb_xgrab( HARBOUR_MAX_RDD_ALIAS_LENGTH + 1 );
            SELF_ALIAS( ( AREAP ) pAreaNode->pArea, ( BYTE * ) szAlias );
            hb_retc( szAlias );
            hb_xfree( szAlias );
            return;
         }
         break;
      }
      pAreaNode = pAreaNode->pNext;
   }
   hb_retc( "" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBEVAL()
 *  $CATEGORY$
 *      DATA BASE
 *  $ONELINER$
 *      Performs a code block operation on the current DATA BASE
 *  $SYNTAX$
 *      DBEVAL( <bBlock>,
 *      [<bFor>], [<bWhile>],
 *      [<nNext>], [<nRecord>],
 *      [<lRest>] ) --> NIL
 *  $ARGUMENTS$
 *      <bBlock> Operation that is to be performed
 *      <bFor> Code block for the For condition
 *      <bWhile> Code block for the WHILE condition
 *      <nNext> Number of NEXT records  to process
 *      <nRecord> Record number to work on exactly
 *      <lRest> Toggle to rewind record pointer
 *  $RETURNS$
 *      NIL
 *  $DESCRIPTION$
 *        Performs a code block operation on the current DATA BASE
 *  $EXAMPLES$
 *      FUNCTION Main()
 *         LOCAL nCount
 *
 *         USE Test
 *
 *         dbGoto( 4 )
 *         ? RecNo()
 *         COUNT TO nCount
 *         ? RecNo(), nCount
 *         COUNT TO nCount NEXT 10
 *         ? RecNo(), nCount
 *
 *         RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *      DBEVAL is fully CA-Clipper compliant.
 *  $SEEALSO$
 *  $END$
 */

HARBOUR HB_DBEVAL( void )
{
   if( pCurrArea )
   {
      DBEVALINFO pEvalInfo;

      pEvalInfo.itmBlock = hb_param( 1, IT_BLOCK );
      if( !pEvalInfo.itmBlock )
      {
         hb_errRT_DBCMD( EG_ARG, 2019, NULL, "DBEVAL" );
         return;
      }

      pEvalInfo.dbsci.itmCobFor = hb_param( 2, IT_BLOCK );
      if( !pEvalInfo.dbsci.itmCobFor )
      {
         if( !ISNIL( 2 ) )
         {
            hb_errRT_DBCMD( EG_ARG, 2019, NULL, "DBEVAL" );
            return;
         }
      }

      pEvalInfo.dbsci.itmCobWhile = hb_param( 3, IT_BLOCK );
      if( !pEvalInfo.dbsci.itmCobWhile )
      {
         if( !ISNIL( 3 ) )
         {
            hb_errRT_DBCMD( EG_ARG, 2019, NULL, "DBEVAL" );
            return;
         }
      }

      pEvalInfo.dbsci.lNext = hb_param( 4, IT_NUMERIC );
      if( !pEvalInfo.dbsci.lNext )
      {
         if( !ISNIL( 4 ) )
         {
            hb_errRT_DBCMD( EG_ARG, 2019, NULL, "DBEVAL" );
            return;
         }
      }

      pEvalInfo.dbsci.itmRecID = hb_param( 5, IT_NUMERIC );
      if( !pEvalInfo.dbsci.itmRecID )
      {
         if( !ISNIL( 5 ) )
         {
            hb_errRT_DBCMD( EG_ARG, 2019, NULL, "DBEVAL" );
            return;
         }
      }

      pEvalInfo.dbsci.fRest = hb_param( 6, IT_LOGICAL );
      if( !pEvalInfo.dbsci.fRest )
      {
         if( !ISNIL( 6 ) )
         {
            hb_errRT_DBCMD( EG_ARG, 2019, NULL, "DBEVAL" );
            return;
         }
      }

      SELF_DBEVAL( ( AREAP ) pCurrArea->pArea, &pEvalInfo );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBEVAL" );
}

/*  $DOC$
 *  $FUNCNAME$
 *     DBF()
 *  $CATEGORY$
 *      DATA BASE
 *  $ONELINER$
 *      Alias name of a work area
 *  $SYNTAX$
 *      Dbf() --> <cWorkArea>
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      <cWorkArea> Name of alias
 *  $DESCRIPTION$
 *      This function returns the same alias name ofthe currently selected work
 *      area.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *
 *         USE Test
 *
 *         select 0
 *         qOut( IF(DBF()=="","No Name",DBF()))
 *         Test->(qOut(DBF())
 *         qOut(Alias(1))
 *
 *         RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      DBF() is fully CA-Clipper compliant.
 *  $SEEALSO$
 *  $END$
 */

HARBOUR HB_DBF( void )
{
   LPAREANODE pAreaNode = pWorkAreas;

   while( pAreaNode )
   {
      if( ( ( AREAP ) pAreaNode->pArea )->uiArea == uiCurrArea )
      {
         if( ( ( AREAP ) pAreaNode->pArea )->atomAlias &&
             ( ( PHB_DYNS ) ( ( AREAP ) pAreaNode->pArea )->atomAlias )->hArea )
         {
            char * szAlias = ( char * ) hb_xgrab( HARBOUR_MAX_RDD_ALIAS_LENGTH + 1 );
            SELF_ALIAS( ( AREAP ) pAreaNode->pArea, ( BYTE * ) szAlias );
            hb_retc( szAlias );
            hb_xfree( szAlias );
            return;
         }
         break;
      }
      pAreaNode = pAreaNode->pNext;
   }
   hb_retc( "" );
}

/*  $DOC$
 *  $FUNCNAME$
 *     BOF()
 *  $CATEGORY$
 *      DATA BASE
 *  $ONELINER$
 *      Test for the beggining-of-file condition
 *  $SYNTAX$
 *      BOF() --> <lBegin>
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      Logical true (.T.) or false (.F.)
 *  $DESCRIPTION$
 *      This function determines if the beggining of the file marker has been
 *      reached. If so, the function will return a logical true (.T.); otherwise,
 *      a logical false(.F.) will be returned.
 *      By default, BOF() will apply to the currently selected database unless
 *      the function is preceded by an alias
 *  $EXAMPLES$
 *      FUNCTION Main()
 *        USE Tests NEW
 *        DBGOTOP()
 *        ? "Is Eof()",EOF()
 *        DBGOBOTTOM()
 *        ? "Is Eof()",EOF()
 *        USE
 *     RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      BOF() is fully CA-Clipper compliant.
 *  $SEEALSO$
 *    EOF(),FOUND(),LASTREC()
 *  $END$
 */

HARBOUR HB_BOF( void )
{
   BOOL bBof = TRUE;

   if( pCurrArea )
      SELF_BOF( ( AREAP ) pCurrArea->pArea, &bBof );
   hb_retl( bBof );
}

/*  $DOC$
 *  $FUNCNAME$
 *     DBAPPEND()
 *  $CATEGORY$
 *      DATA BASE
 *  $ONELINER$
 *      Appends a new record to a database file.
 *  $SYNTAX$
 *      DbAppend(<<lLock>]) --> NIL
 *  $ARGUMENTS$
 *      <lLock> Toggle to release record locks
 *  $RETURNS$
 *  $DESCRIPTION$
 *        This function add a new record to the end of the database
 *     in the selected or aliased work area. All fields in that
 *     database will be given empty data values - character fields
 *     will be filled with blank spaces,date fields with CTOD('//'),
 *     numeric fields with 0,logical fields with .F., and memo fields
 *     with NULL bytes.The header of the database is not updated until
 *     the record is flushed from the buffer and the contents are
 *     written to the disk.
 *        Under a networking enviroment, DBAPPEND() performs an addi-
 *     tional operation: It attrmps to lock the newly added record. If
 *     the database file is currently locked or if a locking assignment
 *     if made to LASTREC()+1,NETERR() will return a logical true (.T.)
 *     immediately after the DBAPPEND() function. This function does
 *     not unlock the locked records.
 *        If <lLock> is passed a logical true (.T.) value, it will
 *     release the record locks, which allows the application to main-
 *     tain multiple record locks during an appending operation. The
 *     default for this parameter is a logical false (.F.).
 *  $EXAMPLES$
 *      FUNCTION Main()
 *
 *         USE Test
 *         local cName="HARBOUR",nId=10
 *         Test->(DbAppend())
 *         Replace Test->Name wit cName,Id with nId
 *         Use
 *         RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      DBAPPEND() is fully CA-Clipper compliant.
 *  $SEEALSO$
 *  $END$
 */

HARBOUR HB_DBAPPEND( void )
{
   BOOL bUnLockAll = TRUE;

   if( pCurrArea )
   {
      bNetError = FALSE;
      if( ISLOG( 1 ) )
         bUnLockAll = hb_parl( 1 );
      bNetError = ( SELF_APPEND( ( AREAP ) pCurrArea->pArea, bUnLockAll ) == FAILURE );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBAPPEND" );
}

/*  $DOC$
 *  $FUNCNAME$
 *     DBCLEARFILTER()
 *  $CATEGORY$
 *      DATA BASE
 *  $ONELINER$
 *      Clears the current filter condiction in a work area
 *  $SYNTAX$
 *      DbClearFilTer() -> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *
 *  $DESCRIPTION$
 *        This function clears any active filter condiction
 *      for the current or selected work area.
 *  $EXAMPLES$
 *      Function Main()
 *
 *       Use Test
 *
 *       Set Filter to Left(Test->Name,2) == "An"
 *
 *       Dbedit()
 *
 *       Test->(DbClearFilter())
 *
 *       USE
 *
 *       Return Nil
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      DBCLEARFILTER() is fully CA-Clipper compliant.
 *  $SEEALSO$
 *  $END$
 */

HARBOUR HB_DBCLEARFILTER( void )
{
   if( pCurrArea )
      SELF_CLEARFILTER( ( AREAP ) pCurrArea->pArea );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBCLEARFILTER" );
}

/*  $DOC$
 *  $FUNCNAME$
 *     DBCLOSEALL()
 *  $CATEGORY$
 *      DATA BASE
 *  $ONELINER$
 *      Close all open files in all work areas.
 *  $SYNTAX$
 *      DbCloseAll() -> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      DBCLOSEALL always return NIL
 *  $DESCRIPTION$
 *        This function close all open databases and all associated
 *      indexes.In addition, it closes all format files and moves
 *      the work area pointer to the first position
 *  $EXAMPLES$
 *      Function Main()
 *
 *       Use Test New
 *
 *       DbEdit()
 *
 *       Use Test1 New
 *
 *       DbEdit()
 *
 *       DbCloseAll()
 *
 *       USE
 *
 *       Return Nil
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      DBCLOSEALL() is fully CA-Clipper compliant.
 *  $SEEALSO$
 *  $END$
 */

HARBOUR HB_DBCLOSEALL( void )
{
   hb_rddCloseAll();
}

/*  $DOC$
 *  $FUNCNAME$
 *     DBCLOSEAREA()
 *  $CATEGORY$
 *      DATA BASE
 *  $ONELINER$
 *      Close a database file in a work area.
 *  $SYNTAX$
 *      DbCloseArea() -> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *
 *  $DESCRIPTION$
 *        This function  will close any database open in the selected
 *      or aliased work area.
 *  $EXAMPLES$
 *      Function Main()
 *
 *       Use Test
 *
 *       Dbedit()
 *
 *       Test->(DbCloseArea())
 *
 *       USE
 *
 *       Return Nil
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      DBCLOSEAREA() is fully CA-Clipper compliant.
 *  $SEEALSO$
 *  $END$
 */

HARBOUR HB_DBCLOSEAREA( void )
{
   if( !pCurrArea )
      return;

   SELF_CLOSE( ( AREAP ) pCurrArea->pArea );
   SELF_RELEASE( ( AREAP ) pCurrArea->pArea );

   if( pWorkAreas == pCurrArea )
   {
      pWorkAreas = pCurrArea->pNext;
      if( pWorkAreas )
         pWorkAreas->pPrev = NULL;
   }
   else
   {
      if( pCurrArea->pPrev )
         pCurrArea->pPrev->pNext = pCurrArea->pNext;
      if( pCurrArea->pNext )
         pCurrArea->pNext->pPrev = pCurrArea->pPrev;
   }

   hb_xfree( pCurrArea->pArea );
   hb_xfree( pCurrArea );
   pCurrArea = NULL;
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBCOMMIT()
 *  $CATEGORY$
 *      DATA BASE
 *  $ONELINER$
 *      Updates all index and database buffers for a given workarea
 *  $SYNTAX$
 *     DBCOMMIT() --> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      DBCOMMIT() always returns NIL.
 *  $DESCRIPTION$
 *      This function updates all of the information for a give,selected,
 *      or active workarea.This operation includes all database and index
 *      buffers for that work area only. This function does not update all
 *      open work areas.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      LOCAL cName:=SPACE(40)
 *      LOCAL nId:=0
 *      USE Test EXCLUSIVE NEW
 *      //
 *      @ 10, 10 GET cName
 *      @ 11, 10 GET nId
 *      READ
 *      //
 *      IF UPDATED()
 *         APPEND BLANK
 *         REPLACE Tests->Name WITH cName
 *         REPLACE Tests->Id WITH nId
 *         Tests->( DBCOMMIT() )
 *      ENDIF
 *      RETURN NIL
 *  $TESTS$
 *
 *  $STATUS$
 *     R
 *  $COMPLIANCE$
 *  $SEEALSO$
 *     DBCLOSEALL(),DBCOMMITALL(),DBUNLOCK()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBCOMMIT( void )
{
   if( pCurrArea )
      SELF_FLUSH( ( AREAP ) pCurrArea->pArea );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBCOMMIT" );
}

/*  $DOC$
 *  $FUNCNAME$
 *     DBCOMMITALL()
 *  $CATEGORY$
 *     DATA BASE
 *  $ONELINER$
 *     Flushes the memory buffer and performs a hard-disk write
 *  $SYNTAX$
 *     DBCOMMIT() --> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *     DBCOMMIT() always returns NIL.
 *  $DESCRIPTION$
 *      This function performs a hard-disk write for all work areas.
 *      Before the disk write is performed,all buffers are flushed.
 *      open work areas.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      LOCAL cName:=SPACE(40)
 *      LOCAL nId:=0
 *      USE Test EXCLUSIVE NEW
 *      USE TestId New INDEX Testid
 *      //
 *      @ 10, 10 GET cName
 *      @ 11, 10 GET nId
 *      READ
 *      //
 *      IF UPDATED()
 *         APPEND BLANK
 *         REPLACE Tests->Name WITH cName
 *         REPLACE Tests->Id WITH nId
 *         IF !TestId->(DBSEEK(nId))
 *            APPEND BLANK
 *            REPLACE Tests->Id WITH nId
 *         ENDIF
 *      ENDIF
 *      DBCOMMITALL()
 *      RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *     R
 *  $COMPLIANCE$
 *  $SEEALSO$
 *     DBCLOSEALL(),DBCOMMIT(),DBUNLOCK()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBCOMMITALL( void )
{
   LPAREANODE pAreaNode;

   pAreaNode = pWorkAreas;
   while( pAreaNode )
   {
      SELF_FLUSH( ( AREAP ) pAreaNode->pArea );
      pAreaNode = pAreaNode->pNext;
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *     __DBCONTINUE()
 *  $CATEGORY$
 *     DATA BASE
 *  $ONELINER$
 *     Resume a pending LOCATE
 *  $SYNTAX$
 *     __DbCONTINUE()   -> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *     __DbCONTINUE()  Always return nil
 *  $DESCRIPTION$
 *   __DBCONTINUE is a database command that searches from the current record
 *   position for the next record meeting the most recent LOCATE condition
 *   executed in the current work area.  It terminates when a match is found
 *   or end of file is encountered.  If __DBCONTINUE is successful, the matching
 *   record becomes the current record and FOUND() returns true (.T.); if
 *   unsuccessful, FOUND() returns false (.F.).
 *
 *   Each work area may have an active LOCATE condition.  In CA-Clipper, a
 *   LOCATE condition remains pending until a new LOCATE condition is
 *   specified.  No other commands release the condition.
 *
 *   Notes
 *
 *     Scope and WHILE condition: Note that the scope and WHILE
 *      condition of the initial LOCATE are ignored; only the FOR condition
 *      is used with CONTINUE.  If you are using a LOCATE with a WHILE
 *      condition and want to continue the search for a matching record, use
 *      SKIP and then repeat the original LOCATE statement adding REST as the
 *      scope.
 *
 *  $EXAMPLES$
 *     This example scans records in Sales.dbf for a particular
 *      salesman and displays a running total sales amounts:
 *
 *      LOCAL nRunTotal := 0
 *      USE Sales NEW
 *      LOCATE FOR Sales->Salesman = "1002"
 *      DO WHILE FOUND()
 *         ? Sales->Salesname, nRunTotal += Sales->Amount
 *         __DBCONTINUE()
 *      ENDDO
 *
 *     This example demonstrates how to continue if the pending
 *      LOCATE scope contains a WHILE condition:
 *
 *      LOCAL nRunTotal := 0
 *      USE Sales INDEX Salesman NEW
 *      SEEK "1002"
 *      LOCATE REST WHILE Sales->Salesman = "1002";
 *            FOR Sales->Amount > 5000
 *      DO WHILE FOUND()
 *         ? Sales->Salesname, nRunTotal += Sales->Amount
 *         SKIP
 *         LOCATE REST WHILE Sales->Salesman = "1002";
 *            FOR Sales->Amount > 5000
 *      ENDDO
 *
 *  $TESTS$
 *
 *  $STATUS$
 *     R
 *  $COMPLIANCE$
 *
 *
 *  $SEEALSO$
 *     EOF(),FOUND()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB___DBCONTINUE()
{
   BOOL bEof;

   if( !pCurrArea )
   {
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBCONTINUE" );
      return;
   }

   if( !( ( AREAP ) pCurrArea->pArea )->dbsi.itmCobFor )
      return;
   ( ( AREAP ) pCurrArea->pArea )->fFound = FALSE;
   SELF_SKIP( ( AREAP ) pCurrArea->pArea, 1 );
   SELF_EOF( ( AREAP ) pCurrArea->pArea, &bEof );
   if( bEof )
      return;
   hb_vmPushSymbol( &hb_symEval );
   hb_vmPush( ( ( AREAP ) pCurrArea->pArea )->dbsi.itmCobFor );
   hb_vmDo( 0 );
   ( ( AREAP ) pCurrArea->pArea )->fFound = hb_itemGetL( &hb_stack.Return );
   while( !bEof && !( ( AREAP ) pCurrArea->pArea )->fFound )
   {
      SELF_SKIP( ( AREAP ) pCurrArea->pArea, 1 );
      SELF_EOF( ( AREAP ) pCurrArea->pArea, &bEof );
      hb_vmPushSymbol( &hb_symEval );
      hb_vmPush( ( ( AREAP ) pCurrArea->pArea )->dbsi.itmCobFor );
      hb_vmDo( 0 );
      ( ( AREAP ) pCurrArea->pArea )->fFound = hb_itemGetL( &hb_stack.Return );
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *     DBCREATE()
 *  $CATEGORY$
 *     DATA BASE
 *  $ONELINER$
 *      Creates an empty database from a array.
 *  $SYNTAX$
 *     DBCREATE(<cDatabase>, <aStruct>,[<cDriver>],[<lOpen>],[<cAlias>]) --> NIL
 *  $ARGUMENTS$
 *      <cDatabase> Name of database to be create
 *      <aStruct>   Name of a multidimensional array that contains the a database
 *                  structure
 *      <cDriver>   Name of the RDD
 *      <lOpen>     Toggle to Open the create File
 *      <cAlias>    Name of database Alias
 *  $RETURNS$
 *      DBCREATE() always returns NIL.
 *  $DESCRIPTION$
 *      This function creates the database file specified as <cDatabase> from the
 *      multidimensional array <aStruct>.If no file extension is use with <cDatabase>
 *      the .DBF extension is assumed.
 *      The array specified in <aStruct> must follow a few guidelines when being
 *      built prior to a call to DBCREATE():
 *
 *      - All subscripts values in the second dimension must be set to proper values
 *
 *      - The fourth subscript value in the second dimension - which contains
 *        the decimal value-must he specified. even 1kw nonnumeric fields.
 *
 *      - The second subscript value in the second dimension-which contains
 *        the field data type-must contain a proper value: C, D, L, M or N
 *        It is possible to use additional letters (or clarity (e.g., 'Numeric'
 *        for 'N'): however, the first letter of this array element must
 *        be a proper value.
 *
 *        The DBCREATE( ) function does not use the decimal field to
 *        calculate the length of a character held longer than 256. Values
 *        up to the maximum length of a character field (which is 65,519 bytes)
 *        are stored directly in the database in the length attribute if that
 *        database was created via this function. However, a file containing
 *        fields longer than 256 bytes is not compatible with any interpreter.
 *
 *      The <cDriver> parameter specifies the name of the Replaceable Da-
 *      tabase Driver to use to create the database. If it is not specified, then the
 *      Replaceable Database Driver in the current work area is tised.
 *      The <lOpen> parameter specifies if the already created database is to be opened,
 *      The <cAlias> parameter specifies the alias name for the new opened database
 *  $EXAMPLES$
 *      function main()
 *
 *      local nI, aStruct := { { "CHARACTER", "C", 25, 0 }, ;
 *                          { "NUMERIC",   "N",  8, 0 }, ;
 *                          { "DOUBLE",    "N",  8, 2 }, ;
 *                          { "DATE",      "D",  8, 0 }, ;
 *                          { "LOGICAL",   "L",  1, 0 }, ;
 *                          { "MEMO1",     "M", 10, 0 }, ;
 *                          { "MEMO2",     "M", 10, 0 } }
 *
 *      REQUEST DBFCDX
 *
 *      dbCreate( "testdbf", aStruct, "DBFCDX", .t., "MYALIAS" )
 *
 *      RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      This function is Not CA-Clipper compliant
 *  $SEEALSO$
 *      AFIELDS(),DBSTRUCT()
 *  $INCLUDE$
 *      "Dbstruct.ch"
 *  $END$
 */

HARBOUR HB_DBCREATE( void )
{
   char * szDriver, * szFileName;
   USHORT uiSize, uiLen, uiRddID;
   ULONG ulRecCount;
   LPRDDNODE pRddNode;
   LPAREANODE pAreaNode;
   DBOPENINFO pInfo;
   PHB_FNAME pFileName;
   PHB_ITEM pStruct, pFieldDesc, pFileExt;
   char cDriverBuffer[ HARBOUR_MAX_RDD_DRIVERNAME_LENGTH ];
   BOOL bError = FALSE;

   szFileName = hb_parc( 1 );
   pStruct = hb_param( 2 , IT_ARRAY );
   uiLen = hb_arrayLen( pStruct );

   if( ( strlen( szFileName ) == 0 ) || !pStruct || uiLen == 0 )
   {
      hb_errRT_DBCMD( EG_ARG, 1014, NULL, "DBCREATE" );
      return;
   }

   for( uiSize = 0; uiSize < uiLen; uiSize++ )
   {
      pFieldDesc = hb_arrayGetItemPtr( pStruct, uiSize + 1 );
      if( hb_arrayLen( pFieldDesc ) < 4 )
      {
         hb_errRT_DBCMD( EG_ARG, 1014, NULL, "DBCREATE" );
         return;
      }

      /* Validate items type, name, size and decimals of field */
      if( !( hb_arrayGetType( pFieldDesc, 1 ) & IT_STRING ) ||
          !( hb_arrayGetType( pFieldDesc, 2 ) & IT_STRING ) ||
          !( hb_arrayGetType( pFieldDesc, 3 ) & IT_NUMERIC ) ||
          !( hb_arrayGetType( pFieldDesc, 4 ) & IT_NUMERIC ) )
      {
         hb_errRT_DBCMD( EG_ARG, 1014, NULL, "DBCREATE" );
         return;
      }
   }

   hb_rddCheck();
   uiLen = hb_parclen( 3 );
   if( uiLen > 0 )
   {
      hb_strncpyUpper( cDriverBuffer, hb_parc( 3 ), uiLen );
      szDriver = cDriverBuffer;
   }
   else
      szDriver = szDefDriver;

   uiRddID = 0;
   pRddNode = hb_rddFindNode( szDriver, &uiRddID ) ;
   if( !pRddNode )
   {
      hb_errRT_DBCMD( EG_ARG, 1015, NULL, "DBCREATE" );
      return;
   }

   if( !ISLOG( 4 ) )
      hb_rddSelectFirstAvailable();
   else
   {
      if( hb_parl( 4 ) )
         hb_rddSelectFirstAvailable();
      else if( pCurrArea )  /* If current WorkArea is in use then close it */
      {
         SELF_CLOSE( ( AREAP ) pCurrArea->pArea );
         SELF_RELEASE( ( AREAP ) pCurrArea->pArea );

         if( pWorkAreas == pCurrArea )
         {
            pWorkAreas = pCurrArea->pNext;
            if( pWorkAreas )
               pWorkAreas->pPrev = NULL;
         }
         else
         {
            if( pCurrArea->pPrev )
               pCurrArea->pPrev->pNext = pCurrArea->pNext;
            if( pCurrArea->pNext )
               pCurrArea->pNext->pPrev = pCurrArea->pPrev;
         }

         hb_xfree( pCurrArea->pArea );
         hb_xfree( pCurrArea );
         pCurrArea = NULL;
      }
   }

   /* Create a new WorkArea node */

   pCurrArea = ( LPAREANODE ) hb_xgrab( sizeof( AREANODE ) );

   if( pRddNode->uiAreaSize == 0 ) /* Calculate the size of WorkArea */
   {
      uiSize = sizeof( AREA );    /* Default Size Area */
      pCurrArea->pArea = ( AREAP ) hb_xgrab( uiSize );
      memset( pCurrArea->pArea, 0, uiSize );
            ( ( AREAP ) pCurrArea->pArea )->lprfsHost = &pRddNode->pTable;

      /* Need more space? */
      SELF_STRUCTSIZE( ( AREAP ) pCurrArea->pArea, &uiSize );
      if( uiSize > sizeof( AREA ) )   /* Size of Area changed */
         pCurrArea->pArea = ( AREAP ) hb_xrealloc( pCurrArea->pArea, uiSize );

      pRddNode->uiAreaSize = uiSize;  /* Update the size of WorkArea */
   }
   else
   {
      pCurrArea->pArea = ( AREAP ) hb_xgrab( pRddNode->uiAreaSize );
      memset( pCurrArea->pArea, 0, pRddNode->uiAreaSize );
              ( ( AREAP ) pCurrArea->pArea )->lprfsHost = &pRddNode->pTable;
   }

   ( ( AREAP ) pCurrArea->pArea )->rddID = uiRddID;

   pCurrArea->pPrev = NULL;
   pCurrArea->pNext = NULL;

   SELF_NEW( ( AREAP ) pCurrArea->pArea );
   if( SELF_CREATEFIELDS( ( AREAP ) pCurrArea->pArea, pStruct ) == FAILURE )
   {
      SELF_RELEASE( ( AREAP ) pCurrArea->pArea );
      hb_xfree( pCurrArea->pArea );
      hb_xfree( pCurrArea );
      pCurrArea = NULL;
      hb_errRT_DBCMD( EG_ARG, 1014, NULL, "DBCREATE" );
      return;
   }

   pFileName = hb_fsFNameSplit( szFileName );
   szFileName = ( char * ) hb_xgrab( _POSIX_PATH_MAX + 3 );
   strcpy( szFileName, hb_parc( 1 ) );
   if( !pFileName->szExtension )
   {
      pFileExt = hb_itemPutC( NULL, "" );
      SELF_INFO( ( AREAP ) pCurrArea->pArea, DBI_TABLEEXT, pFileExt );
      strcat( szFileName, pFileExt->item.asString.value );
      hb_itemRelease( pFileExt );
   }
   hb_xfree( pFileName );
   pInfo.abName = ( BYTE * ) szFileName;
   pInfo.atomAlias = ( BYTE * ) hb_parc( 5 );
   pInfo.uiArea = uiCurrArea;

   ( ( AREAP ) pCurrArea->pArea )->uiArea = uiCurrArea;

   /* Insert the new WorkArea node */

   if( !pWorkAreas )
      pWorkAreas = pCurrArea;  /* The new WorkArea node is the first */
   else
   {
      pAreaNode = pWorkAreas;
      while( pAreaNode )
      {
         if( ( ( AREAP ) pAreaNode->pArea )->uiArea > uiCurrArea )
         {
            /* Insert the new WorkArea node */
            pCurrArea->pPrev = pAreaNode->pPrev;
            pCurrArea->pNext = pAreaNode;
            pAreaNode->pPrev = pCurrArea;
            if( pCurrArea->pPrev )
               pCurrArea->pPrev->pNext = pCurrArea;
            else
               pWorkAreas = pCurrArea;
            break;
         }
         if( pAreaNode->pNext )
            pAreaNode = pAreaNode->pNext;
         else
         {
            /* Append the new WorkArea node */
            pAreaNode->pNext = pCurrArea;
            pCurrArea->pPrev = pAreaNode;
            break;
         }
      }
   }

   ( ( AREAP ) pCurrArea->pArea )->lpDataInfo->szFileName = szFileName;
   ( ( AREAP ) pCurrArea->pArea )->atomAlias = hb_dynsymGet( ( char * ) pInfo.atomAlias );
   if( ( ( PHB_DYNS ) ( ( AREAP ) pCurrArea->pArea )->atomAlias )->hArea )
   {
      hb_errRT_DBCMD( EG_DUPALIAS, 1011, NULL, ( char * ) pInfo.atomAlias );
      bError = TRUE;
   }

   if( !bError )
      bError = ( SELF_CREATE( ( AREAP ) pCurrArea->pArea, &pInfo ) == FAILURE );

   if( !bError )
      ( ( PHB_DYNS ) ( ( AREAP ) pCurrArea->pArea )->atomAlias )->hArea = pInfo.uiArea;

   if( !bError && ( ( AREAP ) pCurrArea->pArea )->lpExtendInfo->fHasMemo )
   {
      pFileExt = hb_itemPutC( NULL, "" );
      SELF_INFO( ( AREAP ) pCurrArea->pArea, DBI_MEMOEXT, pFileExt );
      pFileName = hb_fsFNameSplit( ( char * ) pInfo.abName );
      szFileName = ( char * ) hb_xgrab( _POSIX_PATH_MAX + 3 );
      szFileName[ 0 ] = '\0';
      if( pFileName->szDrive )
         strcat( szFileName, pFileName->szDrive );
      if( pFileName->szPath )
         strcat( szFileName, pFileName->szPath );
      strcat( szFileName, pFileName->szName );
      strcat( szFileName, pFileExt->item.asString.value );
      pInfo.abName = ( BYTE * ) szFileName;
      hb_xfree( pFileName );
      hb_itemRelease( pFileExt );
      ( ( AREAP ) pCurrArea->pArea )->lpDataInfo->pNext =
                              ( LPFILEINFO ) hb_xgrab( sizeof( FILEINFO ) );
      memset( ( ( AREAP ) pCurrArea->pArea )->lpDataInfo->pNext, 0,
              sizeof( FILEINFO ) );
      ( ( AREAP ) pCurrArea->pArea )->lpDataInfo->pNext->hFile = FS_ERROR;
      ( ( AREAP ) pCurrArea->pArea )->lpDataInfo->pNext->szFileName = szFileName;
      bError = ( SELF_CREATEMEMFILE( ( AREAP ) pCurrArea->pArea, &pInfo ) == FAILURE );
   }

   ( ( PHB_DYNS ) ( ( AREAP ) pCurrArea->pArea )->atomAlias )->hArea = 0;
   SELF_RELEASE( ( AREAP ) pCurrArea->pArea );
   if( !ISLOG( 4 ) || bError )
   {
      if( pWorkAreas == pCurrArea )
      {
         pWorkAreas = pCurrArea->pNext;
         if( pWorkAreas )
            pWorkAreas->pPrev = NULL;
      }
      else
      {
         if( pCurrArea->pPrev )
            pCurrArea->pPrev->pNext = pCurrArea->pNext;
         if( pCurrArea->pNext )
            pCurrArea->pNext->pPrev = pCurrArea->pPrev;
      }

      hb_xfree( pCurrArea->pArea );
      hb_xfree( pCurrArea );
      pCurrArea = NULL;
   }
   else
   {
      SELF_NEW( ( AREAP ) pCurrArea->pArea );
      szFileName = hb_parc( 1 );
      pFileName = hb_fsFNameSplit( szFileName );
      szFileName = ( char * ) hb_xgrab( _POSIX_PATH_MAX + 3 );
      strcpy( szFileName, hb_parc( 1 ) );
      if( !pFileName->szExtension )
      {
         pFileExt = hb_itemPutC( NULL, "" );
         SELF_INFO( ( AREAP ) pCurrArea->pArea, DBI_TABLEEXT, pFileExt );
         strcat( szFileName, pFileExt->item.asString.value );
          hb_itemRelease( pFileExt );
      }
      hb_xfree( pFileName );
      pInfo.abName = ( BYTE * ) szFileName;
      pInfo.fShared = !hb_set.HB_SET_EXCLUSIVE;
      pInfo.fReadonly = FALSE;
      ( ( AREAP ) pCurrArea->pArea )->uiArea = uiCurrArea;
      ( ( AREAP ) pCurrArea->pArea )->lpDataInfo->szFileName = szFileName;
      if( SELF_OPEN( ( AREAP ) pCurrArea->pArea, &pInfo ) == FAILURE )
      {
         SELF_RELEASE( ( AREAP ) pCurrArea->pArea );
         hb_xfree( pCurrArea->pArea );
         hb_xfree( pCurrArea );
         pCurrArea = NULL;
      }
      SELF_RECCOUNT( ( AREAP ) pCurrArea->pArea, &ulRecCount );
      ( ( AREAP ) pCurrArea->pArea )->lpExtendInfo->ulRecCount = ulRecCount;
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *     DBDELETE()
 *  $CATEGORY$
 *     DATA BASE
 *  $ONELINER$
 *     Marks records for deletion in a database.
 *  $SYNTAX$
 *     DBDELETE() --> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *     DBDELETE() always returns NIL.
 *  $DESCRIPTION$
 *      This function marks a record for deletion in the selected
 *      or aliased work area.If the DELETED setting is on, the record
 *      will still be visible until the record pointer in that work area
 *      is moved to another record.
 *      In a networking situation, this function requires that the record
 *      be locked prior to issuing the DBDELETE() function.
 *  $EXAMPLES$
 *      nId:=10
 *      USE TestId INDEX TestId NEW
 *      IF TestId->(DBSEEK(nId))
 *         IF TestId->(RLOCK())
 *            DBDELETE()
 *         ENDIF
 *      ENDIF
 *      USE
 *  $TESTS$
 *  $STATUS$
 *     R
 *  $COMPLIANCE$
 *  $SEEALSO$
 *    DBRECALL()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBDELETE( void )
{
   if( pCurrArea )
      SELF_DELETE( ( AREAP ) pCurrArea->pArea );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBDELETE" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBFILTER()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the filter expression in a work area
 *  $SYNTAX$
 *      DBFILTER() --> cFilter
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      DBFILTER() returns the filter expression.
 *  $DESCRIPTION$
 *      This function return the expression of the SET FILTER TO command
 *      for the current or designated work area. If no filter condition
 *      is present,a NULL string will be returned.
 *  $EXAMPLES$
 *      USE Test INDEX Test NEW
 *      SET FILTER TO Name= "Harbour"
 *      USE TestId INDEX TestId NEW
 *      SET FILTER TO Id = 1
 *      SELECT Test
 *      //
 *      ? DBFILTER()
 *      ? TestId->(DBFILTER())
 *  $TESTS$
 *  $STATUS$
 *     R
 *  $COMPLIANCE$
 *  $SEEALSO$
 *      DBRELATION(),DBRSELECT()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBFILTER( void )
{
   PHB_ITEM pFilter;

   if( pCurrArea )
   {
      pFilter = hb_itemPutC( NULL, "" );
      SELF_FILTERTEXT( ( AREAP ) pCurrArea->pArea, pFilter );
      hb_retc( pFilter->item.asString.value );
      hb_itemRelease( pFilter );
   }
   else
      hb_retc( "" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBGOBOTTOM()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Moves the record pointer to the bottom of the database.
 *  $SYNTAX$
 *      DBGOBOTTOM() --> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      DBGOBOTTOM() always returns NIL.
 *  $DESCRIPTION$
 *      This function moves the record pointer in the selected or aliased
 *      work area to the end of the file.The position of the record pointer
 *      is affected by the values in the index key or by an active FILTER
 *      condition.Otherwise,if no index is active or if no filter condition
 *      is present,the value of the record pointer will be LASTREC().
 *  $EXAMPLES$
 *      USE Tests
 *      DBGOTOP()
 *      ? RECNO()
 *      DBGOBOTTOM()
 *      ? RECNO()
 *      USE
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *  $SEEALSO$
 *      BOF(),EOF(),DBSKIP(),DBSEEK(),DBGOTOP()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBGOBOTTOM( void )
{
   if( pCurrArea )
      SELF_GOBOTTOM( ( AREAP ) pCurrArea->pArea );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBGOBOTTOM" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBGOTO()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Position the record pointer to a specific location.
 *  $SYNTAX$
 *      DBGOTO(<xRecordNumber>) --> NIL
 *  $ARGUMENTS$
 *      <xRecordNumber> Record number or unique identity
 *  $RETURNS$
 *      DBGOTO() always returns NIL.
 *  $DESCRIPTION$
 *      This function places the record pointer,if working with a .DBF file,
 *      in selected or aliased work area at the record number specified by
 *      <xRecordNumber>.The position if not affected by an active index or
 *      by any enviromental SET condiction.
 *      Issuing a DBGOTO(RECNO()) call in a network enviroment will refresh
 *      the database and index buffers.This is the same as a DBSKIP(0) call.
 *      The parameter <xRecordNumber> may be something other than a record
 *      number.In some data formats, for example, the value of <xRecordNumber>
 *      is a unique primary key while in other formats,<xRecordNumber> could
 *      be an array offset if the data set was an array.
 *  $EXAMPLES$
 *
 *      The following example uses DBGOTO() to iteratively process
 *      every fourth record:
 *
 *      DBUSEAREA( .T., "DBFNTX", "Sales", "Sales", .T. )
 *      //
 *      // toggle every fourth record
 *      DO WHILE !EOF()
 *         DBGOTO( RECNO() + 4 )
 *         Sales->Group := "Bear"
 *      ENDDO
 *
 *  $TESTS$
 *
 *  $STATUS$
 *     R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *     BOF(),EOF(),DBGOTOP(),DBGOBOTTOM(),DBSEEK(),DBSKIP()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBGOTO( void )
{
   PHB_ITEM pItem;

   if( !pCurrArea )
   {
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBGOTO" );
      return;
   }

   pItem = hb_param( 1, IT_ANY );
   if( !pItem )
      hb_errRT_DBCMD( EG_ARG, 1003, NULL, "DBGOTO" );
   else
      SELF_GOTOID( ( AREAP ) pCurrArea->pArea, pItem );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBGOTOP()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Moves the record pointer to the bottom of the database.
 *  $SYNTAX$
 *      DBGOTOP() --> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      DBGOTOP() always returns NIL.
 *      This function moves the record pointer in the selected or aliased
 *      work area to the top of the file.The position of the record pointer
 *      is affected by the values in the index key or by an active FILTER
 *      condition.Otherwise,if no index is active or if no filter condition
 *      is present,the value of RECNO() will be 1.
 *  $EXAMPLES$
 *      USE Tests
 *      DBGOTOP()
 *      ? RECNO()
 *      DBGOBOTTOM()
 *      ? RECNO()
 *      USE
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *  $SEEALSO$
 *      BOF(),EOF(),DBSKIP(),DBSEEK(),DBGOBOTTOM()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBGOTOP( void )
{
   if( pCurrArea )
      SELF_GOTOP( ( AREAP ) pCurrArea->pArea );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBGOTOP" );
}

HARBOUR HB___DBLOCATE()
{
   PHB_ITEM pFor, pFor2, pWhile, pNext, pRecord, pRest;
   DBSCOPEINFO pScopeInfo;
   ULONG lNext;
   BOOL bEof, bFor, bWhile;

   if( !pCurrArea )
   {
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBSETFILTER" );
      return;
   }

   memset( &pScopeInfo, 0, sizeof( DBSCOPEINFO ) );
   pFor2 = hb_param( 1, IT_BLOCK );
   pWhile = hb_param( 2, IT_BLOCK );
   pNext = hb_param( 3, IT_NUMERIC );
   pRecord = hb_param( 4, IT_NUMERIC );
   pRest = hb_param( 5, IT_LOGICAL );
   if( !pWhile )
   {
      pWhile = hb_itemPutL( NULL, TRUE );
      pScopeInfo.itmCobWhile = pWhile;
   }
   else
   {
      pRest = hb_itemPutL( NULL, TRUE );
      pScopeInfo.fRest = pRest;
   }
   if( !pFor2 )
      pFor = hb_itemPutL( NULL, TRUE );
   else
   {
      pFor = hb_itemNew( NULL );
      hb_itemCopy( pFor, pFor2 );
   }
   if( !pRest )
   {
      pRest = hb_itemPutL( NULL, FALSE );
      pScopeInfo.fRest = pRest;
   }
   pScopeInfo.itmCobFor = pFor;
   SELF_SETLOCATE( ( AREAP ) pCurrArea->pArea, &pScopeInfo );
   ( ( AREAP ) pCurrArea->pArea )->fFound = FALSE;
   if( pRecord )
   {
      SELF_GOTOID( ( AREAP ) pCurrArea->pArea, pRecord );
      SELF_EOF( ( AREAP ) pCurrArea->pArea, &bEof );
      if( bEof )
         return;
      if( hb_itemType( pWhile ) == IT_BLOCK )
      {
         hb_vmPushSymbol( &hb_symEval );
         hb_vmPush( pWhile );
         hb_vmDo( 0 );
         bWhile = hb_itemGetL( &hb_stack.Return );
      }
      else
         bWhile = hb_itemGetL( pWhile );
      if( hb_itemType( pFor ) == IT_BLOCK )
      {
         hb_vmPushSymbol( &hb_symEval );
         hb_vmPush( pFor );
         hb_vmDo( 0 );
         bFor = hb_itemGetL( &hb_stack.Return );
         ( ( AREAP ) pCurrArea->pArea )->fFound = ( bWhile && bFor );
      }
      else
         ( ( AREAP ) pCurrArea->pArea )->fFound = ( bWhile && hb_itemGetL( pFor ) );
   }
   else if( pNext )
   {
      SELF_EOF( ( AREAP ) pCurrArea->pArea, &bEof );
      lNext = hb_parnl( 3 );
      if( bEof || lNext <= 0 )
         return;
      if( hb_itemType( pWhile ) == IT_BLOCK )
      {
         hb_vmPushSymbol( &hb_symEval );
         hb_vmPush( pWhile );
         hb_vmDo( 0 );
         bWhile = hb_itemGetL( &hb_stack.Return );
      }
      else
         bWhile = hb_itemGetL( pWhile );
      if( hb_itemType( pFor ) == IT_BLOCK )
      {
         hb_vmPushSymbol( &hb_symEval );
         hb_vmPush( pFor );
         hb_vmDo( 0 );
         bFor = hb_itemGetL( &hb_stack.Return );
      }
      else
         bFor = hb_itemGetL( pFor );
      while( !bEof && lNext-- > 0 && bWhile && !bFor )
      {
         SELF_SKIP( ( AREAP ) pCurrArea->pArea, 1 );
         SELF_EOF( ( AREAP ) pCurrArea->pArea, &bEof );
         if( hb_itemType( pWhile ) == IT_BLOCK )
         {
            hb_vmPushSymbol( &hb_symEval );
            hb_vmPush( pWhile );
            hb_vmDo( 0 );
            bWhile = hb_itemGetL( &hb_stack.Return );
         }
         else
            bWhile = hb_itemGetL( pWhile );
         if( hb_itemType( pFor ) == IT_BLOCK )
         {
            hb_vmPushSymbol( &hb_symEval );
            hb_vmPush( pFor );
            hb_vmDo( 0 );
            bFor = hb_itemGetL( &hb_stack.Return );
         }
         else
            bFor = hb_itemGetL( pFor );
      }
      ( ( AREAP ) pCurrArea->pArea )->fFound = bFor;
   }
   else if( hb_itemGetL( pRest ) )
   {
      SELF_EOF( ( AREAP ) pCurrArea->pArea, &bEof );
      if( bEof )
         return;
      if( hb_itemType( pWhile ) == IT_BLOCK )
      {
         hb_vmPushSymbol( &hb_symEval );
         hb_vmPush( pWhile );
         hb_vmDo( 0 );
         bWhile = hb_itemGetL( &hb_stack.Return );
      }
      else
         bWhile = hb_itemGetL( pWhile );
      if( hb_itemType( pFor ) == IT_BLOCK )
      {
         hb_vmPushSymbol( &hb_symEval );
         hb_vmPush( pFor );
         hb_vmDo( 0 );
         bFor = hb_itemGetL( &hb_stack.Return );
      }
      else
         bFor = hb_itemGetL( pFor );
      while( !bEof && bWhile && !bFor )
      {
         SELF_SKIP( ( AREAP ) pCurrArea->pArea, 1 );
         SELF_EOF( ( AREAP ) pCurrArea->pArea, &bEof );
         if( hb_itemType( pWhile ) == IT_BLOCK )
         {
            hb_vmPushSymbol( &hb_symEval );
            hb_vmPush( pWhile );
            hb_vmDo( 0 );
            bWhile = hb_itemGetL( &hb_stack.Return );
         }
         else
            bWhile = hb_itemGetL( pWhile );
         if( hb_itemType( pFor ) == IT_BLOCK )
         {
            hb_vmPushSymbol( &hb_symEval );
            hb_vmPush( pFor );
            hb_vmDo( 0 );
            bFor = hb_itemGetL( &hb_stack.Return );
         }
         else
            bFor = hb_itemGetL( pFor );
      }
      ( ( AREAP ) pCurrArea->pArea )->fFound = bFor;
   }
   else
   {
      SELF_GOTOP( ( AREAP ) pCurrArea->pArea );
      SELF_EOF( ( AREAP ) pCurrArea->pArea, &bEof );
      if( bEof )
         return;
      if( hb_itemType( pFor ) == IT_BLOCK )
      {
         hb_vmPushSymbol( &hb_symEval );
         hb_vmPush( pFor );
         hb_vmDo( 0 );
         bFor = hb_itemGetL( &hb_stack.Return );
      }
      else
         bFor = hb_itemGetL( pFor );
      while( !bEof && !bFor )
      {
         SELF_SKIP( ( AREAP ) pCurrArea->pArea, 1 );
         SELF_EOF( ( AREAP ) pCurrArea->pArea, &bEof );
         if( hb_itemType( pFor ) == IT_BLOCK )
         {
            hb_vmPushSymbol( &hb_symEval );
            hb_vmPush( pFor );
            hb_vmDo( 0 );
            bFor = hb_itemGetL( &hb_stack.Return );
         }
         else
            bFor = hb_itemGetL( pFor );
      }
      ( ( AREAP ) pCurrArea->pArea )->fFound = bFor;
   }
}

HARBOUR HB___DBSETLOCATE( void )
{
   PHB_ITEM pLocate, pFor;
   DBSCOPEINFO pScopeInfo;

   if( pCurrArea )
   {
      pLocate = hb_param( 1, IT_BLOCK );
      if( pLocate )
      {
         pFor = hb_itemNew( NULL );
         hb_itemCopy( pFor, pLocate );
         memset( &pScopeInfo, 0, sizeof( DBSCOPEINFO ) );
         pScopeInfo.itmCobFor = pFor;
         SELF_SETLOCATE( ( AREAP ) pCurrArea->pArea, &pScopeInfo );
      }
   }
}

HARBOUR HB___DBPACK( void )
{
   if( pCurrArea )
   {
      /* Additional feature: __dbPack( [<bBlock>, [<nEvery>] )
         Code Block to execute for every record. */
      ( ( AREAP ) pCurrArea->pArea )->lpExtendInfo->itmEval = hb_param( 1, IT_BLOCK );
      ( ( AREAP ) pCurrArea->pArea )->lpExtendInfo->ulEvery = hb_parnl( 2 );
      if( !( ( AREAP ) pCurrArea->pArea )->lpExtendInfo->ulEvery )
         ( ( AREAP ) pCurrArea->pArea )->lpExtendInfo->ulEvery = 1;
      SELF_PACK( ( AREAP ) pCurrArea->pArea );
      ( ( AREAP ) pCurrArea->pArea )->lpExtendInfo->itmEval = NULL;
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "__DBPACK" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBRECALL()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Recalls a record previousy marked for deletion.
 *  $SYNTAX$
 *      DBRECALL() --> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      DBRECALL() always returns NIL.
 *  $DESCRIPTION$
 *      This function unmarks those records marked for deletion nd reactivates
 *      them in the aliased or selected work area.If a record is DELETED and
 *      the DELETED setting is on, the record will still be visible for a
 *      DBRECALL() provided that the database record pointer has not been
 *      skipped.Once a record marked for deletion with the DELETE setting ON
 *      has been skipped, it no longer canbe brought back with DBRECALL().
 *  $EXAMPLES$
 *      USE Test NEW
 *      DBGOTO(10)
 *      DBDELETE()
 *      ? DELETED()
 *      DBRECALL()
 *      ? DELETED()
 *      USE
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *  $SEEALSO$
 *      DBDELETE()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBRECALL( void )
{
   if( pCurrArea )
      SELF_RECALL( ( AREAP ) pCurrArea->pArea );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBRECALL" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBRLOCK()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      This function locks the record basedon identify
 *  $SYNTAX$
 *      DBRLOCK([<xIdentity>]) --> lSuccess
 *  $ARGUMENTS$
 *      <xIdentity> Record indetifier
 *  $RETURNS$
 *      DBRLOCK() returns a logical true (.T.) if lock was successful
 *  $DESCRIPTION$
 *      This function attempts to lock a record which is indentified
 *      by <xIdentity> in the active data set.If the lock is successful
 *      the function will return a logical true (.T.) value;otherwise
 *      a logical false (.F.) will be returned.If <xIdentity> is not
 *      passed it will be assumed to lock the current active record/data item.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      LOCAL x:=0
 *      USE Tests New
 *      FOR x:=1 to reccount()
 *        IF !DBRLOCK()
 *          DBUNLOCK()
 *        ENDIF
 *      NEXT
 *      USE
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      DBUNLOCK(),DBUNLOCKALL(),FLOCK(),RLOCK()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBRLOCK( void )
{
   DBLOCKINFO pLockInfo;

   pLockInfo.fResult = FALSE;
   if( pCurrArea )
   {
      pLockInfo.itmRecID = hb_parnl( 1 );
      pLockInfo.uiMethod = REC_LOCK;
      SELF_LOCK( ( AREAP ) pCurrArea->pArea, &pLockInfo );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBRLOCK" );

   hb_retl( pLockInfo.fResult );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBRLOCKLIST()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      This function return a list of records in the database work area
 *  $SYNTAX$
 *      DBRLOCKLIST() --> aRecordLocks
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      DBRLOCKLIST() return a array of lock records
 *  $DESCRIPTION$
 *      This function will return an array of locked records in a given
 *      and active work area.If the return array is an empty array
 *      (meaning no elements in it),then there are no locked record in that
 *      work area.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      LOCAL aList:={}
 *      LOCAL x:=0
 *      USE Tests NEW
 *      DBGOTO(10)
 *      RLOCK()
 *      DBGOTO(100)
 *      RLOCK()
 *      aList:=DBRLOCKLIST()
 *      FOR x:=1 TO LEN(aList)
 *         ? aList[x]
 *      NEXT
 *      USE
 *      RETURN NIL
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      RLOCK(),DBRLOCK(),DBRUNLOCK()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBRLOCKLIST( void )
{
   PHB_ITEM pList;

   pList = hb_itemArrayNew( 0 );
   if( pCurrArea )
      SELF_INFO( ( AREAP ) pCurrArea->pArea, DBI_GETLOCKARRAY, pList );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBRLOCKLIST" );

   hb_itemReturn( pList );
   hb_itemRelease( pList );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBRUNLOCK()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Unlocks a record base on its indentifier
 *  $SYNTAX$
 *      DBRUNLOCK([<xIdentity>]) --> NIL
 *  $ARGUMENTS$
 *      <xIdentity> Record indentifier,tipicaly a record number
 *  $RETURNS$
 *      DBRUNLOCK() always returns NIL.
 *  $DESCRIPTION$
 *      This function will attempt to unlock the record specified as
 *      <xIdentity>,which in a .DBF format is the record number.If not
 *      specified,them the current active record/data item will be
 *      unlocked
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      USE Tests New
 *      DBGOTO(10)
 *      IF RLOCK()
 *         ? Tests->ID
 *         DBRUNLOCK()
 *      ENDIF
 *      USE
 *      RETURN NIL
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      RLOCK(),DBRLOCK(),DBRLOCKLIST()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBRUNLOCK( void )
{
   if( pCurrArea )
      SELF_UNLOCK( ( AREAP ) pCurrArea->pArea, hb_parnl( 1 ) );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBRUNLOCK" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBSEEK()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Searches for a value based on an active index.
 *  $SYNTAX$
 *      DBSEEK(<expKey>, [<lSoftSeek>],[<lFindLast>]) --> lFound
 *  $ARGUMENTS$
 *      <expKey> Any expression
 *      <lSoftSeek> Toggle SOFTSEEK condition
 *      <lFindLast> is an optional logical value that set the current
 *   record position to the last record if successful
 *  $RETURNS$
 *      DBSEEK() returns logical true (.T.) if found, otherwise false
 *  $DESCRIPTION$
 *      This function searches for the first record in a database file whose index
 *      key matches <expKey>. If the item is found, the function will return a logical
 *      true (.T.), the value of FOUND() wilI be a logical true (.T.), and the value of
 *      EOF() wilI be a logical false (.F.). If no item is found. then the function will
 *      return a logical false, the value of FOUND( ) will be a logical false (.F.), and
 *      the value of EOF( ) will be a logical true (.T.).
 *      This function always "rewinds" the database pointer and starts the search from
 *      the top of the file.
 *      If the SOFTSEEK flag is on or if <lSoftSeek> is set to a logical true (.T.)
 *      the value of FOUND() wilI be a logical false and EOF() will he a logical
 *      false if there is an item in the index key with a greater value than the key
 *      expression <expKey>; at this point the record pointer will position itself on that
 *      record. However, if there is no greater key in the index,EOF() will return a
 *      logical true (.T.) value. If <lSoftSeek> is not passed, the function will look
 *      to the internal status of SOFTSEEK before performing the operation. The default
 *      of <lSoftSeek> is a logical false (.F.)
 *
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      USE Tests New INDEX Tests
 *      DBGOTO(10)
 *      nId:=Tests->nId
 *      IF Tests->(DBSEEK(nId))
 *        IF RLOCK()
 *           ? Tests->Name
 *           DBRUNLOCK()
 *        ENDIF
 *      ENDIF
 *      USE
 *      RETURN NIL
 *
 *      ACCEPT "Employee name: " TO cName
 *      IF ( Employee->(DBSEEK(cName)) )
 *         Employee->(ViewRecord())
 *      ELSE
 *         ? "Not found"
 *      END
 *
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *      DBSEEK() is  Compatible with CA-Clipper 5.3
 *  $SEEALSO$
 *      DBGOBOTTOM(),DBGOTOP(),DBSKIP(),EOF(),BOF(),FOUND()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBSEEK( void )
{
   PHB_ITEM pKey;
   BOOL bSoftSeek, bFindLast;

   if( pCurrArea )
   {
      if( !ISNIL( 1 ) )
      {
         pKey = hb_param( 1, IT_ANY );
         bSoftSeek = ISLOG( 2 ) ? hb_parl( 2 ) : hb_set.HB_SET_SOFTSEEK;
         bFindLast = ISLOG( 3 ) ? hb_parl( 3 ) : FALSE;
         if( SELF_SEEK( ( AREAP ) pCurrArea->pArea, bSoftSeek, pKey, bFindLast ) == SUCCESS )
         {
            hb_retl( ( ( AREAP ) pCurrArea->pArea )->fFound );
            return;
         }
      }
      else
         hb_errRT_DBCMD( EG_ARG, 1001, NULL, "DBSEEK" );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBSEEK" );
   hb_retl( FALSE );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBSELECTAREA()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Change to another work area
 *  $SYNTAX$
 *      DBSELECTAREA(<xArea>) --> NIL
 *  $ARGUMENTS$
 *      <xArea> Alias or work area
 *  $RETURNS$
 *      DBSELECTAREA() always returns NIL.
 *  $DESCRIPTION$
 *      This function moves the Harbour internal primary focus to the work
 *      area designated by <xArea>. If <xArea> is numeric, them it will
 *      select the numeric work area;if <xArea> is character,then it will
 *      select the work area with the alias name.
 *      DBSELECTAREA(0) will select the next avaliable and unused work area.
 *      Up to 255 work areas are supported.Each work area has its own alias
 *      and record pointer, as well as its own FOUND(),DBFILTER(),DBRSELECT(),
 *      and DBRELATION() function values.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      LOCAL nId
 *      USE Tests NEW INDEX Tests
 *      USE Tests1 NEW INDEX Tests1
 *      DBSELECTAREA(1)
 *      nId:=Tests->Id
 *      DBSELECTAREA(2)
 *      IF DBSEEK(nId)
 *         ? Tests1->cName
 *      ENDIF
 *      DBCLOSEALL()
 *      RETURN NIL
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      This function is CA-CLIPPER compatible.
 *  $SEEALSO$
 *      DBUSEAREA(),SELECT()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBSELECTAREA( void )
{
   USHORT uiNewArea;
   char * szAlias;
   LPAREANODE pAreaNode;

   if( ISCHAR( 1 ) )
   {
      ULONG ulLen;

      szAlias = hb_parc( 1 );

      ulLen = strlen( szAlias );

      if( ulLen >= 1 && szAlias[ 0 ] >= '0' && szAlias[ 0 ] <= '9' )
         uiNewArea = atoi( szAlias );
      else if( ulLen == 1 && toupper( szAlias[ 0 ] ) >= 'A' && toupper( szAlias[ 0 ] ) <= 'K' )
         uiNewArea = toupper( szAlias[ 0 ] ) - 'A' + 1;
      else
      {
         if( ( uiNewArea = hb_rddSelect( szAlias ) ) == 0 )
         {
            hb_errRT_BASE( EG_NOALIAS, 1002, NULL, szAlias );
            return;
         }
      }
   }
   else
      uiNewArea = hb_parni( 1 );

   if( uiNewArea == 0 )
      hb_rddSelectFirstAvailable();
   else
      uiCurrArea = uiNewArea;

   pAreaNode = pWorkAreas;
   while( pAreaNode )
   {
      if( ( ( AREAP ) pAreaNode->pArea )->uiArea == uiCurrArea )
      {
         pCurrArea = pAreaNode; /* Select a valid WorkArea */
         return;
      }
      pAreaNode = pAreaNode->pNext;
   }
   pCurrArea = NULL; /* Selected WorkArea is closed */
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBSETDRIVER()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Establishes the name of replaceable daabas driver for a selected work area
 *  $SYNTAX$
 *      DBSETDRIVER([<cDriver>]) --> cCurrentDriver
 *  $ARGUMENTS$
 *      <cDriver> Optional database driver name
 *  $RETURNS$
 *      DBSETDRIVER() returns the name of active driver
 *  $DESCRIPTION$
 *      This function returns the name of the current database driver for the
 *      selected work area. The default will be "DBFNTX". If specified,<cDriver>
 *      contains the name of the database driver that should be used to activate
 *      and manage the work area.If the specified driver is not avaliable,this
 *      function will have no effect.
 *  $EXAMPLES$
 *      DBSETDRIVER("ADS")
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      This function is CA-Clipper compatible
 *  $SEEALSO$
 *      DBUSEAREA()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBSETDRIVER( void )
{
   char * szNewDriver;
   USHORT uiLen;

   hb_rddCheck();
   hb_retc( szDefDriver );
   szNewDriver = hb_parc( 1 );
   if( ( uiLen = strlen( szNewDriver ) ) > 0 )
   {
      hb_strUpper( szNewDriver, uiLen ); /* TOFIX: Direct access to hb_parc() buffer ! */

      if( !hb_rddFindNode( szNewDriver, NULL ) )
      {
         hb_errRT_DBCMD( EG_ARG, 1015, NULL, "DBSETDRIVER" );
         return;
      }

      szDefDriver = ( char * ) hb_xrealloc( szDefDriver, uiLen + 1 );
      strcpy( szDefDriver, szNewDriver );
   }
}

HARBOUR HB___DBSETFOUND( void )
{
   PHB_ITEM pFound;

   if( pCurrArea )
   {
      pFound = hb_param( 1, IT_LOGICAL );
      if( pFound )
         ( ( AREAP ) pCurrArea->pArea )->fFound = hb_itemGetL( pFound );
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBSKIP()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Moves the record pointer in the selected work area.
 *  $SYNTAX$
 *      DBSKIP([<nRecords>]) --> NIL
 *  $ARGUMENTS$
 *      <nRecords> Numbers of records to move record pointer.
 *  $RETURNS$
 *      DBSKIP() always returns NIL.
 *  $DESCRIPTION$
 *      This function moves the record pointer <nRecords> in the selected or
 *      aliased work area.The default value for <nRecords> will be 1.
 *      A DBSKIP(0) will flush and refresh the internal database bufer and make
 *      any changes made to the record visible without moving the record pointer
 *      in either direction.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      USE Tests NEW
 *      DBGOTOP()
 *      WHILE !EOF()
 *        ? Tests->Id,Tests->Name
 *        DBSKIP()
 *      ENDDO
 *      USE
 *      RETURN NIL
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      This function is CA-CLIPPER compatible
 *  $SEEALSO$
 *    BOF(),DBGOBOTTOM(),DBGOTOP(),DBSEEK(),EOF()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBSKIP( void )
{
   LONG lToSkip = 1;

   if( pCurrArea )
   {
      if( ISNUM( 1 ) )
         lToSkip = hb_parnl( 1 );
      SELF_SKIP( ( AREAP ) pCurrArea->pArea, lToSkip );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBSKIP" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBSETFILTER()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Establishes a filter condition for a work area.
 *  $SYNTAX$
 *      DBSETFILTER(<bCondition>, [<cCondition>]) --> NIL
 *  $ARGUMENTS$
 *      <bCondition> Code block expression for filtered evaluation.
 *      <cCondition> Optional character expression of code block.
 *  $RETURNS$
 *      DBSETFILTER() always returns NIL.
 *  $DESCRIPTION$
 *      This function masks a database so that only those records that meet the
 *      condition prescribed by the expression in the code block <bCondition>
 *      and literally expressed as <cCondition> are visible.
 *      If <cCondition> is not passed to this function,then the DBFILTER()
 *      function will return an empty string showing no filter in that work area
 *      which in fact,would be not correct.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      USE Tests NEW
 *      DBSETFILTER({|| Tests->Id <100},"Tests->Id <100")
 *      DBGOTOP()
 *
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      DBFILTER(),DBCLEARFILTER()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBSETFILTER( void )
{
   PHB_ITEM pBlock, pText;
   DBFILTERINFO pFilterInfo;

   if( pCurrArea )
   {
      pBlock = hb_param( 1, IT_BLOCK );
      if( pBlock )
      {
         pText = hb_param( 2, IT_STRING );
         pFilterInfo.itmCobExpr = pBlock;
         if( pText )
            pFilterInfo.abFilterText = pText;
         else
            pFilterInfo.abFilterText = hb_itemPutC( NULL, "" );
         SELF_SETFILTER( ( AREAP ) pCurrArea->pArea, &pFilterInfo );
         if( !pText )
            hb_itemRelease( pFilterInfo.abFilterText );
      }
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBSETFILTER" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBSTRUCT()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Creates a multidimensional array of a database structure.
 *  $SYNTAX$
 *      DBSTRUCT() --> aStruct
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      DBSTRUCT() returns an array pointer to database structure
 *  $DESCRIPTION$
 *      This function returns a multidimensional array.This array has array
 *      pointers to other arrays,each of which contains the characteristic
 *      of a field in the active work area.The lenght of this array is based
 *      in the number of fields in that particular work area.In other words,
 *      LEN(DBSTRUCT()) is equal to the value obtained from FCOUNT().
 *      Each subscript position
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      LOCAL aStru,x
 *      USE Tests NEW
 *      aStru:=DBSTRUCT()
 *      FOR x:=1 TO LEN(aStru)
 *        ? aStru[x,1]
 *      NEXT
 *      USE
 *      RETURN NIL
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      AFIELDS()
 *  $INCLUDE$
 *      DbStruct.ch
 *  $END$
 */

HARBOUR HB_DBSTRUCT( void )
{
   PHB_ITEM pItem, pData;
   USHORT uiFields, uiCount;

   hb_arrayNew( &hb_stack.Return, 0 );

   if( pCurrArea )
   {
      SELF_FIELDCOUNT( ( AREAP ) pCurrArea->pArea, &uiFields );
      pData = hb_itemNew( NULL );
      pItem = hb_itemNew( NULL );
      for( uiCount = 1; uiCount <= uiFields; uiCount++ )
      {
         hb_arrayNew( pItem, 4 );
         SELF_FIELDINFO( ( AREAP ) pCurrArea->pArea, uiCount, DBS_NAME, pData );
         hb_arraySet( pItem, 1, pData );
         SELF_FIELDINFO( ( AREAP ) pCurrArea->pArea, uiCount, DBS_TYPE, pData );
         hb_arraySet( pItem, 2, pData );
         SELF_FIELDINFO( ( AREAP ) pCurrArea->pArea, uiCount, DBS_LEN, pData );
         hb_arraySet( pItem, 3, pData );
         SELF_FIELDINFO( ( AREAP ) pCurrArea->pArea, uiCount, DBS_DEC, pData );
         hb_arraySet( pItem, 4, pData );
         hb_arrayAdd( &hb_stack.Return, pItem );
      }
      hb_itemRelease( pItem );
      hb_itemRelease( pData );
   }
}

HARBOUR HB_DBTABLEEXT( void )
{
   LPRDDNODE pRddNode;
   AREAP pTempArea;
   USHORT uiSize, uiRddID;
   PHB_ITEM pItem;

   if( !pCurrArea )
   {
      hb_rddCheck();
      uiRddID = 0;
      pRddNode = hb_rddFindNode( szDefDriver, &uiRddID );
      if( !pRddNode )
      {
         hb_retc( "" );
         return;
      }
      uiSize = sizeof( AREA );    /* Default Size Area */
      pTempArea = ( AREAP ) hb_xgrab( uiSize );
      memset( pTempArea, 0, uiSize );
      pTempArea->lprfsHost = &pRddNode->pTable;

      /* Need more space? */
      SELF_STRUCTSIZE( ( AREAP ) pTempArea, &uiSize );
      if( uiSize > sizeof( AREA ) )   /* Size of Area changed */
         pTempArea = ( AREAP ) hb_xrealloc( pTempArea, uiSize );

      pRddNode->uiAreaSize = uiSize; /* Update the size of WorkArea */
      pTempArea->rddID = uiRddID;

      if( SELF_NEW( ( AREAP ) pTempArea ) == FAILURE )
         hb_retc( "" );
      else
      {
         pItem = hb_itemPutC( NULL, "" );
         SELF_INFO( ( AREAP ) pTempArea, DBI_TABLEEXT, pItem );
         hb_retc( pItem->item.asString.value );
         hb_itemRelease( pItem );
         SELF_RELEASE( ( AREAP ) pTempArea );
      }
      hb_xfree( pTempArea );
   }
   else
   {
      pItem = hb_itemPutC( NULL, "" );
      SELF_INFO( ( AREAP ) pCurrArea->pArea, DBI_TABLEEXT, pItem );
      hb_retc( pItem->item.asString.value );
      hb_itemRelease( pItem );
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBUNLOCK()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Unlock a record or release a file lock
 *  $SYNTAX$
 *      DBUNLOCK() --> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      DBUNLOCK() always returns NIL.
 *  $DESCRIPTION$
 *      This function releases the file or record lock in the currently selected
 *      or aliased work area.It will not unlock an associated lock in a related data-
 *      bases.
 *  $EXAMPLES$
 *      nId:=10
 *      USE TestId INDEX TestId NEW
 *      IF TestId->(DBSEEK(nId))
 *         IF TestId->(RLOCK())
 *            DBDELETE()
 *         ELSE
 *             DBUNLOCK()
 *         ENDIF
 *      ENDIF
 *      USE
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      This function is CA-Clipper compatible.
 *  $SEEALSO$
 *      DBUNLOCKALL(),FLOCK(),RLOCK()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBUNLOCK( void )
{
   if( pCurrArea )
      SELF_RAWLOCK( ( AREAP ) pCurrArea->pArea, FILE_UNLOCK, 0 );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "DBUNLOCK" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBUNLOCKALL()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Unlocks all records and releases all file locks in all work areas.
 *  $SYNTAX$
 *      DBUNLOCKALL() --> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      DBUNLOCKALL() always returns NIL.
 *  $DESCRIPTION$
 *      This function will remove all file and record locks in all work area.
 *  $EXAMPLES$
 *      nId:=10
 *      USE Tests INDEX TestId NEW
 *      USE Tests1 INDEX Tests NEW
 *      IF TestId->(DBSEEK(nId))
 *         IF TestId->(RLOCK())
 *            DBDELETE()
 *         ELSE
 *            DBUNLOCK()
 *         ENDIF
 *      ELSE
 *         DBUNLOCKALL()
 *      ENDIF
 *      USE
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      DBUNLOCK(),FLOCK(),RLOCK()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBUNLOCKALL( void )
{
   LPAREANODE pTempArea;

   pTempArea = pWorkAreas;
   while( pTempArea )
   {
      SELF_RAWLOCK( ( AREAP ) pTempArea->pArea, FILE_UNLOCK, 0 );
      pTempArea = pTempArea->pNext;
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *      DBUSEAREA()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Opens a work area and uses a database file.
 *  $SYNTAX$
 *      DBUSEAREA( [<lNewArea>], [<cDriver>], <cName>, [<xcAlias>],
 *      [<lShared>], [<lReadonly>]) --> NIL
 *  $ARGUMENTS$
 *     <lNewArea>  A optional logical expression for the new work area
 *     <cDriver>   Database driver name
 *     <cName>     File Name
 *     <xcAlias>   Alias name
 *     <lShared>   Shared/exclusive status flag
 *     <lReadonly> Read-write status flag.
 *  $RETURNS$
 *      DBUSEAREA() always returns NIL.
 *  $DESCRIPTION$
 *      This function opens an existing database named <cName> in the current
 *      work area. If <lNewArea> is set to a logical true (.T.) value, then
 *      the database <cName> will be opened in the next available and unused
 *      work area. The default value of <lNewArea> is a logical false (.F.).
 *      If used, <cDriver> is the name of the database driver associated with
 *      the file <cName> that is opened. The default for this will be the value
 *      of DBSETDRlVER().
 *      IF used, <xcAlias> contains the alias name for that work area, If not
 *      specified, the root name of the database specified in <cName> will be
 *      used.
 *      If <lShared> is set to a logical true (.T.) value, the database that
 *      is specified in <cName> will be opened by the user EXCLUSIVELY. Thus
 *      locking it from all other nodes or users on the network. If <lShared> is
 *      set to a logical false (.F.) value, then the database will be in SHARED
 *      mode. If <lShared> is not passed, then the function will turn to the
 *      internal setting of SET EXCLUSIVE to determine a setting.
 *      If <lReadOnly> is specified, the file will be set to READ ONLY mode.
 *      If it is not specified, the file will he opened in normal read-write
 *      mode.
 *  $EXAMPLES$
 *      DBUSEAREA(.T.,,"Tests")
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      This function is CA-Clipper compliant
 *  $SEEALSO$
 *      DBCLOSEAREA(),DBSETDRIVER(),SELECT(),SET()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DBUSEAREA( void )
{
   char * szDriver, * szFileName;
   LPRDDNODE pRddNode;
   LPAREANODE pAreaNode;
   USHORT uiSize, uiRddID, uiLen;
   ULONG ulLen;
   DBOPENINFO pInfo;
   PHB_FNAME pFileName;
   PHB_ITEM pFileExt;
   char szDriverBuffer[ HARBOUR_MAX_RDD_DRIVERNAME_LENGTH + 1 ];
   char szAlias[ HARBOUR_MAX_RDD_ALIAS_LENGTH + 1 ];

   bNetError = FALSE;

   if( hb_parl( 1 ) )
      hb_rddSelectFirstAvailable();
   else if( pCurrArea )  /* If current WorkArea is in use then close it */
   {
      SELF_CLOSE( ( AREAP ) pCurrArea->pArea );
      SELF_RELEASE( ( AREAP ) pCurrArea->pArea );

      if( pWorkAreas == pCurrArea )
      {
         pWorkAreas = pCurrArea->pNext;
         if( pWorkAreas )
            pWorkAreas->pPrev = NULL;
      }
      else
      {
         if( pCurrArea->pPrev )
            pCurrArea->pPrev->pNext = pCurrArea->pNext;
         if( pCurrArea->pNext )
            pCurrArea->pNext->pPrev = pCurrArea->pPrev;
      }

      hb_xfree( pCurrArea->pArea );
      hb_xfree( pCurrArea );
      pCurrArea = NULL;
   }

   hb_rddCheck();
   uiLen = hb_parclen( 2 );
   if( uiLen > 0 )
   {
      if( uiLen > HARBOUR_MAX_RDD_DRIVERNAME_LENGTH )
         uiLen = HARBOUR_MAX_RDD_DRIVERNAME_LENGTH;
      hb_strncpyUpper( szDriverBuffer, hb_parc( 2 ), uiLen );
      szDriver = szDriverBuffer;
   }
   else
      szDriver = szDefDriver;

   uiRddID = 0;
   pRddNode = hb_rddFindNode( szDriver, &uiRddID );
   if( !pRddNode )
   {
      hb_errRT_DBCMD( EG_ARG, 1015, NULL, "DBUSEAREA" );
      return;
   }

   szFileName = hb_parc( 3 );
   if( strlen( szFileName ) == 0 )
   {
      hb_errRT_DBCMD( EG_ARG, 1005, NULL, "DBUSEAREA" );
      return;
   }

   pFileName = hb_fsFNameSplit( szFileName );
   strncpy( szAlias, hb_parc( 4 ), HARBOUR_MAX_RDD_ALIAS_LENGTH );
   ulLen = strlen( szAlias );
   if( ulLen == 0 )
      strncpy( szAlias, pFileName->szName, HARBOUR_MAX_RDD_ALIAS_LENGTH );
   else if( ulLen == 1 )
   {
      /* Alias with a single letter. Only are valid 'L' and > 'M' */
      if( toupper( szAlias[ 0 ] ) < 'N' && toupper( szAlias[ 0 ] ) != 'L' )
      {
         hb_xfree( pFileName );
         hb_errRT_DBCMD( EG_DUPALIAS, 1011, NULL, "DBUSEAREA" );
         return;
      }
   }

   /* Create a new WorkArea node */

   pCurrArea = ( LPAREANODE ) hb_xgrab( sizeof( AREANODE ) );

   if( pRddNode->uiAreaSize == 0 ) /* Calculate the size of WorkArea */
   {
      uiSize = sizeof( AREA );    /* Default Size Area */
      pCurrArea->pArea = ( AREAP ) hb_xgrab( uiSize );
      memset( pCurrArea->pArea, 0, uiSize );
      ( ( AREAP ) pCurrArea->pArea )->lprfsHost = &pRddNode->pTable;

      /* Need more space? */
      SELF_STRUCTSIZE( ( AREAP ) pCurrArea->pArea, &uiSize );
      if( uiSize > sizeof( AREA ) )   /* Size of Area changed */
         pCurrArea->pArea = ( AREAP ) hb_xrealloc( pCurrArea->pArea, uiSize );

      pRddNode->uiAreaSize = uiSize;  /* Update the size of WorkArea */
   }
   else
   {
      pCurrArea->pArea = ( AREAP ) hb_xgrab( pRddNode->uiAreaSize );
      memset( pCurrArea->pArea, 0, pRddNode->uiAreaSize );
      ( ( AREAP ) pCurrArea->pArea )->lprfsHost = &pRddNode->pTable;
   }

   ( ( AREAP ) pCurrArea->pArea )->rddID = uiRddID;

   pCurrArea->pPrev = NULL;
   pCurrArea->pNext = NULL;

   SELF_NEW( ( AREAP ) pCurrArea->pArea );

   szFileName = ( char * ) hb_xgrab( _POSIX_PATH_MAX + 3 );
   strcpy( szFileName, hb_parc( 3 ) );
   if( !pFileName->szExtension )
   {
      pFileExt = hb_itemPutC( NULL, "" );
      SELF_INFO( ( AREAP ) pCurrArea->pArea, DBI_TABLEEXT, pFileExt );
      strcat( szFileName, pFileExt->item.asString.value );
      hb_itemRelease( pFileExt );
   }
   hb_xfree( pFileName );
   pInfo.uiArea = uiCurrArea;
   pInfo.abName = ( BYTE * ) szFileName;
   pInfo.atomAlias = ( BYTE * ) szAlias;
   pInfo.fShared = ISLOG( 5 ) ? hb_parl( 5 ) : !hb_set.HB_SET_EXCLUSIVE;
   pInfo.fReadonly = ISLOG( 6 ) ? hb_parl( 6 ) : FALSE;

   ( ( AREAP ) pCurrArea->pArea )->uiArea = uiCurrArea;

   /* Insert the new WorkArea node */

   if( !pWorkAreas )
      pWorkAreas = pCurrArea;  /* The new WorkArea node is the first */
   else
   {
      pAreaNode = pWorkAreas;
      while( pAreaNode )
      {
         if( ( ( AREAP ) pAreaNode->pArea )->uiArea > uiCurrArea )
         {
            /* Insert the new WorkArea node */
            pCurrArea->pPrev = pAreaNode->pPrev;
            pCurrArea->pNext = pAreaNode;
            pAreaNode->pPrev = pCurrArea;
            if( pCurrArea->pPrev )
               pCurrArea->pPrev->pNext = pCurrArea;
            else
               pWorkAreas = pCurrArea;
            break;
         }
         if( pAreaNode->pNext )
            pAreaNode = pAreaNode->pNext;
         else
         {
            /* Append the new WorkArea node */
            pAreaNode->pNext = pCurrArea;
            pCurrArea->pPrev = pAreaNode;
            break;
         }
      }
   }

   ( ( AREAP ) pCurrArea->pArea )->lpDataInfo->szFileName = szFileName;
   if( SELF_OPEN( ( AREAP ) pCurrArea->pArea, &pInfo ) == FAILURE )
   {
      SELF_RELEASE( ( AREAP ) pCurrArea->pArea );

      if( pWorkAreas == pCurrArea )
      {
         pWorkAreas = pCurrArea->pNext;
         if( pWorkAreas )
            pWorkAreas->pPrev = NULL;
      }
      else
      {
         if( pCurrArea->pPrev )
            pCurrArea->pPrev->pNext = pCurrArea->pNext;
         if( pCurrArea->pNext )
            pCurrArea->pNext->pPrev = pCurrArea->pPrev;
      }

      hb_xfree( pCurrArea->pArea );
      hb_xfree( pCurrArea );
      pCurrArea = NULL;
      return;
   }
   SELF_RECCOUNT( ( AREAP ) pCurrArea->pArea, &ulLen );
   ( ( AREAP ) pCurrArea->pArea )->lpExtendInfo->ulRecCount = ulLen;
}

/*  $DOC$
 *  $FUNCNAME$
 *      __DBZAP()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Remove all records from the current database file
 *  $SYNTAX$
 *      __DbZap()  -> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      __DbZap()   will always return nil
 *  $DESCRIPTION$
 *      __DbZap*( is a database command that permanently removes all records from
 *   files open in the current work area.  This includes the current database
 *   file, index files, and associated memo file.  Disk space previously
 *   occupied by the ZAPped files is released to the operating system.
 *   __DbZap() performs the same operation as DELETE ALL followed by PACK but is
 *   almost  instantaneous.
 *
 *   To ZAP in a network environment, the current database file must be USEd
 *   EXCLUSIVEly.
 *
 *  $EXAMPLES$
 *      This example demonstrates a typical ZAP operation in a network
 *      environment:
 *
 *      USE Sales EXCLUSIVE NEW
 *      IF !NETERR()
 *         SET INDEX TO Sales, Branch, Salesman
 *         __dbZAP()
 *         CLOSE Sales
 *      ELSE
 *         ? "Zap operation failed"
 *         BREAK
 *      ENDIF
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *
 *  $INCLUDE$
 *
 *  $END$
 */

/*  $DOC$
 *  $COMMANDNAME$
 *      ZAP
 *  $CATEGORY$
 *      Command
 *  $ONELINER$
 *      Remove all records from the current database file
 *  $SYNTAX$
 *      ZAP
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *
 *  $DESCRIPTION$
 *      This command removes all of the records from the database in the
 *      current work area.This operation also updates any index file in
 *      use at the time of this operation.In addition, this command removes
 *      all items within an associated memo file.
 *      In a network enviroment,any file that is about to be ZAPped must
 *      be used exclusively.
 *  $EXAMPLES$
 *      USE Tests NEW index Tests
 *      ZAP
 *      USE
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      DELETE,PACK,USE
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB___DBZAP( void )
{
   if( pCurrArea )
      SELF_ZAP( ( AREAP ) pCurrArea->pArea );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "__DBZAP" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      DELETED()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Tests the record's deletion flag.
 *  $SYNTAX$
 *      DELETED() --> lDeleted
 *  $ARGUMENTS$
 *  $RETURNS$
 *      DELETED() return a logical true (.T.) or false (.F.).
 *  $DESCRIPTION$
 *      This function returns a logical true (.T.) is the current record in the
 *      selected or designated work area ha ben marked for deletion.If not, the
 *      function will return a logical false (.F.).
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      USE Test New
 *      DBGOTO()
 *      DBDELETE()
 *      ? "Is Record Deleted",Test->(DELETED())
 *      DBRECALL()
 *      USE
 *      RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      This function is CA-Clipper compliant
 *  $SEEALSO$
 *      DBDELETE()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_DELETED( void )
{
   BOOL bDeleted = FALSE;

   if( pCurrArea )
      SELF_DELETED( ( AREAP ) pCurrArea->pArea, &bDeleted );
   hb_retl( bDeleted );
}

/*  $DOC$
 *  $FUNCNAME$
 *      EOF()
 *  $CATEGORY$
 *      DATA BASE
 *  $ONELINER$
 *      Test for end-of-file condition.
 *  $SYNTAX$
 *      EOF() --> <lEnd>
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      Logical true (.T.) or false (.F.)
 *  $DESCRIPTION$
 *      This function determines if the end-of-file marker has been reached.
 *      If it has, the function will return a logical true (.T.); otherwise
 *      a logical false (.F.) will be returnd
 *  $EXAMPLES$
 *      FUNCTION Main()
 *        USE Tests NEW
 *        DBGOTOP()
 *        ? "Is Eof()",EOF()
 *        DBGOBOTTOM()
 *        ? "Is Eof()",EOF()
 *        USE
 *     RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      EOF() is fully CA-Clipper compliant.
 *  $SEEALSO$
 *    BOF(),FOUND(),LASTREC()
 *  $END$
 */

HARBOUR HB_EOF( void )
{
   BOOL bEof = TRUE;

   if( pCurrArea )
      SELF_EOF( ( AREAP ) pCurrArea->pArea, &bEof );
   hb_retl( bEof );
}

/*  $DOC$
 *  $FUNCNAME$
 *      FCOUNT()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Counts the number of fields in an active database.
 *  $SYNTAX$
 *      FCOUNT() --> nFields
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      FCOUNT() Return the number of fields
 *  $DESCRIPTION$
 *      This function returns the number of fields in the current or designated
 *      work area.If no database is open in this work area, the function will
 *      return 0.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *        USE Tests NEW
 *        ? "This database have ",Tests->(FCOUNT()),"Fields"
 *        USE
 *      RETURN Nil
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      This function is CA-Clipper compliant
 *  $SEEALSO$
 *      FIELDNAME(),TYPE()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_FCOUNT( void )
{
   USHORT uiFields = 0;

   if( pCurrArea )
      SELF_FIELDCOUNT( ( AREAP ) pCurrArea->pArea, &uiFields );
   hb_retni( uiFields );
}

/*  $DOC$
 *  $FUNCNAME$
 *      FIELDGET()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Obtains the value  of a specified field
 *  $SYNTAX$
 *      FIELDGET(<nField>) --> ValueField
 *  $ARGUMENTS$
 *      <nField> Is the numeric field position
 *  $RETURNS$
 *      FIELDGET() Any expression
 *  $DESCRIPTION$
 *      This function returns the value of the field at the <nField>th location
 *      in the selected or designed work area.If the value in <nField> does not
 *      correspond to n avaliable field position in this work area, the function
 *      will return a NIL data type.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      USE Test NEW
 *      ? Test->(FieldGet(1))
 *      USE
 *      RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      This function is CA-Clipper Compliant.
 *  $SEEALSO$
 *      FIELDPUT()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_FIELDGET( void )
{
   PHB_ITEM pItem;
   USHORT uiField;

   pItem = hb_itemNew( NULL );
   uiField = hb_parni( 1 );

   if( pCurrArea && uiField )
      SELF_GETVALUE( ( AREAP ) pCurrArea->pArea, uiField, pItem );

   hb_itemReturn( pItem );
   hb_itemRelease( pItem );
}

/*  $DOC$
 *  $FUNCNAME$
 *      FIELDNAME()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the name of a field at a numeric field location.
 *  $SYNTAX$
 *      FIELDNAME/FIELD(<nPosition>) --> cFieldName
 *  $ARGUMENTS$
 *      <nPosition> Field order in the database.
 *  $RETURNS$
 *      FIELDNAME() returns the field name.
 *  $DESCRIPTION$
 *      This function return the name of the field at the <nPosition>th position.
 *      If the numeric value passed to this function does not correspond to an
 *      existing field in the designated or selected work area,this function
 *      will return a NULL byte.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *        LOCAL x
 *        USE Tests NEW
 *        FOR x := 1 to Tests->(FCOUNT())
 *          ? "Field Name",FieldName(x)
 *        NEXT
 *        USE
 *      RETURN Nil
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      This function is CA-Clipper compatible.
 *  $SEEALSO$
 *    DBSTRUCT(),FCOUNT(),LEN(),VALTYPE()
 *
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_FIELDNAME( void )
{
   USHORT uiFields, uiIndex;
   char * szName;

   if( pCurrArea )
   {
      uiIndex = hb_parni( 1 );
      if( SELF_FIELDCOUNT( ( AREAP ) pCurrArea->pArea, &uiFields ) == SUCCESS )
      {
         if( uiIndex > 0 && uiIndex <= uiFields )
         {
            szName = ( char * ) hb_xgrab( HARBOUR_MAX_RDD_FIELDNAME_LENGTH + 1 );
            SELF_FIELDNAME( ( AREAP ) pCurrArea->pArea, hb_parni( 1 ), szName );
            hb_retc( szName );
            hb_xfree( szName );
            return;
         }
         hb_errRT_DBCMD( EG_ARG, 1009, NULL, "FIELDNAME" );
      }
   }
   hb_retc( "" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      FIELDPOS()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the ordinal position of a field.
 *  $SYNTAX$
 *      FIELDPOS(<cFieldName>) --> nFieldPos
 *  $ARGUMENTS$
 *      <cFieldName> Name of a field.
 *  $RETURNS$
 *      FIELDPOS() returns the ordinal position of the field.
 *  $DESCRIPTION$
 *      This function return the ordinal position of the specified field <cField>
 *      in the current or aliased work areaIf there isn't  field under the name
 *      of <cField> or of no database is open in the selected work area, the func-
 *      tion will return a 0.
 *  $EXAMPLES$
 *      FUNCTION Main()
 *      USE Test NEW
 *      ? Test->(FIELDPOS("ID"))
 *      USE
 *      RETURN NIL
 *  $TESTS$
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *      This function is CA-Clipper compliant.
 *  $SEEALSO$
 *      FIELDGET(),FIELDPUT()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_FIELDPOS( void )
{
   USHORT uiCount;
   LPFIELD pField;

   if( pCurrArea )
   {
      char szName[ HARBOUR_MAX_RDD_FIELDNAME_LENGTH ];

      hb_strncpyUpper( szName, hb_parc( 1 ), hb_parclen( 1 ) );
      uiCount = 0;
      pField = ( ( AREAP ) pCurrArea->pArea )->lpFields;
      while( pField )
      {
         ++uiCount;
         if( strcmp( szName, ( ( PHB_DYNS ) pField->sym )->pSymbol->szName ) == 0 )
         {
            hb_retni( uiCount );
            return;
         }
         pField = pField->lpfNext;
      }
   }
   hb_retni( 0 );
}

/*  $DOC$
 *  $FUNCNAME$
 *      FIELDPUT()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Set the value of a field variable
 *  $SYNTAX$
 *      FIELDPUT(<nField>, <expAssign>) --> ValueAssigned
 *  $ARGUMENTS$
 *    <nField> is the ordinal position of the field in the current
 *   database file.
 *
 *      <expAssign> is the value to assign to the given field.  The data
 *   type of this expression must match the data type of the designated field
 *   variable.
 *  $RETURNS$
 *      FIELDPUT() returns the value assigned to the designated field.  If
 *   <nField> does not correspond to the position of any field in the current
 *   database file, FIELDPUT() returns NIL.
 *  $DESCRIPTION$
 *      FIELDPUT() is a database function that assigns <expAssign> to the field
 *   at ordinal position <nField> in the current work area.  This function
 *   allows you to set the value of a field using its position within the
 *   database file structure rather than its field name.  Within generic
 *   database service functions this allows, among other things, the setting
 *   of field values without use of the macro operator.
 *  $EXAMPLES$
 *      This example compares FIELDPUT() to functionally equivalent
 *      code that uses the macro operator to set the value of a field:
 *
 *      // Using macro operator
 *      FName := FIELD(nField)         // Get field name
 *      FIELD->&FName := FVal          // Set field value
 *      // Using FIELDPUT()
 *      FIELDPUT(nField, FVal)         // Set field value
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      FIELDGET()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_FIELDPUT( void )
{
   USHORT uiIndex;
   PHB_ITEM pItem;

   uiIndex = hb_parni( 1 );
   if( pCurrArea && uiIndex )
   {
      pItem = hb_param( 2, IT_ANY );
      if( SELF_PUTVALUE( ( AREAP ) pCurrArea->pArea, uiIndex, pItem ) == SUCCESS )
      {
         hb_itemReturn( pItem );
         return;
      }
   }
   hb_ret();
}

/*  $DOC$
 *  $FUNCNAME$
 *      FLOCK()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Lock an open and shared database file
 *  $SYNTAX$
 *      FLOCK() --> lSuccess
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      FLOCK() returns true (.T.) if an attempt to lock a database file in USE
 *   in the current work area succeeds; otherwise, it returns false (.F.).
 *   For more information on file locking, refer to the Network Programming
 *   chapter in the Programming and Utilities guide.
 *
 *  $DESCRIPTION$
 *      FLOCK() is a database function used in network environments to lock an
 *   open and shared database file, preventing other users from updating the
 *   file until the lock is released.  Records in the locked file are
 *   accessible for read-only operations.
 *
 *      FLOCK() is related to USE...EXCLUSIVE and RLOCK().  USE...EXCLUSIVE
 *   opens a database file so that no other user can open the same file at
 *   the same time and is the most restrictive locking mechanism in
 *   HARBOUR.  RLOCK() is the least restrictive and attempts to place an
 *   update lock on a shared record, precluding other users from updating the
 *   current record.  FLOCK() falls in the middle.
 *
 *    FLOCK() is used for operations that access the entire database file.
 *   Typically, these are commands that update the file with a scope or a
 *   condition such as DELETE or REPLACE ALL.  The following is a list of
 *   such commands:
 *
 *   Commands that require an FLOCK()
 *   컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
 *   Command                       Mode
 *   컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
 *   APPEND FROM                   FLOCK() or USE...EXCLUSIVE
 *   DELETE (multiple records)     FLOCK() or USE...EXCLUSIVE
 *   RECALL (multiple records)     FLOCK() or USE...EXCLUSIVE
 *   REPLACE (multiple records)    FLOCK() or USE...EXCLUSIVE
 *   UPDATE ON                     FLOCK() or USE...EXCLUSIVE
 *   컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
 *
 *      For each invocation of FLOCK(), there is one attempt to lock the
 *   database file, and the result is returned as a logical value.  A file
 *   lock fails if another user currently has a file or record lock for the
 *   same database file or EXCLUSIVE USE of the database file.  If FLOCK() is
 *   successful, the file lock remains in place until you UNLOCK, CLOSE the
 *   DATABASE, or RLOCK().
 *
 *      By default, FLOCK() operates on the currently selected work area as
 *   shown in the example below.
 *
 *      Notes
 *
 *      SET RELATION: HARBOUR does not automatically lock all work
 *      areas in the relation chain when you lock the current work area, and
 *      an UNLOCK has no effect on related work areas.
 *  $EXAMPLES$
 *      This example uses FLOCK() for a batch update of prices in
 *      Inventory.dbf:
 *
 *      USE Inventory NEW
 *      IF FLOCK()
 *         REPLACE ALL Inventory->Price WITH ;
 *               Inventory->Price * 1.1
 *      ELSE
 *         ? "File not available"
 *      ENDIF
 *
 *      This example uses an aliased expression to attempt a file lock
 *      in an unselected work area:
 *
 *      USE Sales NEW
 *      USE Customer NEW
 *      //
 *      IF !Sales->(FLOCK())
 *         ? "Sales is in use by another"
 *      ENDIF
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      RLOCK()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_FLOCK( void )
{
   DBLOCKINFO pLockInfo;

   pLockInfo.fResult = FALSE;
   if( pCurrArea )
   {
      pLockInfo.itmRecID = 0;
      pLockInfo.uiMethod = FILE_LOCK;
      SELF_LOCK( ( AREAP ) pCurrArea->pArea, &pLockInfo );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "FLOCK" );

   hb_retl( pLockInfo.fResult );
}

/*  $DOC$
 *  $FUNCNAME$
 *      FOUND()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Determine if the previous search operation succeeded
 *  $SYNTAX$
 *      FOUND() --> lSuccess
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      FOUND() returns true (.T.) if the last search command was successful;
 *   otherwise, it returns false (.F.).
 *
 *  $DESCRIPTION$
 *      FOUND() is a database function that determines whether a search
 *   operation (i.e., FIND, LOCATE, CONTINUE, SEEK, or SET RELATION)
 *   succeeded.  When any of these commands are executed, FOUND() is set to
 *   true (.T.) if there is a match; otherwise, it is set to false (.F.).
 *
 *      If the search command is LOCATE or CONTINUE, a match is the next record
 *   meeting the scope and condition.  If the search command is FIND, SEEK or
 *   SET RELATION, a match is the first key in the controlling index that
 *   equals the search argument.  If the key value equals the search
 *   argument, FOUND() is true (.T.); otherwise, it is false (.F.).
 *
 *      The value of FOUND() is retained until another record movement command
 *   is executed.  Unless the command is another search command, FOUND() is
 *   automatically set to false (.F.).
 *
 *      Each work area has a FOUND() value.  This means that if one work area
 *   has a RELATION set to a child work area, querying FOUND() in the child
 *   returns true (.T.) if there is a match.
 *
 *      By default, FOUND() operates on the currently selected work area.  It
 *   can be made to operate on an unselected work area by specifying it
 *   within an aliased expression (see example below).
 *
 *    FOUND() will return false (.F.) if there is no database open in the
 *   current work area.
 *  $EXAMPLES$
 *      This example illustrates the behavior of FOUND() after a
 *      record movement command:
 *
 *      USE Sales INDEX Sales
 *      ? INDEXKEY(0)            // Result: SALESMAN
 *      SEEK "1000"
 *      ? FOUND()                // Result: .F.
 *      SEEK "100"
 *      ? FOUND()                // Result: .T.
 *      SKIP
 *      ? FOUND()                // Result: .F.
 *
 *      This example tests a FOUND() value in an unselected work area
 *      using an aliased expression:
 *
 *      USE Sales INDEX Sales NEW
 *      USE Customer INDEX Customer NEW
 *      SET RELATION TO CustNum INTO Sales
 *      //
 *      SEEK "Smith"
 *      ? FOUND(), Sales->(FOUND())
 *
 *      This code fragment processes all Customer records with the key
 *      value "Smith" using FOUND() to determine when the key value changes:
 *
 *      USE Customer INDEX Customer NEW
 *      SEEK "Smith"
 *      DO WHILE FOUND()
 *         .
 *         . <statements>
 *         .
 *         SKIP
 *         LOCATE REST WHILE Name == "Smith"
 *      ENDDO
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      EOF()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_FOUND( void )
{
   BOOL bFound = FALSE;

   if( pCurrArea )
      SELF_FOUND( ( AREAP ) pCurrArea->pArea, &bFound );
   hb_retl( bFound );
}

/*  $DOC$
 *  $FUNCNAME$
 *      HEADER()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the current database file header length
 *  $SYNTAX$
 *      HEADER() --> nBytes
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *     HEADER() returns the number of bytes in the header of the current
 *   database file as an integer numeric value.  If no database file is in
 *   use, HEADER() returns a zero (0).
 *  $DESCRIPTION$
 *     HEADER() is a database function that is used with LASTREC(), RECSIZE(),
 *   and DISKSPACE() to create procedures for backing up files.
 *
 *     By default, HEADER() operates on the currently selected work area.  It
 *   will operate on an unselected work area if you specify it as part of an
 *   aliased expression (see example below).
 *  $EXAMPLES$
 *     This example determines the header size of the Sales.dbf:
 *
 *      USE Sales NEW
 *      ? HEADER()            // Result: 258
 *
 *     This example defines a pseudofunction, DbfSize(), that uses
 *      HEADER() with RECSIZE() and LASTREC() to calculate the size of the
 *      current database file in bytes:
 *
 *      #define DbfSize()   ((RECSIZE() * LASTREC()) + ;
 *            HEADER() + 1)
 *
 *      Later you can use DbfSize() as you would any function:
 *
 *      USE Sales NEW
 *      USE Customer NEW
 *      ? DbfSize()
 *      ? Sales->(DbfSize())
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      DISKSPACE(),LASTREC(),RECSIZE()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_HEADER( void )
{
   PHB_ITEM pRecSize;

   if( !pCurrArea )
      hb_retni( 0 );
   else
   {
      pRecSize = hb_itemNew( NULL );
      SELF_INFO( ( AREAP ) pCurrArea->pArea, DBI_GETHEADERSIZE, pRecSize );
      hb_itemReturn( pRecSize );
      hb_itemRelease( pRecSize );
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *      INDEXORD()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the order position of the controlling index
 *  $SYNTAX$
 *      INDEXORD() --> nOrder
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *     INDEXORD() returns an integer numeric value.  The value returned is
 *   equal to the position of the controlling index in the list of open
 *   indexes for the current work area.  A value of zero indicates that there
 *   is no controlling index and records are being accessed in natural order.
 *   If no database file is open, INDEXORD() will also return a zero.
 *  $DESCRIPTION$
 *     INDEXORD() is a database function that determines the position of the
 *   controlling index in the list of index files opened by the last
 *   USE...INDEX or SET INDEX TO in the current work area.  It is often
 *   useful to save the last controlling index so it can be restored later.
 *
 *     By default, INDEXORD() operates on the currently selected work area.  It
 *   will operate on an unselected work area if you specify it as part of an
 *   aliased expression (see example below).
 *  $EXAMPLES$
 *     This example uses INDEXORD() to save the current order.  After
 *      changing to a new order, it uses the saved value to restore the
 *      original order:
 *
 *      USE Customer INDEX Name, Serial NEW
 *      nOrder := INDEXORD()                  // Result: 1
 *      SET ORDER TO 2
 *      ? INDEXORD()                          // Result: 2
 *      SET ORDER TO nOrder
 *      ? INDEXORD()                          // Result: 1
 *
 *     This example uses an aliased expression to determine the order
 *      number of the controlling index in an unselected work area:
 *
 *      USE Sales INDEX Salesman, CustNum NEW
 *      USE Customer INDEX Name, Serial NEW
 *      ? Sales->(INDEXORD())                 // Result: 1
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      INDEXKEY()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_INDEXORD( void )
{
   DBORDERINFO pInfo;

   if( pCurrArea )
   {
      pInfo.itmResult = hb_itemPutNI( NULL, 0 );
      pInfo.itmOrder = NULL;
      SELF_ORDINFO( ( AREAP ) pCurrArea->pArea, DBOI_NUMBER, &pInfo );
      hb_retni( hb_itemGetNI( pInfo.itmResult ) );
      hb_itemRelease( pInfo.itmResult );
   }
   else
      hb_retni( 0 );
}

/*  $DOC$
 *  $FUNCNAME$
 *      LASTREC()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Determine the number of records in the current (.dbf) file
 *  $SYNTAX$
 *          LASTREC() | RECCOUNT()* --> nRecords
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *     LASTREC() returns the number of physical records in the current database
 *   file as an integer numeric value.  Filtering commands such as SET FILTER
 *   or SET DELETED have no effect on the return value.  LASTREC() returns
 *   zero if there is no database file in USE in the current work area.
 *  $DESCRIPTION$
 *     LASTREC() is a database function that determines the number of physical
 *   records in the current database file.  LASTREC() is identical to
 *   RECCOUNT() which is supplied as a compatibility function.
 *
 *     By default, LASTREC() operates on the currently selected work area.  It
 *   will operate on an unselected work area if you specify it as part of an
 *   aliased expression (see example below).
 *  $EXAMPLES$
 *     This example illustrates the relationship between LASTREC(),
 *      RECCOUNT(), and COUNT:
 *
 *      USE Sales NEW
 *      ? LASTREC(), RECCOUNT()          // Result: 84 84
 *      //
 *      SET FILTER TO Salesman = "1001"
 *      COUNT TO nRecords
 *      ? nRecords, LASTREC()            // Result: 14 84
 *
 *     This example uses an aliased expression to access the number
 *      of records in a open database file in an unselected work area:
 *
 *      USE Sales NEW
 *      USE Customer NEW
 *      ? LASTREC(), Sales->(LASTREC())
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      EOF()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_LASTREC( void )
{
   HB_RECCOUNT();
}

HARBOUR HB_LOCK( void )
{
   DBLOCKINFO pLockInfo;

   pLockInfo.fResult = FALSE;
   if( pCurrArea )
   {
      pLockInfo.itmRecID = 0;
      pLockInfo.uiMethod = FILE_LOCK;
      SELF_LOCK( ( AREAP ) pCurrArea->pArea, &pLockInfo );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "LOCK" );

   hb_retl( pLockInfo.fResult );
}

/*  $DOC$
 *  $FUNCNAME$
 *      LUPDATE()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the last modification date of a (.dbf) file
 *  $SYNTAX$
 *      LUPDATE() --> dModification
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *     LUPDATE() returns the date of last change to the open database file in
 *   the current work area.  If there is no database file in USE, LUPDATE()
 *   returns a blank date.
 *
 *  $DESCRIPTION$
 *     LUPDATE() is a database function that determines the date the database
 *   file in the current work area was last modified and CLOSEd.  By default,
 *   LUPDATE() operates on the currently selected work area.  It will operate
 *   on an unselected work area if you specify it as part of an aliased
 *   expression as shown in the example below.
 *  $EXAMPLES$
 *     This example demonstrates that the modification date of
 *      database file is not changed until the database file is closed:
 *
 *      ? DATE()                  // Result: 09/01/90
 *      USE Sales NEW
 *      ? LUPDATE()               // Result: 08/31/90
 *      //
 *      APPEND BLANK
 *      ? LUPDATE()               // Result: 08/31/90
 *      CLOSE DATABASES
 *      //
 *      USE Sales NEW
 *      ? LUPDATE()               // Result: 09/01/90
 *
 *     This example uses an aliased expression to access LUPDATE()
 *      for a database file open in an unselected work area:
 *
 *      USE Sales NEW
 *      USE Customer NEW
 *      ? LUPDATE(), Sales->(LUPDATE())
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *  FIELDNAME(),LASTREC(),RECSIZE()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_LUPDATE( void )
{
   if( !pCurrArea )
      hb_itemPutDS( &hb_stack.Return, "" );
   else
      SELF_INFO( ( AREAP ) pCurrArea->pArea, DBI_LASTUPDATE, &hb_stack.Return );
}

/*  $DOC$
 *  $FUNCNAME$
 *      NETERR()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Determine if a network command has failed
 *  $SYNTAX$
 *      NETERR([<lNewError>]) --> lError
 *  $ARGUMENTS$
 *     <lNewError> if specified sets the value returned by NETERR() to the
 *   specified status.  <lNewError> can be either true (.T.) or false (.F.).
 *   Setting NETERR() to a specified value allows the runtime error handler
 *   to control the way certain file errors are handled.  For more
 *   information, refer to Errorsys.prg.
 *
 *  $RETURNS$
 *     NETERR() returns true (.T.) if a USE or APPEND BLANK fails.  The initial
 *   value of NETERR() is false (.F.).  If the current process is not running
 *   under a network operating system, NETERR() always returns false (.F.).
 *  $DESCRIPTION$
 *     NETERR() is a network function.  It is a global flag set by USE,
 *   USE...EXCLUSIVE, and APPEND BLANK in a network environment.  It is used
 *   to test whether any of these commands have failed by returning true
 *   (.T.) in the following situations:
 *
 *   NETERR() Causes
 *   컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
 *   Command             Cause
 *   컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
 *   USE                 USE EXCLUSIVE by another process
 *   USE...EXCLUSIVE     USE EXCLUSIVE or USE by another process
 *   APPEND BLANK        FLOCK() or RLOCK() of LASTREC() + 1 by another user
 *   컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
 *
 *     NETERR() is generally applied in a program by testing it following a USE
 *   or APPEND BLANK command.  If it returns false (.F.), you can perform the
 *   next operation.  If the command is USE, you can open index files.  If it
 *   is APPEND BLANK, you can assign values to the new record with a REPLACE
 *   or @...GET command.  Otherwise, you must handle the error by either
 *   retrying the USE or APPEND BLANK, or terminating the current operation
 *   with a BREAK or RETURN.
 *  $EXAMPLES$
 *     This example demonstrates typical usage of NETERR().  If the
 *      USE succeeds, the index files are opened and processing continues.
 *      If the USE fails, a message displays and control returns to the
 *      nearest BEGIN SEQUENCE construct:
 *
 *      USE Customer SHARED NEW
 *      IF !NETERR()
 *         SET INDEX TO CustNum, CustOrders, CustZip
 *      ELSE
 *         ? "File is in use by another"
 *         BREAK
 *      ENDIF
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      FLOCK(),RLOCK()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_NETERR( void )
{
   if( ISLOG( 1 ) )
      bNetError = hb_parl( 1 );

   hb_retl( bNetError );
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDBAGEXT()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the default Order Bag RDD extension
 *  $SYNTAX$
 *      ORDBAGEXT() --> cBagExt
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      ORDBAGEXT() returns a character expression.
 *  $DESCRIPTION$
 *     ORDBAGEXT() is an Order management function that returns a character
 *   expression that is the default Order Bag extension of the current or
 *   aliased work area.  cBagExt is determined by the RDD active in the
 *   current work area.
 *
 *     ORDBAGEXT() supersedes the INDEXEXT() and is not recommended.
 *
 *  $EXAMPLES$
 *     USE sample VIA "DBFNTX"
 *     ? ORDBAGEXT()      //  Returns .ntx
 *
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      INDEXEXT(),ORDBAGNAME()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDBAGEXT( void )
{
   LPRDDNODE pRddNode;
   AREAP pTempArea;
   USHORT uiSize, uiRddID;
   DBORDERINFO pInfo;
   pInfo.itmOrder = NULL;
   if( !pCurrArea )
   {
      hb_rddCheck();
      uiRddID = 0;
      pRddNode = hb_rddFindNode( szDefDriver, &uiRddID );
      if( !pRddNode )
      {
         hb_retc( "" );
         return;
      }
      uiSize = sizeof( AREA );    /* Default Size Area */
      pTempArea = ( AREAP ) hb_xgrab( uiSize );
      memset( pTempArea, 0, uiSize );
      pTempArea->lprfsHost = &pRddNode->pTable;

      /* Need more space? */
      SELF_STRUCTSIZE( ( AREAP ) pTempArea, &uiSize );
      if( uiSize > sizeof( AREA ) )   /* Size of Area changed */
         pTempArea = ( AREAP ) hb_xrealloc( pTempArea, uiSize );

      pRddNode->uiAreaSize = uiSize; /* Update the size of WorkArea */
      pTempArea->rddID = uiRddID;

      if( SELF_NEW( ( AREAP ) pTempArea ) == FAILURE )
         hb_retc( "" );
      else
      {
         pInfo.itmResult = hb_itemPutC( NULL, "" );
         SELF_ORDINFO( pTempArea, DBOI_BAGEXT, &pInfo );
         hb_retc( pInfo.itmResult->item.asString.value );
         hb_itemRelease( pInfo.itmResult );
         SELF_RELEASE( ( AREAP ) pTempArea );
      }
      hb_xfree( pTempArea );
   }
   else
   {
      pInfo.itmResult = hb_itemPutC( NULL, "" );
      SELF_ORDINFO( ( AREAP ) pCurrArea->pArea, DBOI_BAGEXT, &pInfo );
      hb_retc( pInfo.itmResult->item.asString.value );
      hb_itemRelease( pInfo.itmResult );
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDBAGNAME()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the Order Bag name of a specific Order
 *  $SYNTAX$
 *      ORDBAGNAME(<nOrder> | <cOrderName>) --> cOrderBagName
 *  $ARGUMENTS$
 *     <nOrder> is an integer that identifies the position in the Order
 *   List of the target Order whose Order Bag name is sought.
 *
 *     <cOrderName> is a character string that represents the name of the
 *   target Order whose Order Bag name is sought.
 *
 *  $RETURNS$
 *     ORDBAGNAME() returns a character string, the Order Bag name of the
 *   specific Order.
 *  $DESCRIPTION$
 *     ORDBAGNAME() is an Order management function that lets you access the
 *   name of the Order Bag in which <cOrderName> resides.  You may identify
 *   the Order as a character string or with an integer that represents its
 *   position in the Order List.  In case of duplicate names, ORDBAGNAME()
 *   only recognizes the first matching name.
 *  $EXAMPLES$
 *     The following example uses ORDBAGNAME() with the default
 *      DBFNTX driver:
 *
 *      USE Customer VIA "DBFNTX" NEW
 *      SET INDEX TO CuAcct, CuName, CuZip
 *      ORDBAGNAME( 2 )               // Returns: CuName
 *      ORDBAGNAME( 1 )               // Returns: CuAcct
 *      ORDBAGNAME( 3 )               // Returns: CuZip
 *
 *
 *     In this example, Customer.cdx contains three orders named
 *      CuAcct, CuName, CuZip:
 *
 *      USE Customer VIA "DBFCDX" NEW
 *      SET INDEX TO Customer
 *      ORDBAGNAME( "CuAcct" )        // Returns: Customer
 *      ORDBAGNAME( "CuName" )        // Returns: Customer
 *      ORDBAGNAME( "CuZip" )         // Returns: Customer
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      ORDBAGEXT()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDBAGNAME( void )
{
   DBORDERINFO pOrderInfo;

   if( pCurrArea )
   {
      pOrderInfo.itmOrder = hb_param( 1, IT_STRING );
      if( !pOrderInfo.itmOrder )
         pOrderInfo.itmOrder = hb_param( 1, IT_NUMERIC );
      if( !pOrderInfo.itmOrder )
      {
         hb_errRT_DBCMD( EG_ARG, 1006, NULL, "ORDBAGNAME" );
         return;
      }
      pOrderInfo.itmResult = hb_itemPutC( NULL, "" );
      SELF_ORDINFO( ( AREAP ) pCurrArea->pArea, DBOI_BAGNAME, &pOrderInfo );
      hb_retc( pOrderInfo.itmResult->item.asString.value );
      hb_itemRelease( pOrderInfo.itmResult );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "ORDBAGNAME" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDCONDSET()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Set the Condition and scope for an order
 *  $SYNTAX$
 *      ORDCONSET([<cForCondition>],
 *        [<bForCondition>],
 *        [<lAll>],
 *        [<bWhileCondition>],
 *        [<bEval>],
 *        [<nInterval>],
 *        [<nStart>],
 *        [<nNext>],
 *        [<nRecord>],
 *        [<lRest>],
 *        [<lDescend>],
 *        [<lAdditive>],
 *        [<lCurrent>],
 *        [<lCustom>],
 *        [<lNoOptimize>])
 *  $ARGUMENTS$
 *      <cForCondition> is a string that specifies the FOR condition for the
 *   order.
 *     <bForCondition> is a code block that defines a FOR condition that
 *   each record within the scope must meet in order to be processed. If
 *   a record does not meet the specified condition,it is ignored and the
 *   next  record is processed.Duplicate keys values are not added to the
 *   index file when a FOR condition is Used.
 *  $RETURNS$
 *
 *  $DESCRIPTION$
 *
 *  $EXAMPLES$
 *
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *      ORDCONDSET() is CA-Clipper compliant
 *  $SEEALSO$
 *
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDCONDSET( void )
{
   LPDBORDERCONDINFO pOrderCondInfo;
   char * szFor;
   ULONG ulLen;
   PHB_ITEM pItem;

   if( pCurrArea )
   {
      pOrderCondInfo = ( LPDBORDERCONDINFO ) hb_xgrab( sizeof( DBORDERCONDINFO ) );
      szFor = hb_parc( 1 );
      ulLen = strlen( szFor );
      if( ulLen )
      {
         pOrderCondInfo->abFor = ( BYTE * ) hb_xgrab( ulLen + 1 );
         strcpy( ( char * ) pOrderCondInfo->abFor, szFor );
      }
      else
         pOrderCondInfo->abFor = NULL;
      pItem = hb_param( 2, IT_BLOCK );
      if( pItem )
      {
         pOrderCondInfo->itmCobFor = hb_itemNew( NULL );
         hb_itemCopy( pOrderCondInfo->itmCobFor, pItem );
      }
      else
         pOrderCondInfo->itmCobFor = NULL;
      if( ISLOG( 3 ) )
         pOrderCondInfo->fAll = hb_parl( 3 );
      else
         pOrderCondInfo->fAll = TRUE;
      pItem = hb_param( 4, IT_BLOCK );
      if( pItem )
      {
         pOrderCondInfo->itmCobWhile = hb_itemNew( NULL );
         hb_itemCopy( pOrderCondInfo->itmCobWhile, pItem );
      }
      else
         pOrderCondInfo->itmCobWhile = NULL;
      pItem = hb_param( 5, IT_BLOCK );
      if( pItem )
      {
         pOrderCondInfo->itmCobEval = hb_itemNew( NULL );
         hb_itemCopy( pOrderCondInfo->itmCobEval, pItem );
      }
      else
         pOrderCondInfo->itmCobEval = NULL;
      pOrderCondInfo->lStep = hb_parnl( 6 );
      pOrderCondInfo->lStartRecno = hb_parnl( 7 );
      pOrderCondInfo->lNextCount = hb_parnl( 8 );
      pOrderCondInfo->lRecno = hb_parnl( 9 );
      pOrderCondInfo->fRest = hb_parl( 10 );
      pOrderCondInfo->fDescending = hb_parl( 11 );
      pOrderCondInfo->fAdditive = hb_parl( 12 );
      pOrderCondInfo->fScoped = hb_parl( 13 );
      pOrderCondInfo->fCustom = hb_parl( 14 );
      pOrderCondInfo->fNoOptimize = hb_parl( 15 );
      if( !pOrderCondInfo->itmCobWhile )
         pOrderCondInfo->fRest = TRUE;
      if( pOrderCondInfo->lNextCount || pOrderCondInfo->lRecno || pOrderCondInfo->fRest )
         pOrderCondInfo->fAll = FALSE;
      hb_retl( SELF_ORDSETCOND( ( AREAP ) pCurrArea->pArea, pOrderCondInfo ) == SUCCESS );
   }
   else
      hb_retl( FALSE );
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDCREATE()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Create an Order in an Order Bag
 *  $SYNTAX$
 *     ORDCREATE(<cOrderBagName>,[<cOrderName>],    <cExpKey>,
 *      [<bExpKey>], [<lUnique>]) --> NIL
 *  $ARGUMENTS$
 *     <cOrderBagName> is the name of a disk file containing one or more
 *   Orders.  You may specify <cOrderBagName> as the filename with or without
 *   the pathname or extension.  If you do not include the extension as part
 *   of <cOrderBagName> HARBOUR uses the default extension of the current
 *   RDD.
 *
 *     <cOrderName> is the name of the Order to be created.
 *
 *     Note: Although both <cOrderBagName> and <cOrderName> are both
 *   optional, at least one of them must be specified.
 *
 *     <cExpKey> is an expression that returns the key value to place in
 *   the Order for each record in the current work area.  <cExpKey> can
 *   represent a character, date, logical, or numeric data type.  The maximum
 *   length of the index key expression is determined by the database driver.
 *
 *     <bExpKey> is a code block that evaluates to a key value that is
 *   placed in the Order for each record in the current work area.  If you do
 *   not supply <bExpKey>, it is macro-compiled from <cExpKey>.
 *
 *     <lUnique> is an optional logical value that specifies whether a
 *   unique Order is to be created.  If <lUnique> is omitted, the current
 *   global _SET_UNIQUE setting is used.
 *  $RETURNS$
 *      ORDCREATE() always returns NIL.
 *  $DESCRIPTION$
 *     ORDCREATE() is an Order management function that creates an Order in the
 *   current work area.  It works like DBCREATEINDEX() except that it lets
 *   you create Orders in RDDs that recognize multiple-Order Bags.
 *   ORDCREATE() supersedes the DBCREATEINDEX() function because of this
 *   capability, and is the preferred function.
 *
 *     The active RDD determines the Order capacity of an Order Bag.  The
 *   default  DBFNTX and the DBFNDX drivers only support single-Order Bags,
 *   while other RDDs may support multiple-Order Bags (e.g., the DBFCDX and
 *   DBFMDX drivers).
 *
 *     In RDDs that support production or structural indexes (e.g., DBFCDX,
 *   DBFMDX), if you specify a Tag but do not specify an Order Bag, the Tag is
 *   created and added to the index.  If no production or structural index
 *   exists, it will be created and the Tag will be added to it.  When using
 *   RDDs that support multiple Order Bags, you must explicitly SET ORDER (or
 *   ORDSETFOCUS()) to the desired controlling Order.  If you do not specify
 *   a controlling Order, the data file will be viewed in natural Order.
 *
 *     If <cOrderBagName> does not exist, it is created in accordance with the
 *   RDD in the current or specified work area.
 *
 *    If <cOrderBagName> exists and the RDD specifies that Order Bags can only
 *   contain a single Order, <cOrderBagName> is erased and the new Order is
 *   added to the Order List in the current or specified work area.
 *
 *     If <cOrderBagName> exists and the RDD specifies that Order Bags can
 *   contain multiple Tags, <cOrderName> is created if it does not already
 *   exist, otherwise <cOrderName> is replaced in <cOrderBagName> and the
 *   Order is added to the Order List in the current or specified work area.
 *  $EXAMPLES$
 *     The following example demonstrates ORDCREATE() with the DBFNDX
 *      driver:
 *
 *      USE Customer VIA "DBFNDX" NEW
 *      ORDCREATE( "CuAcct",, "Customer->Acct" )
 *
 *
 *     The following example demonstrates ORDCREATE() with the
 *      default DBFNTX driver:
 *
 *      USE Customer VIA "DBFNTX" NEW
 *      ORDCREATE( "CuAcct", "CuAcct", "Customer->Acct", ;
 *            {|| Customer->Acct } )
 *
 *     The following example demonstrates ORDCREATE() with the FoxPro
 *      driver, DBFCDX:
 *
 *      USE Customer VIA "DBFCDX" NEW
 *      ORDCREATE( "Customer", "CuAcct", "Customer->Acct" )
 *
 *     This example creates the Order "CuAcct" and adds it to the
 *      production index (Order Bag) "Customer".  The production index , will
 *      be created if it doesn't exist:
 *
 *      USE Customer VIA "DBFMDX" NEW
 *      ORDCREATE( , "CuAcct", "Customer->Acct" )
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      DBCREATEINDEX()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDCREATE( void )
{
   DBORDERCREATEINFO pOrderInfo;

   if( pCurrArea )
   {
      pOrderInfo.abBagName = ( BYTE * ) hb_parc( 1 );
      pOrderInfo.atomBagName = ( BYTE * ) hb_parc( 2 );
      pOrderInfo.abExpr = hb_param( 3, IT_STRING );
      if( ( ( strlen( ( char * ) pOrderInfo.abBagName ) == 0 ) &&
            ( strlen( ( char * ) pOrderInfo.atomBagName ) == 0 ) ) ||
          !pOrderInfo.abExpr )
      {
         hb_errRT_DBCMD( EG_ARG, 1006, NULL, "ORDCREATE" );
         return;
      }
      pOrderInfo.itmCobExpr = hb_param( 4, IT_BLOCK );
      if( ISLOG( 5 ) )
         pOrderInfo.fUnique = hb_parl( 5 );
      else
         pOrderInfo.fUnique = hb_set.HB_SET_UNIQUE;
      SELF_ORDCREATE( ( AREAP ) pCurrArea->pArea, &pOrderInfo );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "ORDCREATE" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDDESTROY()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Remove a specified Order from an Order Bag
 *  $SYNTAX$
 *          ORDDESTROY(<cOrderName> [, <cOrderBagName> ]) --> NIL
 *  $ARGUMENTS$
 *     <cOrderName> is the name of the Order to be removed from the current
 *   or specified work area.
 *
 *     <cOrderBagName> is the name of a disk file containing one or more
 *   Orders.  You may specify <cOrderBagName> as the filename with or without
 *   the pathname or appropriate extension.  If you do not include the
 *   extension as part of <cOrderBagName> HARBOUR uses the default
 *   extension of the current RDD.
 *  $RETURNS$
 *      ORDDESTROY() always returns NIL.
 *  $DESCRIPTION$
 *     ORDDESTROY() is an Order management function that removes a specified
 *   Order from multiple-Order Bags.
 *
 *     The active RDD determines the Order capacity of an Order Bag.  The
 *   default DBFNTX and the DBFNDX drivers only support single-Order Bags,
 *   while other RDDs may support multiple-Order Bags (e.g., the DBFCDX and
 *   DBPX drivers).
 *
 *     Note:  RDD suppliers may define specific behaviors for this command.
 *
 *     Warning!  ORDDESTROY() is not supported for DBFNDX and DBFNTX.
 *  $EXAMPLES$
 *     This example demonstrates ORDDESTROY() with the FoxPro driver,
 *      DBFCDX:
 *
 *      USE Customer VIA "DBFCDX" NEW
 *      SET INDEX TO Customer, CustTemp
 *      ORDDESTROY( "CuAcct", "Customer" )
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      ORDCREATE()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDDESTROY( void )
{
   DBORDERINFO pOrderInfo;

   if( pCurrArea )
   {
      pOrderInfo.itmOrder = hb_param( 1, IT_STRING );
      if( !pOrderInfo.itmOrder )
         pOrderInfo.itmOrder = hb_param( 1, IT_NUMERIC );
      pOrderInfo.atomBagName = hb_param( 2, IT_STRING );
      SELF_ORDDESTROY( ( AREAP ) pCurrArea->pArea, &pOrderInfo );
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDFOR()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the FOR expression of an Order
 *  $SYNTAX$
 *     ORDFOR(<cOrderName> | <nOrder>
 *      [, <cOrderBagName>]) --> cForExp
 *
 *  $ARGUMENTS$
 *     <cOrderName> is the name of the target Order, whose cForExp is
 *   sought.
 *
 *     <nOrder> is an integer that identifies the position in the Order
 *   List of the target Order whose cForExp is sought.
 *
 *     <cOrderBagName> is the name of an Order Bag containing one or more
 *   Orders.  You may specify <cOrderBagName> as the filename with or without
 *   the pathname or appropriate extension.  If you do not include the
 *   extension as part of <cOrderBagName> HARBOUR uses the default
 *   extension of the current RDD.
 *  $RETURNS$
 *     ORDFOR() returns a character expression, cForExp, that represents the
 *   FOR condition of the specified Order.  If the Order was not created
 *   using the FOR clause the return value will be an empty string ("").  If
 *   the database driver does not support the FOR condition, it may either
 *   return an empty string ("") or raise an "unsupported function" error,
 *   depending on the driver.
 *  $DESCRIPTION$
 *     ORDFOR() is an Order management function that returns the character
 *   string, cForExp, that represents the logical FOR condition of the Order,
 *   <cOrderName> or <nOrder>.
 *  $EXAMPLES$
 *     This example retrieves the FOR condition from an Order:
 *
 *      USE Customer NEW
 *      INDEX ON  Customer->Acct ;
 *         TO  Customer          ;
 *         FOR Customer->Acct > "AZZZZZ"
 *
 *      ORDFOR( "Customer" )      // Returns: Customer->Acct > "AZZZZZ"
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      ORDKEY(),ORDCREATE(),ORDNAME(),ORDNUMBER()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDFOR( void )
{
   DBORDERINFO pOrderInfo;

   if( pCurrArea )
   {
      pOrderInfo.itmOrder = hb_param( 1, IT_STRING );
      if( !pOrderInfo.itmOrder )
         pOrderInfo.itmOrder = hb_param( 1, IT_NUMERIC );
      pOrderInfo.atomBagName = hb_param( 2, IT_STRING );
      if( !pOrderInfo.itmOrder )
      {
         hb_errRT_DBCMD( EG_ARG, 1006, NULL, "ORDFOR" );
         return;
      }
      pOrderInfo.itmResult = hb_itemPutC( NULL, "" );
      SELF_ORDINFO( ( AREAP ) pCurrArea->pArea, DBOI_CONDITION, &pOrderInfo );
      hb_retc( pOrderInfo.itmResult->item.asString.value );
      hb_itemRelease( pOrderInfo.itmResult );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "ORDFOR" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDKEY()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the key expression of an Order
 *  $SYNTAX$
 *     ORDKEY(<cOrderName> | <nOrder>
 *      [, <cOrderBagName>]) --> cExpKey
 *  $ARGUMENTS$
 *     <cOrderName> is the name of an Order, a logical ordering of a
 *   database.
 *
 *     <nOrder> is an integer that identifies the position in the Order
 *   List of the target Order whose cExpKey is sought.
 *
 *     <cOrderBagName> is the name of a disk file containing one or more
 *   Orders.  You may specify <cOrderBagName> as the filename with or without
 *   the pathname or appropriate extension.  If you do not include the
 *   extension as part of <cOrderBagName> HARBOUR uses the default
 *   extension of the current RDD.
 *  $RETURNS$
 *      Returns a character string, cExpKey.
 *  $DESCRIPTION$
 *     ORDKEY() is an Order management function that returns a character
 *   expression, cExpKey, that represents the key expression of the specified
 *   Order.
 *
 *     You may specify the Order by name or with a number that represents its
 *   position in the Order List.  Using the Order name is the preferred
 *   method.
 *
 *     The active RDD determines the Order capacity of an Order Bag.  The
 *   default DBFNTX and the DBFNDX drivers only support single-Order Bags,
 *   while other RDDs may support multiple-Order Bags (e.g., the DBFCDX and
 *   DBFMDX drivers).
 *  $EXAMPLES$
 *     This example retrieves the index expression from an Order:
 *
 *      USE Customer NEW
 *      INDEX ON  Customer->Acct  ;
 *         TO  Customer           ;
 *         FOR Customer->Acct > "AZZZZZ"
 *
 *      ORDKEY( "Customer" )      // Returns: Customer->Acct
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      ORDFOR(),ORDNAME(),ORDNUMBER()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDKEY( void )
{
   DBORDERINFO pOrderInfo;

   if( pCurrArea )
   {
      pOrderInfo.itmOrder = hb_param( 1, IT_STRING );
      if( !pOrderInfo.itmOrder )
         pOrderInfo.itmOrder = hb_param( 1, IT_NUMERIC );
      pOrderInfo.atomBagName = hb_param( 2, IT_STRING );
      if( !pOrderInfo.itmOrder )
      {
         hb_errRT_DBCMD( EG_ARG, 1006, NULL, "ORDKEY" );
         return;
      }
      pOrderInfo.itmResult = hb_itemPutC( NULL, "" );
      SELF_ORDINFO( ( AREAP ) pCurrArea->pArea, DBOI_EXPRESSION, &pOrderInfo );
      hb_retc( pOrderInfo.itmResult->item.asString.value );
      hb_itemRelease( pOrderInfo.itmResult );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "ORDKEY" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDLISTADD()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Add Orders to the Order List
 *  $SYNTAX$
 *     ORDLISTADD(<cOrderBagName>
 *      [, <cOrderName>]) --> NIL
 *  $ARGUMENTS$
 *     <cOrderBagName> is the name of a disk file containing one or more
 *   Orders.  You may specify <cOrderBagName> as the filename with or without
 *   the pathname or appropriate extension.  If you do not include the
 *   extension as part of <cOrderBagName> HARBOUR uses the default
 *   extension of the current RDD.
 *
 *     <cOrderName> the name of the specific Order from the Order Bag to be
 *   added to the Order List of the current work area.  If you do not specify
 *   <cOrderName>, all orders in the Order Bag are added to the Order List of
 *   the current work area.
 *  $RETURNS$
 *      ORDLISTADD() always returns NIL.
 *  $DESCRIPTION$
 *     ORDLISTADD() is an Order management function that adds the contents of
 *   an Order Bag , or a single Order in an Order Bag, to the Order List.
 *   This function lets you extend the Order List without issuing a SET INDEX
 *   command that, first, clears all the active Orders from the Order List.
 *
 *     Any Orders already associated with the work area continue to be active.
 *   If the newly opened Order Bag contains the only Order associated with
 *   the work area, it becomes the controlling Order; otherwise, the
 *   controlling Order remains unchanged.
 *
 *     After the new Orders are opened, the work area is positioned to the
 *   first logical record in the controlling Order.
 *
 *     ORDLISTADD() is similar to the SET INDEX command or the INDEX clause of
 *   the USE command, except that it does not clear the Order List prior to
 *   adding the new order(s).
 *
 *     ORDLISTADD() supersedes the DBSETINDEX() function.
 *
 *     The active RDD determines the Order capacity of an Order Bag.  The
 *   default  DBFNTX and the DBFNDX drivers only support single-Order Bags,
 *   while other RDDs may support multiple-Order Bags (e.g., the DBFCDX and
 *   DBPX drivers).  When using RDDs that support multiple Order Bags, you
 *   must explicitly SET ORDER (or ORDSETFOCUS()) to the desired controlling
 *   Order.  If you do not specify a controlling Order, the data file will be
 *   viewed in natural Order.
 *  $EXAMPLES$
 *     In this example Customer.cdx contains three orders, CuAcct,
 *      CuName, and CuZip.  ORDLISTADD() opens Customer.cdx but only uses the
 *      order named CuAcct:
 *
 *      USE Customer VIA "DBFCDX" NEW
 *      ORDLISTADD( "Customer", "CuAcct" )
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      DBSETINDEX()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDLISTADD( void )
{
   DBORDERINFO pOrderInfo;

   if( pCurrArea )
   {
      pOrderInfo.atomBagName = hb_param( 1, IT_STRING );
      pOrderInfo.itmOrder = hb_param( 2, IT_STRING );
      if( !pOrderInfo.atomBagName )
      {
         hb_errRT_DBCMD( EG_ARG, 1006, NULL, "ORDLISTADD" );
         return;
      }
      SELF_ORDLSTADD( ( AREAP ) pCurrArea->pArea, &pOrderInfo );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "ORDLISTADD" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDLISTCLEAR()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Clear the current Order List
 *  $SYNTAX$
 *      ORDLISTCLEAR() --> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      ORDLISTCLEAR() always returns NIL.
 *  $DESCRIPTION$
 *     ORDLISTCLEAR() is an Order management function that removes all Orders
 *   from the Order List for the current or aliased work area.  When you are
 *   done, the Order List is empty.
 *
 *    This function supersedes the function DBCLEARINDEX().
 *
 *  $EXAMPLES$
 *   USE Sales NEW
 *   SET INDEX TO SaRegion, SaRep, SaCode
 *   .
 *   . < statements >
 *   .
 *   ORDLISTCLEAR()      // Closes all the current indexes
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *    DBCLEARINDEX()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDLISTCLEAR( void )
{
   if( pCurrArea )
      SELF_ORDLSTCLEAR( ( AREAP ) pCurrArea->pArea );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "ORDLISTCLEAR" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDLISTREBUILD()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Rebuild all Orders in the Order List of the current work area
 *  $SYNTAX$
 *      ORDLISTREBUILD() --> NIL
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      ORDLISTREBUILD() always returns NIL.
 *  $DESCRIPTION$
 *     ORDLISTREBUILD() is an Order management function that rebuilds all the
 *   orders in the current or aliased Order List.
 *
 *     To only rebuild a single Order use the function ORDCREATE().
 *
 *     Unlike ORDCREATE(), this function rebuilds all Orders in the Order List.
 *   It is equivalent to REINDEX.
 *
 *  $EXAMPLES$
 *   USE Customer NEW
 *   SET INDEX TO CuAcct, CuName, CuZip
 *   ORDLISTREBUILD()     // Causes CuAcct, CuName, CuZip to
 *                        // be rebuilt
 *
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      ORDCREATE()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDLISTREBUILD( void )
{
   if( pCurrArea )
      SELF_ORDLSTREBUILD( ( AREAP ) pCurrArea->pArea );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "ORDLISTCLEAR" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDNAME()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the name of an Order in the Order List
 *  $SYNTAX$
 *      ORDNAME(<nOrder>[,<cOrderBagName>])
 *      --> cOrderName
 *  $ARGUMENTS$
 *     <nOrder> is an integer that identifies the position in the Order
 *   List of the target Order whose database name is sought.
 *
 *     <cOrderBagName> is the name of a disk file containing one or more
 *   Orders.  You may specify <cOrderBagName> as the filename with or without
 *   the pathname or appropriate extension.  If you do not include the
 *   extension as part of <xcOrderBagName> HARBOUR uses the default
 *   extension of the current RDD.
 *  $RETURNS$
 *     ORDNAME() returns the name of the specified Order in the current Order
 *   List or the specified Order Bag if opened in the Current Order list.
 *  $DESCRIPTION$
 *     ORDNAME() is an Order management function that returns the name of the
 *   specified Order in the current Order List.
 *
 *     If <cOrderBagName> is an Order Bag that has been emptied into the
 *   current Order List, only those Orders in the Order List that correspond
 *   to <cOrderBagName> Order Bag are searched.
 *
 *     The active RDD determines the Order capacity of an Order Bag.  The
 *   default DBFNTX and the DBFNDX drivers only support single-Order Bags,
 *   while other RDDs may support multiple-Order Bags (e.g., the DBFCDX and
 *   DBPX drivers).
 *  $EXAMPLES$
 *     This example retrieves the name of an Order using its position
 *      in the order list:
 *
 *      USE Customer NEW
 *      SET INDEX TO CuAcct, CuName, CuZip
 *      ORDNAME( 2 )                        // Returns: CuName
 *
 *     This example retrieves the name of an Order given its position
 *      within a specific Order Bag in the Order List:
 *
 *      USE Customer NEW
 *      SET INDEX TO Temp, Customer
 *      // Assume Customer contains CuAcct, CuName, CuZip
 *      ORDNAME( 2, "Customer" )            // Returns: CuName
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      ORDFOR(),ORDKEY(),ORDNUMBER()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDNAME( void )
{
   DBORDERINFO pOrderInfo;

   if( pCurrArea )
   {
      pOrderInfo.itmOrder = hb_param( 1, IT_NUMERIC );
      pOrderInfo.atomBagName = hb_param( 2, IT_STRING );
      if( !pOrderInfo.itmOrder )
      {
         hb_errRT_DBCMD( EG_ARG, 1006, NULL, "ORDNAME" );
         return;
      }
      pOrderInfo.itmResult = hb_itemPutC( NULL, "" );
      SELF_ORDINFO( ( AREAP ) pCurrArea->pArea, DBOI_NAME, &pOrderInfo );
      hb_retc( pOrderInfo.itmResult->item.asString.value );
      hb_itemRelease( pOrderInfo.itmResult );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "ORDNAME" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDNUMBER()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the position of an Order in the current Order List
 *  $SYNTAX$
 *     ORDNUMBER(<cOrderName>
 *      [, <cOrderBagName>]) --> nOrderNo
 *  $ARGUMENTS$
 *     <cOrderName> the name of the specific Order whose position in the
 *   Order List is sought.
 *
 *     <cOrderBagName> is the name of a disk file containing one or more
 *   Orders.  You may specify <cOrderBagName> as the filename with or without
 *   the pathname or appropriate extension.  If you do not include the
 *   extension as part of <cOrderBagName> HARBOUR uses the default
 *   extension of the current RDD.
 *  $RETURNS$
 *     Returns nOrderNo, an integer that represents the position of the
 *   specified Order in the Order List.
 *  $DESCRIPTION$
 *     ORDNUMBER() is an Order management function that lets you determine the
 *   position in the current Order List of the specified Order.  ORDNUMBER()
 *   searches the Order List in the current work area and returns the
 *   position of the first Order that matches <cOrderName>.    If
 *   <cOrderBagName> is the name of an Order Bag newly emptied into the
 *   current Order List, only those orders in the Order List that have been
 *   emptied from <cOrderBagName> are searched.
 *
 *     If <cOrderName> is not found ORDNUMBER() raises a recoverable runtime
 *   error.
 *
 *     The active RDD determines the Order capacity of an Order Bag.  The
 *   default DBFNTX driver only supports single-Order Bags, while other RDDs
 *   may support multiple-Order Bags (e.g., the DBFCDX and DBPX drivers).
 *  $EXAMPLES$
 *   USE Customer VIA "DBFNTX" NEW
 *   SET INDEX TO CuAcct, CuName, CuZip
 *   ORDNUMBER( "CuName" )            // Returns: 2
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      INDEXORD()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDNUMBER( void )
{
   DBORDERINFO pOrderInfo;

   if( pCurrArea )
   {
      pOrderInfo.itmOrder = hb_param( 1, IT_STRING );
      pOrderInfo.atomBagName = hb_param( 2, IT_STRING );
      if( !pOrderInfo.itmOrder )
      {
         hb_errRT_DBCMD( EG_ARG, 1006, NULL, "ORDNUMBER" );
         return;
      }
      pOrderInfo.itmResult = hb_itemPutNI( NULL, 0 );
      SELF_ORDINFO( ( AREAP ) pCurrArea->pArea, DBOI_NUMBER, &pOrderInfo );
      hb_retni( hb_itemGetNI( pOrderInfo.itmResult ) );
      hb_itemRelease( pOrderInfo.itmResult );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "ORDNUMBER" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      ORDSETFOCUS()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Set focus to an Order in an Order List
 *  $SYNTAX$
 *     ORDSETFOCUS([<cOrderName> | <nOrder>]
 *      [,<cOrderBagName>]) --> cPrevOrderNameInFocus
 *
 *  $ARGUMENTS$
 *     <cOrderName> is the name of the selected Order, a logical ordering
 *   of a database.  ORDSETFOCUS() ignores any invalid values of
 *   <cOrderName>.
 *
 *     <nOrder> is a number representing the position in the Order List of
 *   the selected Order.
 *
 *     <cOrderBagName> is the name of a disk file containing one or more
 *   Orders.  You may specify <cOrderBagName> as the filename with or without
 *   the pathname or appropriate extension.  If you do not include the
 *   extension as part of <cOrderBagName> HARBOUR uses the default
 *   extension of the current RDD.
 *  $RETURNS$
 *      ORDSETFOCUS() returns the Order Name of the previous controlling Order.
 *  $DESCRIPTION$
 *     ORDSETFOCUS() is an Order management function that returns the Order
 *   Name of the previous controlling Order and optionally sets the focus to
 *   an new Order.
 *
 *     If you do not specify <cOrderName> or <nOrder>, the name of the
 *   currently controlling order is returned and the controlling order
 *   remains unchanged.
 *
 *     All Orders in an Order List are properly updated no matter what
 *   <cOrderName> is the controlling Order.  After a change of controlling
 *   Orders, the record pointer still points to the same record.
 *
 *     The active RDD determines the Order capacity of an Order Bag.  The
 *   default DBFNTX driver only supports single-Order Bags, while other RDDs
 *   may support multiple-Order Bags (e.g., the DBFCDX and DBPX drivers).
 *
 *     ORDSETFOCUS() supersedes INDEXORD().
 *  $EXAMPLES$
 *
 *   USE Customer VIA "DBFNTX" NEW
 *   SET INDEX TO CuAcct, CuName, CuZip
 *   ? ORDSETFOCUS( "CuName" )        // Displays: "CuAcct"
 *   ? ORDSETFOCUS()                  // Displays: "CuName"
 *  $TESTS$
 *
 *  $STATUS$
 *      S
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_ORDSETFOCUS( void )
{
   DBORDERINFO pInfo;

   if( pCurrArea )
   {
      pInfo.itmOrder = hb_param( 1, IT_STRING );
      if( !pInfo.itmOrder )
         pInfo.itmOrder = hb_param( 1, IT_NUMERIC );
      pInfo.atomBagName = hb_param( 2, IT_STRING );
      pInfo.itmResult = hb_itemPutC( NULL, "" );
      SELF_ORDLSTFOCUS( ( AREAP ) pCurrArea->pArea, &pInfo );
      hb_retc( pInfo.itmResult->item.asString.value );
      hb_itemRelease( pInfo.itmResult );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "ORDSETFOCUS" );
}

/*  $DOC$
 *  $FUNCNAME$
 *      RDDLIST()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return an array of the available Replaceable Database Drivers
 *  $SYNTAX$
 *      RDDLIST([<nRDDType>]) --> aRDDList
 *  $ARGUMENTS$
 *     <nRDDType> is an integer that represents the type of the RDD you
 *   wish to list.  The constants RDT_FULL and RDT_TRANSFER represent the two
 *   types of RDDs currently available.
 *
 *   RDDType Summary
 *   컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
 *   Constant       Value     Meaning
 *   컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
 *   RDT_FULL       1         Full RDD implementation
 *   RDT_TRANSFER   2         Import/Export only driver
 *   컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
 *
 *     RDT_FULL identifies full-featured RDDs that have all the capabilities
 *   associated with an RDD.
 *
 *     RDT_TRANSFER identifies RDDs of limited capability.  They can only
 *   transfer records between files.  You cannot use these limited RDD
 *   drivers to open a file in a work area.  The SDF and DELIM drivers are
 *   examples of this type of RDD.  They are only used in the implementation
 *   of APPEND FROM and COPY TO with SDF or DELIMITED files.
 *  $RETURNS$
 *     RDDLIST() returns a one-dimensional array of the RDD names registered
 *   with the application as <nRDDType>.
 *  $DESCRIPTION$
 *     RDDLIST() is an RDD function that returns a one-dimensional array that
 *   lists the available RDDs.
 *
 *     If you do not supply <nRDDType>, all available RDDs, regardless of type,
 *   are returned.
 *  $EXAMPLES$
 *     In this example RDDLIST() returns an array containing the
 *      character strings, "DBF", "SDF", "DELIM", "DBFCDX", and "DBFNTX":
 *
 *      REQUEST DBFCDX
 *
 *      .
 *      . < statements >
 *      .
 *
 *      aRDDs := RDDLIST()
 *
 *            // Returns {"DBF", SDF", "DELIM", "DBFCDX", "DBFNTX" }
 *
 *     In this example, RDDLIST() returns an array containing the
 *      character strings, "SDF" and "DELIM":
 *
 *      #include "rddsys.ch"
 *      .
 *      . < statements >
 *      .
 *      aImpExp := RDDLIST( RDT TRANSFER )
 *
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *
 *  $INCLUDE$
 *      RDDSYS.CH
 *  $END$
 */

HARBOUR HB_RDDLIST( void )
{
   USHORT uiType;
   PHB_ITEM pName;
   LPRDDNODE pRddNode;

   hb_rddCheck();
   hb_arrayNew( &hb_stack.Return, 0 );
   pName = hb_itemNew( NULL );
   pRddNode = pRddList;
   uiType = hb_parni( 1 );       /* 0 all types of RDD's */
   while( pRddNode )
   {
      if( ( uiType == 0 ) || ( pRddNode->uiType == uiType ) )
         hb_arrayAdd( &hb_stack.Return, hb_itemPutC( pName, pRddNode->szName ) );
      pRddNode = pRddNode->pNext;
   }
   hb_itemRelease( pName );
}

/*  $DOC$
 *  $FUNCNAME$
 *      RDDNAME()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the name of the currently active RDD
 *  $SYNTAX$
 *      RDDNAME() --> cRDDName
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      Returns a character string, cRDDName, the registered name of the active
 *   RDD in the current or specified work area.
 *  $DESCRIPTION$
 *      RDDNAME() is an RDD function that returns a character string, cRDDName,
 *   the name of the active RDD in the current or specified work area.
 *
 *      You can specify a work area other than the currently active work area by
 *   aliasing the function.
 *  $EXAMPLES$
 *   USE Customer VIA "DBFNTX" NEW
 *   USE Sales    VIA "DBFCDX" NEW
 *
 *   ? RDDNAME()                          // Returns: DBFCDX
 *   ? Customer->( RDDNAME() )            // Returns: DBFNTX
 *   ? Sales->( RDDNAME() )               // Returns: DBFCDX
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      RDDLIST()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_RDDNAME( void )
{
   char * pBuffer;

   if( pCurrArea )
   {
      pBuffer = ( char * ) hb_xgrab( HARBOUR_MAX_RDD_DRIVERNAME_LENGTH + 1 );
      pBuffer[ 0 ] = '\0';
      SELF_SYSNAME( ( AREAP ) pCurrArea->pArea, ( BYTE * ) pBuffer );
      hb_retc( pBuffer );
      hb_xfree( pBuffer );
   }
   else
   {
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "RDDNAME" );
      hb_retc( "" );
   }
}

HARBOUR HB_RDDREGISTER( void )
{
   char szDriver[ HARBOUR_MAX_RDD_DRIVERNAME_LENGTH ];
   USHORT uiLen;

   hb_rddCheck();
   uiLen = hb_parclen( 1 );
   if( uiLen > 0 )
   {
      hb_strncpyUpper( szDriver, hb_parc( 1 ), uiLen );
      /*
       * hb_rddRegister returns:
       *
       * 0: Ok, RDD registered
       * 1: RDD already registerd
       * > 1: error
       */
      if( hb_rddRegister( szDriver, hb_parni( 2 ) ) <= 1 )
         return;
      hb_errInternal( 9002, "", "", "" );
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *      RDDSETDEFAULT()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Set or return the default RDD for the application
 *  $SYNTAX$
 *      RDDSETDEFAULT([<cNewDefaultRDD>])
 *      --> cPreviousDefaultRDD
 *
 *  $ARGUMENTS$
 *      <cNewDefaultRDD> is a character string, the name of the RDD that is
 *   to be made the new default RDD in the application.
 *  $RETURNS$
 *      RDDSETDEFAULT() returns a character string, cPreviousDefaultRDD, the
 *   name of the previous default driver.  The default driver is the driver
 *   that HARBOUR uses if you do not explicitly specify an RDD with the
 *   VIA clause of the USE command.
 *  $DESCRIPTION$
 *      RDDSETDEFAULT() is an RDD function that sets or returns the name of the
 *   previous default RDD driver and, optionally, sets the current driver to
 *   the new RDD driver specified by cNewDefaultRDD.  If <cNewDefaultDriver>
 *   is not specified, the current default driver name is returned and
 *   continues to be the current default driver.
 *
 *    This function replaces the DBSETDRIVER() function.
 *  $EXAMPLES$
 *   // If the default driver is not DBFNTX, make it the default
 *
 *   IF ( RDDSETDEFAULT() != "DBFNTX" )
 *      cOldRdd := RDDSETDEFAULT( "DBFNTX" )
 *   ENDIF
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      DBSETDRIVER()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_RDDSETDEFAULT( void )
{
   char szNewDriver[ HARBOUR_MAX_RDD_DRIVERNAME_LENGTH ];
   USHORT uiLen;

   hb_rddCheck();
   hb_retc( szDefDriver );

   uiLen = hb_parclen( 1 );
   if( uiLen > 0 )
   {
      hb_strncpyUpper( szNewDriver, hb_parc( 1 ), uiLen );

      if( !hb_rddFindNode( szNewDriver, NULL ) )
      {
         hb_errRT_DBCMD( EG_ARG, 1015, NULL, "RDDSETDEFAULT" );
         return;
      }

      szDefDriver = ( char * ) hb_xrealloc( szDefDriver, uiLen + 1 );
      strcpy( szDefDriver, szNewDriver );
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *      RECCOUNT()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Determine the number of records in the current (.dbf) file
 *  $SYNTAX$
 *      RECCOUNT()* | LASTREC() --> nRecords
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *     RECCOUNT() returns the number of physical records in the current
 *   database file as an integer numeric value.  Filtering commands such as
 *   SET FILTER or SET DELETED have no effect on the return value.
 *   RECCOUNT() returns zero if there is no database file open in the current
 *   work area.
 *
 *  $DESCRIPTION$*
 *     RECCOUNT() is a database function that is a synonym for LASTREC().  By
 *   default, RECCOUNT() operates on the currently selected work area.  It
 *   will operate on an unselected work area if you specify it as part of an
 *   aliased expression (see example below).
 *  $EXAMPLES$
 *     This example illustrates the relationship between COUNT and
 *      RECCOUNT():
 *
 *      USE Sales NEW
 *      ? RECCOUNT()                      // Result: 84
 *      //
 *      SET FILTER TO Salesman = "1001"
 *      COUNT TO nRecords
 *      ? nRecords                        // Result: 14
 *      ? RECCOUNT()                      // Result: 84
 *
 *     This example uses an aliased expression to access the number
 *      of records in an unselected work area:
 *
 *      USE Sales NEW
 *      USE Customer NEW
 *      ? RECCOUNT(), Sales->(RECCOUNT())
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      EOF(),LASTREC()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_RECCOUNT( void )
{
   ULONG ulRecCount = 0;

   if( pCurrArea )
   {
      SELF_RECCOUNT( ( AREAP ) pCurrArea->pArea, &ulRecCount );
      ( ( AREAP ) pCurrArea->pArea )->lpExtendInfo->ulRecCount = ulRecCount;
   }
   hb_retnl( ulRecCount );
}

/*  $DOC$
 *  $FUNCNAME$
 *      RECNO()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Return the identity at the position of the record pointer
 *  $SYNTAX$
 *      RECNO() --> Identity
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *     RECNO() returns the identity found at the position of the record
 *   pointer.
 *  $DESCRIPTION$
 *     RECNO() is a database function that returns the identity found at the
 *   current position of the record pointer.  Identity is a unique value
 *   guaranteed by the structure of the data file to reference a specific
 *   record of data file.  The data file need not be a traditional Xbase
 *   file.  Therefore, unlike earlier versions of HARBOUR, the value
 *   returned need not be a numeric data type.
 *
 *     Under all RDDs, RECNO() returns the value at the position of the record
 *   pointer; the data type and other characteristics of this value are
 *   determined by the content of the accessed data and the RDD active in the
 *   current work area.  In an Xbase database this value is the record
 *   number.
 *  $EXAMPLES$
 *   USE Sales VIA "DBFNTX"
 *   .
 *   . < statements >
 *   .
 *   DBGOTOP()
 *   RECNO()            // Returns 1
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      DBGOTO()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_RECNO( void )
{
   PHB_ITEM pRecNo;

   pRecNo = hb_itemPutNL( NULL, 0 );
   if( pCurrArea )
      SELF_RECNO( ( AREAP ) pCurrArea->pArea, pRecNo );
   hb_itemReturn( pRecNo );
   hb_itemRelease( pRecNo );
}

/*  $DOC$
 *  $FUNCNAME$
 *      RECSIZE()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Determine the record length of a database (.dbf) file
 *  $SYNTAX$
 *      RECSIZE() --> nBytes
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *     RECSIZE() returns, as a numeric value, the record length, in bytes, of
 *   the database file open in the current work area.  RECSIZE() returns zero
 *   if no database file is open.
 *  $DESCRIPTION$
 *     RECSIZE() is a database function that determines the length of a record
 *   by summing the lengths of each field then adding one for the DELETED()
 *   status flag.  When this value is multiplied by LASTREC(), the product is
 *   the amount of space occupied by the file's records.
 *
 *     RECSIZE() is useful in programs that perform automatic file backup.
 *   When used in conjunction with DISKSPACE(), the RECSIZE() function can
 *   assist in ensuring that sufficient free space exists on the disk before a
 *   file is stored.
 *
 *     By default, RECSIZE() operates on the currently selected work area.  It
 *   will operate on an unselected work area if you specify it as part of an
 *   aliased expression (see example below).
 *  $EXAMPLES$
 *     The following user-defined function, DbfSize(), uses RECSIZE()
 *      to calculate the size of the current database file:
 *
 *      FUNCTION DbfSize
 *         RETURN ((RECSIZE() * LASTREC()) + HEADER() + 1)
 *
 *     This example illustrates the use of RECSIZE() to determine the
 *      record length of database files open in unselected work areas:
 *
 *      USE Customer NEW
 *      USE Sales NEW
 *      //
 *      ? RECSIZE(), Customer->(RECSIZE())
 *      ? DbfSize(), Customer->(DbfSize())
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *  DISKSPACE(),FIELDNAME(),HEADER(),LASTREC()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_RECSIZE( void )
{
   PHB_ITEM pRecSize;

   if( !pCurrArea )
      hb_retni( 0 );
   else
   {
      pRecSize = hb_itemNew( NULL );
      SELF_INFO( ( AREAP ) pCurrArea->pArea, DBI_GETRECSIZE, pRecSize );
      hb_itemReturn( pRecSize );
      hb_itemRelease( pRecSize );
   }
}

/*  $DOC$
 *  $FUNCNAME$
 *      RLOCK()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Lock the current record in the active work area
 *  $SYNTAX$
 *      RLOCK() --> lSuccess
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *     RLOCK() returns true (.T.) if the record lock is obtained; otherwise, it
 *   returns false (.F.).
 *  $DESCRIPTION$
 *     RLOCK() is a network function that locks the current record, preventing
 *      other users from updating the record until the lock is released.
 *      RLOCK() provides a shared lock, allowing other users read-only access to
 *      the locked record while allowing only the current user to modify it.  A
 *      record lock remains until another record is locked, an UNLOCK is
 *      executed, the current database file is closed, or an FLOCK() is obtained
 *      on the current database file.
 *
 *      For each invocation of RLOCK(), there is one attempt to lock the current
 *      record, and the result is returned as a logical value.  An attempt to
 *      obtain a record lock fails if another user currently has a file or
 *      record lock, or EXCLUSIVE USE of the database file.  An attempt to
 *      RLOCK() in an empty database returns true (.T.).
 *
 *      By default, RLOCK() operates on the currently selected work area.  It
 *      will operate on an unselected work area if you specify it as part of an
 *      aliased expression (see example below).  This feature is useful since
 *      RLOCK() does not automatically attempt a record lock for related files.
 *
 *      As a general rule, RLOCK() operates solely on the current record.  This
 *      includes the following commands:
 *
 *      @...GET
 *
 *      DELETE (single record)
 *
 *      RECALL (single record)
 *
 *      REPLACE (single record)
 *
 *
 *      Notes
 *
 *      SET RELATION: HARBOUR does not automatically lock all
 *      records in the relation chain when you lock the current work area
 *      record.  Also, an UNLOCK has no effect on related work areas.
 *  $EXAMPLES$
 *     This example deletes a record in a network environment, using
 *      RLOCK():
 *
 *      USE Customer INDEX CustName SHARED NEW
 *      SEEK "Smith"
 *      IF FOUND()
 *         IF RLOCK()
 *            DELETE
 *            ? "Smith deleted"
 *         ELSE
 *            ? "Record in use by another"
 *         ENDIF
 *      ELSE
 *         ? "Smith not in Customer file"
 *      ENDIF
 *      CLOSE
 *
 *      This example specifies RLOCK() as an aliased expression to
 *      lock a record in an unselected work area:
 *
 *      USE Sales SHARED NEW
 *      USE Customer SHARED NEW
 *      //
 *      IF !Sales->(RLOCK())
 *         ? "The current Sales record is in use by another"
 *      ENDIF
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      FLOCK()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_RLOCK( void )
{
   DBLOCKINFO pLockInfo;
   PHB_ITEM pRecNo;

   pLockInfo.fResult = FALSE;
   if( pCurrArea )
   {
      pRecNo = hb_itemPutNL( NULL, 0 );
      SELF_RECNO( ( AREAP ) pCurrArea->pArea, pRecNo );
      pLockInfo.itmRecID = pRecNo->item.asLong.value;
      pLockInfo.uiMethod = REC_LOCK;
      SELF_LOCK( ( AREAP ) pCurrArea->pArea, &pLockInfo );
      hb_itemRelease( pRecNo );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, "RLOCK" );

   hb_retl( pLockInfo.fResult );
}

/*  $DOC$
 *  $FUNCNAME$
 *      SELECT()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Determine the work area number of a specified alias
 *  $SYNTAX$
 *      SELECT([<cAlias>]) --> nWorkArea
 *  $ARGUMENTS$
 *      <cAlias> is the target work area alias name.
 *  $RETURNS$
 *      SELECT() returns the work area of the specified alias as a integer
 *      numeric value.
 *  $DESCRIPTION$
 *      SELECT() is a database function that determines the work area number of
 *      an alias.  The number returned can range from zero to 250.  If <cAlias>
 *      is not specified, the current work area number is returned.  If <cAlias>
 *      is specified and the alias does not exist, SELECT() returns zero.
 *
 *      Note:  The SELECT() function and SELECT command specified with an
 *      extended expression argument look somewhat alike.  This shouldn't be a
 *      problem since the SELECT() function is not very useful on a line by
 *      itself
 *  $EXAMPLES$
 *     This example uses SELECT() to determine which work area
 *      USE...NEW selected:
 *
 *      USE Sales NEW
 *      SELECT 1
 *      ? SELECT("Sales")          // Result: 4
 *
 *      To reselect the value returned from the SELECT() function, use
 *      the SELECT command with the syntax, SELECT (<idMemvar>), like this:
 *
 *      USE Sales NEW
 *      nWorkArea:= SELECT()
 *      USE Customer NEW
 *      SELECT (nWorkArea)
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      ALIAS(),USED()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_SELECT( void )
{
   char * szAlias;
   ULONG ulLen;

   szAlias = hb_parc( 1 );
   ulLen = strlen( szAlias );

   if( ulLen == 1 && toupper( szAlias[ 0 ] ) >= 'A' && toupper( szAlias[ 0 ] ) <= 'K' )
      hb_retni( toupper( szAlias[ 0 ] ) - 'A' + 1 );
   else if( ulLen > 0 )
      hb_retni( hb_rddSelect( szAlias ) );
   else
      hb_retni( uiCurrArea );
}

/*  $DOC$
 *  $FUNCNAME$
 *      USED()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Determine whether a database file is in USE
 *  $SYNTAX$
 *      USED() --> lDbfOpen
 *  $ARGUMENTS$
 *
 *  $RETURNS$
 *      USED() returns true (.T.) if there is a database file in USE; otherwise,
 *      it returns false (.F.).
 *  $DESCRIPTION$
 *      USED() is a database function that determines whether there is a
 *      database file in USE in a particular work area.  By default, USED()
 *      operates on the currently selected work area.  It will operate on an
 *      unselected work area if you specify it as part of an aliased expression.
 *  $EXAMPLES$
 *      This example determines whether a database file is in USE in
 *      the current work area:
 *
 *      USE Customer NEW
 *      ? USED()              // Result: .T.
 *      CLOSE
 *      ? USED()              // Result: .F.
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      ALIAS(),SELECT()
 *  $INCLUDE$
 *
 *  $END$
 */

HARBOUR HB_USED( void )
{
   hb_retl( pCurrArea != NULL );
}

/*  $DOC$
 *  $FUNCNAME$
 *      __RDDSETDEFAULT()
 *  $CATEGORY$
 *      Data Base
 *  $ONELINER$
 *      Set or return the default RDD for the application
 *  $SYNTAX$
 *      __RDDSETDEFAULT([<cNewDefaultRDD>])
 *      --> cPreviousDefaultRDD
 *
 *  $ARGUMENTS$
 *      <cNewDefaultRDD> is a character string, the name of the RDD that is
 *      to be made the new default RDD in the application.
 *  $RETURNS$
 *      __RDDSETDEFAULT() returns a character string, cPreviousDefaultRDD, the
 *      name of the previous default driver.  The default driver is the driver
 *      that HARBOUR uses if you do not explicitly specify an RDD with the
 *      VIA clause of the USE command.
 *  $DESCRIPTION$
 *      RDDSETDEFAULT() is an RDD function that sets or returns the name of the
 *      previous default RDD driver and, optionally, sets the current driver to
 *      the new RDD driver specified by cNewDefaultRDD.  If <cNewDefaultDriver>
 *      is not specified, the current default driver name is returned and
 *      continues to be the current default driver.
 *
 *      This function replaces the DBSETDRIVER() function.
 *  $EXAMPLES$
 *      // If the default driver is not DBFNTX, make it the default
 *
 *      IF ( __RDDSETDEFAULT() != "DBFNTX" )
 *           cOldRdd := __RDDSETDEFAULT( "DBFNTX" )
 *      ENDIF
 *  $TESTS$
 *
 *  $STATUS$
 *      R
 *  $COMPLIANCE$
 *
 *  $SEEALSO$
 *      DBSETDRIVER()
 *  $INCLUDE$
 *
 *  $END$
 */

/* NOTE: Same as dbSetDriver() and rddSetDefault(), but doesn't
         throw any error if the driver doesn't exist, this is
         required in the RDDSYS INIT function, since it's not guaranteed
         that the RDD is already registered at that point. [vszakats] */

HARBOUR HB___RDDSETDEFAULT( void )
{
   char * szNewDriver;
   USHORT uiLen;
   char cDriverBuffer[ HARBOUR_MAX_RDD_DRIVERNAME_LENGTH ];

   hb_rddCheck();
   hb_retc( szDefDriver );
   szNewDriver = hb_parc( 1 );
   if( ( uiLen = strlen( szNewDriver ) ) > 0 )
   {
      hb_strncpyUpper( cDriverBuffer, szNewDriver, uiLen );
      szDefDriver = ( char * ) hb_xrealloc( szDefDriver, uiLen + 1 );
      strcpy( szDefDriver, cDriverBuffer );
   }
}
