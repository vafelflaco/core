/*
 * $Id$
 */

/*
 * Harbour Project source code:
 * Base-routines for OOPS system
 *
 * Copyright 1999 Antonio Linares <alinares@fivetech.com>
 * www - http://www.harbour-project.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/).
 *
 * As a special exception, the Harbour Project gives permission for
 * additional uses of the text contained in its release of Harbour.
 *
 * The exception is that, if you link the Harbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the Harbour library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by the Harbour
 * Project under the name Harbour.  If you copy code from other
 * Harbour Project or Free Software Foundation releases into a copy of
 * Harbour, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

/*
 * The following parts are Copyright of the individual authors.
 * www - http://www.harbour-project.org
 *
 * Copyright 1999 Eddie Runia <eddie@runia.com>
 *    :CLASSSEL()
 *    __clsDelMsg()
 *    __clsModMsg()
 *    __clsInstSuper()
 *    __cls_CntClsData()
 *    __cls_CntData()
 *    __cls_DecData()
 *    __cls_IncData()
 *    __objClone()
 *    __objHasMsg()
 *    __objSendMsg()
 *
 * Copyright 1999-2001 Viktor Szakats <viktor.szakats@syenar.hu>
 *    hb___msgEval()
 *    __CLASSNEW()
 *    __CLASSINSTANCE()
 *    __CLASSADD()
 *    __CLASSNAME()
 *    __CLASSSEL() (based on hb___msgClsSel())
 *
 * Copyright 1999 Janica Lubos <janica@fornax.elf.stuba.sk>
 *    hb_clsDictRealloc()
 *
 * Copyright 2000 ( ->07/2000 ) JF. Lefebvre <jfl@mafact.com> & RA. Cuylen <cakiral@altern.org
 *    Multiple inheritence fully implemented
 *    Forwarding, delegating
 *    Data initialisation & Autoinit for Bool and Numeric
 *    Scoping : Protected / exported
 *
 * Copyright 2000 ( 08/2000-> ) JF. Lefebvre <jfl@mafact.com>
 *    hb_clsDictRealloc()   New version
 *    Now support of shared and not shared class data
 *    Multiple datas declaration fully supported
 *
 *    2000 RGlab
 *    Garbage collector fixe
 *
 * Copyright 2001 JF. Lefebvre <jfl@mafact.com>
 *    Super msg corrected
 *    Scoping : working for protected, hidden and readonly
 *    To Many enhancement and correction to give a full list :-)
 *    Improved class(y) compatibility
 *    Improved TopClass compatibility
 *    __CLS_PAR00() (Allow the creation of class wich not autoinherit of the default HBObject)
 *    Adding HB_CLS_ENFORCERO FLAG to disable Write access to RO VAR
 *    outside of Constructors /!\ Could be related to some incompatibility
 *    Added hb_objGetRealClsName to keep a full class tree ( for 99% cases )
 *    Fixed hb_clsIsParent
 *
 *
 *    hb_objGetMthd() & __CLSADDMSG modified to translate the followings operators
 *
 "+"     = __OpPlus
 "-"     = __OpMinus
 "*"     = __OpMult
 "/"     = __OpDivide
 "%"     = __OpMod
 "^"     = __OpPower
 "**"    = __OpPower
 "++"    = __OpInc
 "--"    = __OpDec
 "=="    = __OpEqual
 "="     = __OpEqual (same as "==")
 "!="    = __OpNotEqual
 "<>"    = __OpNotEqual (same as "!=")
 "#"     = __OpNotEqual (same as "!=")
 "<"     = __OpLess
 "<="    = __OpLessEqual
 ">"     = __OpGreater
 ">="    = __OpGreaterEqual
 "$"     = __OpInstring
 "!"     = __OpNot
 ".NOT." = __OpNot (same as "!")
 ".AND." = __OpAnd
 ".OR."  = __OpOr
 ":="    = __OpAssign   ... not tested ...
 "[]"    = __OpArrayIndex
 *
 *
 * See doc/license.txt for licensing terms.
 *
 */

#include "hbvmopt.h"
#include "hbapi.h"
#include "hbapicls.h"
#include "hbstack.h"
#include "hbapierr.h"
#include "hbapiitm.h"
#include "hbvm.h"
#include "hboo.ch"

#include <ctype.h>             /* For toupper() */

/* DEBUG only*/
/* #include <windows.h> */

typedef struct
{
   PHB_ITEM pInitValue;          /* Init Value for data */
   USHORT   uiType;              /* HB_OO_MSG_DATA, HB_OO_MSG_CLASSDATA or HB_OO_MSG_INITIALIZED */
   USHORT   uiData;              /* Item position in instance area or class data */
   USHORT   uiOffset;            /* Supper cast instance are offset */
   USHORT   uiSprClass;          /* The real class where method were defined */
} INITDATA, * PINITDATA;

typedef struct
{
   PHB_DYNS pMessage;            /* Method symbolic name */
   PHB_SYMB pFuncSym;            /* Function symbol */
   USHORT   uiSprClass;          /* Originalclass'handel (super or current class'handel if not herited). */ /*Added by RAC&JF*/
   USHORT   uiScope;             /* Scoping value */
   USHORT   uiData;              /* Item position for instance data, class or shared data (Harbour like, begin from 1) */
   USHORT   uiOffset;            /* position in pInitData for class datas (from 1) or offset to instance area in inherited instance data and supercast messages (from 0) */
#ifndef HB_NO_PROFILER
   ULONG    ulCalls;             /* profiler support */
   ULONG    ulTime;              /* profiler support */
   ULONG    ulRecurse;           /* profiler support */
#endif
} METHOD, * PMETHOD;

typedef struct
{
   char *   szName;           /* Class name */
   PHB_DYNS pClassSym;        /* Class symbolic name */
   PMETHOD  pMethods;         /* Class methods */
   PHB_SYMB pClassFuncSym;    /* Class function symbol */
   PHB_SYMB pFriendModule;    /* Class friend symbols */
   PINITDATA pInitData;       /* Class/instance Initialization data */
   PHB_ITEM pClassDatas;      /* Harbour Array for Class Datas */
   PHB_ITEM pSharedDatas;     /* Harbour Array for Class Shared Datas */
   PHB_ITEM pInlines;         /* Array for inline codeblocks */
   PHB_SYMB * pFriendSyms;    /* Friend functions' symbols */
   ULONG    ulOpFlags;        /* Flags for overloaded operators */
   BOOL     fHasDestructor;   /* has the class destructor message? */
   BOOL     fHasOnError;      /* has the class OnError message? */
   USHORT   uiMethods;        /* Total Method initialised Counter */
   USHORT   uiInitDatas;      /* Total Method initialised Counter */
   USHORT   uiDatas;          /* Total Data Counter */
   USHORT   uiDataFirst;      /* First instance item from this class */
   USHORT   uiFriendSyms;     /* Number of friend function's symbols */
   USHORT   uiFriendModule;   /* Number of friend symbols in pFriendModule */
   USHORT   fLocked;          /* Lock the class agains modifications */
   USHORT   uiHashKey;
} CLASS, * PCLASS;

#define BUCKETBITS      2
#define BUCKETSIZE      ( 1 << BUCKETBITS )
#define BUCKETMASK      ( BUCKETSIZE - 1 )
#define HASHBITS        3
#define HASH_KEY        ( ( 1 << HASHBITS ) - 1 )
#define HASH_KEYMAX     ( 1 << ( 16 - BUCKETBITS ) )
#define hb_clsMthNum(p) ( ( ( ULONG ) (p)->uiHashKey + 1 ) << BUCKETBITS )

#if defined( HB_REAL_BLOCK_SCOPE )
#  undef HB_CLASSY_BLOCK_SCOPE
#elif !defined( HB_CLASSY_BLOCK_SCOPE )
#  define HB_REAL_BLOCK_SCOPE
#endif

#if !defined( HB_CLASSY_BLOCK_SCOPE )
#  define hb_clsSenderOffset()      hb_stackBaseProcOffset( 1 )
#endif


static HARBOUR  hb___msgGetData( void );
static HARBOUR  hb___msgSetData( void );
static HARBOUR  hb___msgGetClsData( void );
static HARBOUR  hb___msgSetClsData( void );
static HARBOUR  hb___msgGetShrData( void );
static HARBOUR  hb___msgSetShrData( void );
static HARBOUR  hb___msgEvalInline( void );
static HARBOUR  hb___msgVirtual( void );
static HARBOUR  hb___msgSuper( void );
static HARBOUR  hb___msgRealClass( void );
static HARBOUR  hb___msgNoMethod( void );
static HARBOUR  hb___msgScopeErr( void );
static HARBOUR  hb___msgNull( void );

static HARBOUR  hb___msgClsH( void );
static HARBOUR  hb___msgClsName( void );
static HARBOUR  hb___msgClsSel( void );
static HARBOUR  hb___msgEval( void );
/* static HARBOUR  hb___msgClass( void ); */
/* static HARBOUR  hb___msgClsParent( void ); */

/*
 * The positions of items in symbol table below have to correspond
 * to HB_OO_OP_* constants in hbapicls.h, [druzus]
 */
static HB_SYMB s_opSymbols[ HB_OO_MAX_OPERATOR + 1 ] = {
   { "__OPPLUS",              {HB_FS_MESSAGE}, {NULL}, NULL },  /* 00 */
   { "__OPMINUS",             {HB_FS_MESSAGE}, {NULL}, NULL },  /* 01 */
   { "__OPMULT",              {HB_FS_MESSAGE}, {NULL}, NULL },  /* 02 */
   { "__OPDIVIDE",            {HB_FS_MESSAGE}, {NULL}, NULL },  /* 03 */
   { "__OPMOD",               {HB_FS_MESSAGE}, {NULL}, NULL },  /* 04 */
   { "__OPPOWER",             {HB_FS_MESSAGE}, {NULL}, NULL },  /* 05 */
   { "__OPINC",               {HB_FS_MESSAGE}, {NULL}, NULL },  /* 06 */
   { "__OPDEC",               {HB_FS_MESSAGE}, {NULL}, NULL },  /* 07 */
   { "__OPEQUAL",             {HB_FS_MESSAGE}, {NULL}, NULL },  /* 08 */
   { "__OPEXACTEQUAL",        {HB_FS_MESSAGE}, {NULL}, NULL },  /* 09 */
   { "__OPNOTEQUAL",          {HB_FS_MESSAGE}, {NULL}, NULL },  /* 10 */
   { "__OPLESS",              {HB_FS_MESSAGE}, {NULL}, NULL },  /* 11 */
   { "__OPLESSEQUAL",         {HB_FS_MESSAGE}, {NULL}, NULL },  /* 12 */
   { "__OPGREATER",           {HB_FS_MESSAGE}, {NULL}, NULL },  /* 13 */
   { "__OPGREATEREQUAL",      {HB_FS_MESSAGE}, {NULL}, NULL },  /* 14 */
   { "__OPADDIGN",            {HB_FS_MESSAGE}, {NULL}, NULL },  /* 15 */
   { "__OPINSTRING",          {HB_FS_MESSAGE}, {NULL}, NULL },  /* 16 */
   { "__OPNOT",               {HB_FS_MESSAGE}, {NULL}, NULL },  /* 17 */
   { "__OPAND",               {HB_FS_MESSAGE}, {NULL}, NULL },  /* 18 */
   { "__OPOR",                {HB_FS_MESSAGE}, {NULL}, NULL },  /* 19 */
   { "__OPARRAYINDEX",        {HB_FS_MESSAGE}, {NULL}, NULL },  /* 20 */
   { "__ENUMINDEX",           {HB_FS_MESSAGE}, {NULL}, NULL },  /* 21 */
   { "__ENUMBASE",            {HB_FS_MESSAGE}, {NULL}, NULL },  /* 22 */
   { "__ENUMVALUE",           {HB_FS_MESSAGE}, {NULL}, NULL },  /* 23 */
   { "__ENUMSTART",           {HB_FS_MESSAGE}, {NULL}, NULL },  /* 24 */
   { "__ENUMSKIP",            {HB_FS_MESSAGE}, {NULL}, NULL },  /* 25 */
   { "__ENUMSTOP",            {HB_FS_MESSAGE}, {NULL}, NULL }   /* 26 */
};

static HB_SYMB s___msgDestructor = { "__msgDestructor", {HB_FS_MESSAGE}, {NULL},               NULL };
static HB_SYMB s___msgOnError    = { "__msgOnError",    {HB_FS_MESSAGE}, {NULL},               NULL };

static HB_SYMB s___msgNew        = { "NEW",             {HB_FS_MESSAGE}, {NULL},               NULL };

static HB_SYMB s___msgSetData    = { "__msgSetData",    {HB_FS_MESSAGE}, {hb___msgSetData},    NULL };
static HB_SYMB s___msgGetData    = { "__msgGetData",    {HB_FS_MESSAGE}, {hb___msgGetData},    NULL };
static HB_SYMB s___msgSetClsData = { "__msgSetClsData", {HB_FS_MESSAGE}, {hb___msgSetClsData}, NULL };
static HB_SYMB s___msgGetClsData = { "__msgGetClsData", {HB_FS_MESSAGE}, {hb___msgGetClsData}, NULL };
static HB_SYMB s___msgSetShrData = { "__msgSetShrData", {HB_FS_MESSAGE}, {hb___msgSetShrData}, NULL };
static HB_SYMB s___msgGetShrData = { "__msgGetShrData", {HB_FS_MESSAGE}, {hb___msgGetShrData}, NULL };
static HB_SYMB s___msgEvalInline = { "__msgEvalInline", {HB_FS_MESSAGE}, {hb___msgEvalInline}, NULL };
static HB_SYMB s___msgVirtual    = { "__msgVirtual",    {HB_FS_MESSAGE}, {hb___msgVirtual},    NULL };
static HB_SYMB s___msgSuper      = { "__msgSuper",      {HB_FS_MESSAGE}, {hb___msgSuper},      NULL };
static HB_SYMB s___msgRealClass  = { "__RealClass",     {HB_FS_MESSAGE}, {hb___msgRealClass},  NULL };
static HB_SYMB s___msgNoMethod   = { "__msgNoMethod",   {HB_FS_MESSAGE}, {hb___msgNoMethod},   NULL };
static HB_SYMB s___msgScopeErr   = { "__msgScopeErr",   {HB_FS_MESSAGE}, {hb___msgScopeErr},   NULL };

static HB_SYMB s___msgClassName  = { "CLASSNAME",       {HB_FS_MESSAGE}, {hb___msgClsName},    NULL };
static HB_SYMB s___msgClassH     = { "CLASSH",          {HB_FS_MESSAGE}, {hb___msgClsH},       NULL };
static HB_SYMB s___msgClassSel   = { "CLASSSEL",        {HB_FS_MESSAGE}, {hb___msgClsSel},     NULL };
static HB_SYMB s___msgEval       = { "EVAL",            {HB_FS_MESSAGE}, {hb___msgEval},       NULL };
static HB_SYMB s___msgExec       = { "EXEC",            {HB_FS_MESSAGE}, {hb___msgNull},       NULL };
static HB_SYMB s___msgName       = { "NAME",            {HB_FS_MESSAGE}, {hb___msgNull},       NULL };
/*
static HB_SYMB s___msgClsParent  = { "ISDERIVEDFROM",   {HB_FS_MESSAGE}, {hb___msgClsParent},  NULL };
static HB_SYMB s___msgClass      = { "CLASS",           {HB_FS_MESSAGE}, {hb___msgClass},      NULL };
*/

/* Default enumerator methods (FOR EACH) */
static HB_SYMB s___msgEnumIndex  = { "__ENUMINDEX",     {HB_FS_MESSAGE}, {hb___msgNull},       NULL };
static HB_SYMB s___msgEnumBase   = { "__ENUMBASE",      {HB_FS_MESSAGE}, {hb___msgNull},       NULL };
static HB_SYMB s___msgEnumValue  = { "__ENUMVALUE",     {HB_FS_MESSAGE}, {hb___msgNull},       NULL };

/* WITH OBJECT base value access/asign methods (:__withobject) */
static HB_SYMB s___msgWithObjectPush = { "__WITHOBJECT",  {HB_FS_MESSAGE}, {hb___msgNull},       NULL };
static HB_SYMB s___msgWithObjectPop  = { "___WITHOBJECT", {HB_FS_MESSAGE}, {hb___msgNull},       NULL };

static PCLASS   s_pClasses     = NULL;
static USHORT   s_uiClasses    = 0;

/* ================================================ */

static USHORT hb_clsMsgBucket( PHB_DYNS pMsg, USHORT uiMask )
{
   /*
    * we can use PHB_DYNS address as base for hash key.
    * This value is perfectly unique and we do not need anything more
    * but it's not continuous so we will have to add dynamic BUCKETSIZE
    * modification to be 100% sure that we can resolve all symbol name
    * conflicts (though even without it it's rather theoretical problem).
    * [druzus]
    */

    /* Safely divide it by 16 - it's minimum memory allocated for single
     * HB_DYNS structure
     */
    /* 
    return ( ( USHORT ) ( ( HB_PTRDIFF ) pMsg >> 4 ) & uiMask ) << BUCKETBITS;
    */

   /* Using continuous symbol numbers we are 100% sure that we will cover
    * the whole 16bit area and we will never have any problems until number
    * of symbols is limited to 2^16. [druzus]
    */

   return ( pMsg->uiSymNum & uiMask ) << BUCKETBITS;
}

/*
 * hb_clsDictRealloc( PCLASS )
 *
 * Realloc (widen) class
 */
static BOOL hb_clsDictRealloc( PCLASS pClass )
{
   ULONG ulNewHashKey, ulLimit, ul;
   PMETHOD pNewMethods;

   HB_TRACE(HB_TR_DEBUG, ("hb_clsDictRealloc(%p)", pClass));

   ulNewHashKey = ( ULONG ) pClass->uiHashKey + 1;
   ulLimit = ulNewHashKey << BUCKETBITS;

   do
   {
      ulNewHashKey <<= 1;
      if( ulNewHashKey > HASH_KEYMAX )
      {
         hb_errInternal( 9999, "Not able to realloc classmessage! __clsDictRealloc", NULL, NULL );
         return FALSE;
      }

      pNewMethods = ( PMETHOD ) hb_xgrab( ( ulNewHashKey << BUCKETBITS ) * sizeof( METHOD ) );
      memset( pNewMethods, 0, ( ulNewHashKey << BUCKETBITS ) * sizeof( METHOD ) );

      for( ul = 0; ul < ulLimit; ul++ )
      {
         PHB_DYNS pMessage = ( PHB_DYNS ) pClass->pMethods[ ul ].pMessage;

         if( pMessage )
         {
            PMETHOD pMethod = pNewMethods + hb_clsMsgBucket( pMessage, ulNewHashKey - 1 );
            USHORT uiBucket = BUCKETSIZE;

            do
            {
               if( ! pMethod->pMessage ) /* this message position is empty */
               {
                  memcpy( pMethod, pClass->pMethods + ul, sizeof( METHOD ) );
                  break;
               }
               ++pMethod;
            } while( --uiBucket );

            /* Not enough go back to the beginning */
            if( ! uiBucket )
            {
               hb_xfree( pNewMethods );
               break;
            }
         }
      }
   }
   while( ul < ulLimit );

   pClass->uiHashKey = ( USHORT ) ( ulNewHashKey - 1 );
   hb_xfree( pClass->pMethods );
   pClass->pMethods = pNewMethods;

   return TRUE;
}

static void hb_clsDictInit( PCLASS pClass, USHORT uiHashKey )
{
   ULONG ulSize;

   HB_TRACE(HB_TR_DEBUG, ("hb_clsDictInit(%p,%hu)", pClass, uiHashKey));

   ulSize = ( ( ( ULONG ) uiHashKey + 1 ) << BUCKETBITS ) * sizeof( METHOD );
   pClass->uiHashKey = uiHashKey;
   pClass->pMethods = ( PMETHOD ) hb_xgrab( ulSize );
   memset( pClass->pMethods, 0, ulSize );
}

static void hb_clsCopyClass( PCLASS pClsDst, PCLASS pClsSrc )
{
   PMETHOD pMethod;
   ULONG ulLimit;

   HB_TRACE(HB_TR_DEBUG, ("hb_clsCopyClass(%p,%p)", pClsDst, pClsSrc));

   hb_clsDictInit( pClsDst, pClsSrc->uiHashKey );
   pClsDst->fHasOnError = pClsSrc->fHasOnError;
   pClsDst->fHasDestructor = pClsSrc->fHasDestructor;

   /* CLASS DATA Not Shared ( new array, new value ) */
   pClsDst->pClassDatas  = hb_arrayClone( pClsSrc->pClassDatas );
   /* do not copy shared data array - just simply create new one */
   pClsDst->pSharedDatas = hb_itemArrayNew( 0 );
   pClsDst->pInlines = hb_arrayClone( pClsSrc->pInlines );
   pClsDst->uiDatas = pClsSrc->uiDatas;
   pClsDst->ulOpFlags = pClsSrc->ulOpFlags;

   if( pClsSrc->uiInitDatas )
   {
      ULONG ulSize = ( ULONG ) pClsSrc->uiInitDatas * sizeof( INITDATA );
      USHORT uiData;

      pClsDst->uiInitDatas = pClsSrc->uiInitDatas;
      pClsDst->pInitData = ( PINITDATA ) hb_xgrab( ulSize );
      memcpy( pClsDst->pInitData, pClsSrc->pInitData, ulSize );
      for( uiData = 0; uiData < pClsDst->uiInitDatas; ++uiData )
      {
         if( pClsDst->pInitData[ uiData ].uiType == HB_OO_MSG_INITIALIZED )
            pClsDst->pInitData[ uiData ].uiType = HB_OO_MSG_CLASSDATA;
         pClsDst->pInitData[ uiData ].pInitValue =
                        hb_itemNew( pClsDst->pInitData[ uiData ].pInitValue );
      }
   }

   ulLimit = hb_clsMthNum( pClsDst );
   memcpy( pClsDst->pMethods, pClsSrc->pMethods, ulLimit * sizeof( METHOD ) );
   pClsDst->uiMethods = pClsSrc->uiMethods;

   pMethod = pClsDst->pMethods;
   do
   {
      if( pMethod->pMessage )
      {
         pMethod->uiScope |= HB_OO_CLSTP_SUPER;
      }
      ++pMethod;
   }
   while( --ulLimit );
}

static PMETHOD hb_clsFindMsg( PCLASS pClass, PHB_DYNS pMsg )
{
   PMETHOD pMethod;
   USHORT uiBucket;

   HB_TRACE(HB_TR_DEBUG, ("hb_clsFindMsg(%p,%p)", pClass, pMsg));

   pMethod = pClass->pMethods + hb_clsMsgBucket( pMsg, pClass->uiHashKey );
   uiBucket = BUCKETSIZE;

   do
   {
      if( pMethod->pMessage == pMsg )
         return pMethod;
      ++pMethod;
   }
   while( --uiBucket );

   return NULL;
}

static PMETHOD hb_clsAllocMsg( PCLASS pClass, PHB_DYNS pMsg )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_clsAllocMsg(%p,%p)", pClass, pMsg));

   do
   {
      PMETHOD pMethod = pClass->pMethods + hb_clsMsgBucket( pMsg, pClass->uiHashKey );
      USHORT uiBucket = BUCKETSIZE;

      do
      {
         if( ! pMethod->pMessage || pMethod->pMessage == pMsg )
            return pMethod;
         ++pMethod;
      }
      while( --uiBucket );
   }
   while( hb_clsDictRealloc( pClass ) );

   return NULL;
}

static BOOL hb_clsClearMsg( PCLASS pClass, PMETHOD pMethod )
{
   if( pMethod->pFuncSym == &s___msgEvalInline )
   {  /* INLINE method deleted, delete INLINE block */
      hb_itemClear( hb_arrayGetItemPtr( pClass->pInlines, pMethod->uiData ) );
   }

   return TRUE;
}

static void hb_clsFreeMsg( PCLASS pClass, PHB_DYNS pMsg )
{
   PMETHOD pMethod;
   USHORT uiBucket;

   HB_TRACE(HB_TR_DEBUG, ("hb_clsFreeMsg(%p,%p)", pClass, pMsg));

   pMethod = pClass->pMethods + hb_clsMsgBucket( pMsg, pClass->uiHashKey );
   uiBucket = BUCKETSIZE;

   do
   {
      if( pMethod->pMessage == pMsg )
      {
         if( hb_clsClearMsg( pClass, pMethod ) )
         {
            /* Move messages */
            while( --uiBucket )
            {
               memcpy( pMethod, pMethod + 1, sizeof( METHOD ) );
               pMethod++;
            }
            memset( pMethod, 0, sizeof( METHOD ) );
            pClass->uiMethods--;       /* Decrease number of messages */
         }
         return;
      }
      ++pMethod;
   }
   while( --uiBucket );
}

static USHORT hb_clsAddInitValue( PCLASS pClass, PHB_ITEM pItem,
                                  USHORT uiType, USHORT uiData,
                                  USHORT uiOffset, USHORT uiSprClass )
{
   PINITDATA pInitData;

   HB_TRACE(HB_TR_DEBUG, ("hb_clsAddInitValue(%p,%p,%hu,%hu,%hu,%hu)", pClass, pItem, uiType, uiData, uiOffset, uiSprClass));

   if( ! pItem || HB_IS_NIL( pItem ) )
      return 0;

   if( ! pClass->uiInitDatas )
   {
      pClass->pInitData = ( PINITDATA ) hb_xgrab( sizeof( INITDATA ) );
      pInitData = pClass->pInitData + pClass->uiInitDatas++;
   }
   else
   {
      USHORT ui = pClass->uiInitDatas;
      pInitData = pClass->pInitData;
      do
      {
         if( pInitData->uiType == uiType &&
             pInitData->uiData + pInitData->uiOffset == uiData + uiOffset )
         {
            hb_itemRelease( pInitData->pInitValue );
            break;
         }
         ++pInitData;
      }
      while( --ui );

      if( ui == 0 )
      {
         pClass->pInitData = ( PINITDATA ) hb_xrealloc( pClass->pInitData,
                  ( ULONG ) ( pClass->uiInitDatas + 1 ) * sizeof( INITDATA ) );
         pInitData = pClass->pInitData + pClass->uiInitDatas++;
      }
   }

   pInitData->pInitValue = hb_itemClone( pItem );
   pInitData->uiType = uiType;
   pInitData->uiData = uiData;
   pInitData->uiOffset = uiOffset;
   pInitData->uiSprClass = uiSprClass;

   return pClass->uiInitDatas;
}

static BOOL hb_clsIsFriendSymbol( PCLASS pClass, PHB_SYMB pSym )
{
   USHORT uiCount;

   HB_TRACE(HB_TR_DEBUG, ("hb_clsIsFriendSymbol(%p,%p)", pClass, pSym));

   if( pSym >= pClass->pFriendModule &&
       pSym < pClass->pFriendModule + pClass->uiFriendModule )
      return TRUE;

   for( uiCount = 0; uiCount < pClass->uiFriendSyms; ++uiCount )
   {
      if( pClass->pFriendSyms[ uiCount ] == pSym )
         return TRUE;
   }

   return FALSE;
}

static void hb_clsAddFriendSymbol( PCLASS pClass, PHB_SYMB pSym )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_clsAddFriendSymbol(%p,%p)", pClass, pSym));

   if( ! hb_clsIsFriendSymbol( pClass, pSym ) )
   {
      if( pClass->uiFriendSyms == 0 )
      {
         pClass->pFriendSyms = ( PHB_SYMB * ) hb_xgrab( sizeof( PHB_SYMB ) );
         pClass->pFriendSyms[ 0 ] = pSym;
         pClass->uiFriendSyms++;
      }
      else 
      {
         pClass->pFriendSyms = ( PHB_SYMB * ) hb_xrealloc( pClass->pFriendSyms,
                           ( pClass->uiFriendSyms + 1 ) * sizeof( PHB_SYMB ) );
         pClass->pFriendSyms[ pClass->uiFriendSyms++ ] = pSym;
      }
   }
}

/*
 * initialize Classy/OO system at HVM startup
 */
void hb_clsInit( void )
{
   PHB_SYMB pOpSym;
   USHORT uiOperator;

   HB_TRACE(HB_TR_DEBUG, ("hb_clsInit()"));

   for( uiOperator = 0, pOpSym = s_opSymbols; uiOperator <= HB_OO_MAX_OPERATOR;
        ++uiOperator, ++pOpSym )
   {
      pOpSym->pDynSym = hb_dynsymGetCase( pOpSym->szName );
   }

   s___msgDestructor.pDynSym  = hb_dynsymGetCase( s___msgDestructor.szName );
   s___msgOnError.pDynSym     = hb_dynsymGetCase( s___msgOnError.szName );

   s___msgClassName.pDynSym   = hb_dynsymGetCase( s___msgClassName.szName );  /* Standard messages        */
   s___msgClassH.pDynSym      = hb_dynsymGetCase( s___msgClassH.szName );     /* Not present in classdef. */
   s___msgClassSel.pDynSym    = hb_dynsymGetCase( s___msgClassSel.szName );
   s___msgEval.pDynSym        = hb_dynsymGetCase( s___msgEval.szName );
   s___msgExec.pDynSym        = hb_dynsymGetCase( s___msgExec.szName );
   s___msgName.pDynSym        = hb_dynsymGetCase( s___msgName.szName );
   s___msgNew.pDynSym         = hb_dynsymGetCase( s___msgNew.szName );
/*
   s___msgClsParent.pDynSym   = hb_dynsymGetCase( s___msgClsParent.szName );
   s___msgClass.pDynSym       = hb_dynsymGetCase( s___msgClass.szName );
*/
   s___msgEnumIndex.pDynSym   = hb_dynsymGetCase( s___msgEnumIndex.szName );
   s___msgEnumBase.pDynSym    = hb_dynsymGetCase( s___msgEnumBase.szName );
   s___msgEnumValue.pDynSym   = hb_dynsymGetCase( s___msgEnumValue.szName );

   s___msgWithObjectPush.pDynSym = hb_dynsymGetCase( s___msgWithObjectPush.szName );
   s___msgWithObjectPop.pDynSym  = hb_dynsymGetCase( s___msgWithObjectPop.szName );
}

/*
 * hb_clsRelease( <pClass> )
 *
 * Release a class from memory
 */
static void hb_clsRelease( PCLASS pClass )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_clsRelease(%p)", pClass));

   if( pClass->uiInitDatas )
   {
      USHORT ui = pClass->uiInitDatas;
      PINITDATA pInitData = pClass->pInitData;

      do
      {
         hb_itemRelease( pInitData->pInitValue );
         ++pInitData;
      }
      while( --ui );
      hb_xfree( pClass->pInitData );
   }

   if( pClass->szName )
      hb_xfree( pClass->szName );
   if( pClass->pMethods )
      hb_xfree( pClass->pMethods );
   if( pClass->uiFriendSyms )
      hb_xfree( pClass->pFriendSyms );
   if( pClass->pClassDatas )
      hb_itemRelease( pClass->pClassDatas );
   if( pClass->pSharedDatas )
      hb_itemRelease( pClass->pSharedDatas );
   if( pClass->pInlines )
      hb_itemRelease( pClass->pInlines );
}


/*
 * hb_clsReleaseAll()
 *
 * Release all classes
 */
void hb_clsReleaseAll( void )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_clsReleaseAll()"));

   if( s_uiClasses )
   {
      USHORT uiClass = s_uiClasses;

      /* It blocks destructor execution - don't move. [druzus] */
      s_uiClasses = 0;

      do
      {
         hb_clsRelease( &s_pClasses[ --uiClass ] );
      }
      while( uiClass );

      hb_xfree( s_pClasses );
      s_pClasses  = NULL;
   }
}

/* Mark all internal data as used so it will not be released by the
 * garbage collector
 */

void hb_clsIsClassRef( void )
{
   /*
    * All internal items are allocated with hb_itemNew()
    * GC knows them and scan itself so it's not necessary
    * to repeat scanning here [druzus].
    */
#if 0
   USHORT uiClass = s_uiClasses;
   PCLASS pClass = s_pClasses;

   HB_TRACE(HB_TR_DEBUG, ("hb_clsIsClassRef()"));

   while( uiClass-- )
   {
      if( pClass->pInlines )
         hb_gcItemRef( pClass->pInlines );

      if( pClass->pClassDatas )
         hb_gcItemRef( pClass->pClassDatas );

      if( pClass->pSharedDatas )
         hb_gcItemRef( pClass->pSharedDatas );

      if( pClass->uiInitDatas )
      {
         USHORT ui = pClass->uiInitDatas;
         PINITDATA pInitData = pClass->pInitData;

         do
         {
            if( HB_IS_GCITEM( pInitData->pInitValue ) )
               hb_gcItemRef( pInitData->pInitValue );
            ++pInitData;
         }
         while( --ui );
      }
      ++pClass;
   }
#endif
}

static BOOL hb_clsHasParent( PCLASS pClass, PHB_DYNS pParentSym )
{
   PMETHOD pMethod = hb_clsFindMsg( pClass, pParentSym );

   return pMethod && pMethod->pFuncSym == &s___msgSuper;
}

static USHORT hb_clsParentInstanceOffset( PCLASS pClass, PHB_DYNS pParentSym )
{
   PMETHOD pMethod = hb_clsFindMsg( pClass, pParentSym );

   return ( pMethod && pMethod->pFuncSym == &s___msgSuper ) ? pMethod->uiOffset : 0;
}

HB_EXPORT BOOL hb_clsIsParent( USHORT uiClass, char * szParentName )
{
   if( uiClass && uiClass <= s_uiClasses )
   {
      PCLASS pClass = &s_pClasses[ uiClass - 1 ];

      if( strcmp( pClass->szName, szParentName ) == 0 )
         return TRUE;
      else
      {
         PHB_DYNS pMsg = hb_dynsymFindName( szParentName );

         if( pMsg )
            return hb_clsHasParent( &s_pClasses[ uiClass - 1 ], pMsg );
      }
   }

   return FALSE;
}

HB_EXPORT USHORT hb_objGetClass( PHB_ITEM pItem )
{
   if( pItem && HB_IS_ARRAY( pItem ) )
      return pItem->item.asArray.value->uiClass;
   else
      return 0;
}

/* ================================================ */

/*
 * Get the class name of an object
 */
HB_EXPORT char * hb_objGetClsName( PHB_ITEM pObject )
{
   char * szClassName;

   HB_TRACE(HB_TR_DEBUG, ("hb_objGetClsName(%p)", pObject));

   if( HB_IS_ARRAY( pObject ) )
   {
      if( ! pObject->item.asArray.value->uiClass )
         szClassName = "ARRAY";
      else
         szClassName =
               s_pClasses[ pObject->item.asArray.value->uiClass - 1 ].szName;
   }
   else                                         /* built in types */
   {
      switch( pObject->type )
      {
         case HB_IT_NIL:
            szClassName = "NIL";
            break;

         case HB_IT_STRING:
         case HB_IT_MEMO:
            szClassName = "CHARACTER";
            break;

         case HB_IT_BLOCK:
            szClassName = "BLOCK";
            break;

         case HB_IT_DATE:
            szClassName = "DATE";
            break;

         case HB_IT_INTEGER:
         case HB_IT_LONG:
         case HB_IT_DOUBLE:
            szClassName = "NUMERIC";
            break;

         case HB_IT_LOGICAL:
            szClassName = "LOGICAL";
            break;

         case HB_IT_POINTER:
            szClassName = "POINTER";
            break;

         case HB_IT_SYMBOL:
            szClassName = "SYMBOL";
            break;

         default:
            szClassName = "UNKNOWN";
            break;
      }
   }

   return szClassName;
}

HB_EXPORT char * hb_clsName( USHORT uiClass )
{
   if( uiClass && uiClass <= s_uiClasses )
      return s_pClasses[ uiClass - 1 ].szName;
   else
      return NULL;
}

/*
 * Get the real class name of an object message
 * Will return the class name from wich the message is inherited in case
 * of inheritance.
 */
HB_EXPORT char * hb_objGetRealClsName( PHB_ITEM pObject, char * szName )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_objGetrealClsName(%p)", pObject));

   if( HB_IS_OBJECT( pObject ) )
   {
      USHORT uiClass;

      uiClass = pObject->item.asArray.value->uiClass;
      if( uiClass && uiClass <= uiClass )
      {
         PHB_DYNS pMsg = hb_dynsymFindName( szName );

         if( pMsg )
         {
            PMETHOD pMethod = hb_clsFindMsg( &s_pClasses[ uiClass - 1 ], pMsg );
            if( pMethod )
               uiClass = pMethod->uiSprClass;
         }
         if( uiClass && uiClass <= s_uiClasses )
            return s_pClasses[ uiClass - 1 ].szName;
      }
   }

   return hb_objGetClsName( pObject );
}

/*
 * return real function name ignoring aliasing
 */
char * hb_clsRealMethodName( void )
{
   LONG lOffset = hb_stackBaseProcOffset( 1 );
   char * szName = NULL;

   if( lOffset >=0 )
   {
      PHB_STACK_STATE pStack = hb_stackItem( lOffset )->item.asSymbol.stackstate;

      if( pStack->uiClass && pStack->uiClass <= s_uiClasses )
      {
         PCLASS pClass = &s_pClasses[ pStack->uiClass - 1 ];

         if( ( ULONG ) pStack->uiMethod < hb_clsMthNum( pClass ) )
         {
            PMETHOD pMethod = pClass->pMethods + pStack->uiMethod;

            if( pMethod->pMessage )
               szName = pMethod->pMessage->pSymbol->szName;
         }
      }
   }
   return szName;
}

#if defined( HB_CLASSY_BLOCK_SCOPE )
static LONG hb_clsSenderOffset( void )
{
   LONG lOffset = hb_stackBaseProcOffset( 1 );

   if( lOffset >=0 )
   {
      /* Is it inline method? */
      if( lOffset > 0 && HB_IS_BLOCK( hb_stackItem( lOffset + 1 ) ) &&
          ( hb_stackItem( lOffset )->item.asSymbol.value == &hb_symEval ||
            hb_stackItem( lOffset )->item.asSymbol.value->pDynSym ==
            s___msgEval.pDynSym ) )
      {
         lOffset = hb_stackItem( lOffset )->item.asSymbol.stackstate->lBaseItem;

         /* I do not like it but Class(y) makes sth like that. [druzus] */
         while( lOffset > 0 &&
                hb_stackItem( lOffset )->item.asSymbol.stackstate->uiClass == 0 )
            lOffset = hb_stackItem( lOffset )->item.asSymbol.stackstate->lBaseItem;
      }
      return lOffset;
   }
   return -1;
}
#endif

#if 0
static USHORT hb_clsSenderClasss( void )
{
   LONG lOffset = hb_clsSenderOffset();

   if( lOffset >=0 )
      return hb_stackItem( lOffset )->item.asSymbol.stackstate->uiClass;
   else
      return 0;
}
#endif

static USHORT hb_clsSenderMethodClasss( void )
{
   LONG lOffset = hb_clsSenderOffset();

   if( lOffset >=0 )
   {
      PHB_STACK_STATE pStack = hb_stackItem( lOffset )->item.asSymbol.stackstate;

      if( pStack->uiClass )
         return ( s_pClasses[ pStack->uiClass - 1 ].pMethods +
                  pStack->uiMethod )->uiSprClass;
   }
   return 0;
}

static PHB_SYMB hb_clsSenderSymbol( void )
{
   PHB_SYMB pSym = NULL;
   LONG lOffset = hb_clsSenderOffset();

   if( lOffset >=0 )
   {
      pSym = hb_stackItem( lOffset )->item.asSymbol.value;

      if( pSym == &hb_symEval || pSym->pDynSym == s___msgEval.pDynSym )
      {
         PHB_ITEM pBlock = hb_stackItem( lOffset + 1 );

         if( HB_IS_BLOCK( pBlock ) )
            pSym = pBlock->item.asBlock.value->pDefSymb;
      }
   }

   return hb_vmGetRealFuncSym( pSym );
}

static USHORT hb_clsSenderObjectClasss( void )
{
   LONG lOffset = hb_clsSenderOffset();

   if( lOffset >=0 )
   {
      PHB_ITEM pSender = hb_stackItem( lOffset + 1 );

      if( HB_IS_ARRAY( pSender ) )
         return pSender->item.asArray.value->uiClass;
   }
   return 0;
}

static PHB_SYMB hb_clsValidScope( PMETHOD pMethod, PHB_STACK_STATE pStack )
{
   if( pMethod->uiScope & ( HB_OO_CLSTP_HIDDEN | HB_OO_CLSTP_PROTECTED |
                            HB_OO_CLSTP_OVERLOADED ) )
   {
      USHORT uiSenderClass = hb_clsSenderMethodClasss();

      if( uiSenderClass )
      {
         if( uiSenderClass == pMethod->uiSprClass )
            return pMethod->pFuncSym;

         /*
          * Warning!!! Friends cannot access overloaded non virtual methods.
          * This feature is available _ONLY_ for real class members, [druzus]
          */
         if( pMethod->uiScope & HB_OO_CLSTP_OVERLOADED )
         {
            PCLASS pClass = &s_pClasses[ uiSenderClass - 1 ];
            PMETHOD pHiddenMthd = hb_clsFindMsg( pClass, pMethod->pMessage );

            if( pHiddenMthd && pHiddenMthd->uiSprClass == uiSenderClass &&
                pHiddenMthd->uiScope & HB_OO_CLSTP_NONVIRTUAL )
            {
               pStack->uiClass = uiSenderClass;
               pStack->uiMethod = ( USHORT ) ( pHiddenMthd - pClass->pMethods );
               return pHiddenMthd->pFuncSym;
            }
         }

         if( pMethod->uiScope & HB_OO_CLSTP_HIDDEN )
         {
            if( ! hb_clsIsFriendSymbol( &s_pClasses[ pStack->uiClass - 1 ],
                              s_pClasses[ uiSenderClass - 1 ].pClassFuncSym ) )
               return &s___msgScopeErr;
         }
         else if( pMethod->uiScope & HB_OO_CLSTP_PROTECTED &&
             ! hb_clsHasParent( &s_pClasses[ pStack->uiClass - 1 ],
                                s_pClasses[ uiSenderClass - 1 ].pClassSym ) &&
             ! hb_clsHasParent( &s_pClasses[ uiSenderClass - 1 ],
                                s_pClasses[ pStack->uiClass - 1 ].pClassSym ) &&
             ! hb_clsIsFriendSymbol( &s_pClasses[ pStack->uiClass - 1 ],
                                     s_pClasses[ uiSenderClass - 1 ].pClassFuncSym ) )
            return &s___msgScopeErr;
      }
      else if( pMethod->uiScope & ( HB_OO_CLSTP_HIDDEN | HB_OO_CLSTP_PROTECTED ) )
      {
         if( ! hb_clsIsFriendSymbol( &s_pClasses[ pStack->uiClass - 1 ],
                                     hb_clsSenderSymbol() ) )
            return &s___msgScopeErr;
      }
   }

   return pMethod->pFuncSym;
}

/*
 * <pFuncSym> = hb_objGetMethod( <pObject>, <pMessage>, <pStackState> )
 *
 * Internal function to the function pointer of a message of an object
 */
PHB_SYMB hb_objGetMethod( PHB_ITEM pObject, PHB_SYMB pMessage,
                          PHB_STACK_STATE pStack )
{
   PCLASS pClass = NULL;
   PHB_DYNS pMsg;

   HB_TRACE(HB_TR_DEBUG, ("hb_objGetMethod(%p, %p, %p)", pObject, pMessage, pStack));

   pMsg = pMessage->pDynSym;

   if( HB_IS_ARRAY( pObject ) )
   {
      USHORT uiClass = pObject->item.asArray.value->uiClass;

      if( pStack )
      {
         pStack->uiClass = uiClass;
         if( pObject->item.asArray.value->uiPrevCls )
         {
            /*
             * Copy real object - do not move! the same super casted
             * object can be used more then once and we mustn't destroy it.
             * We can safely use hb_stackReturnItem() here.
             */
            hb_itemCopy( hb_stackReturnItem(), pObject->item.asArray.value->pItems );
            /* move real object back to the stack */
            hb_itemMove( pObject, hb_stackReturnItem() );
         }
      }

      if( uiClass && uiClass <= s_uiClasses )
      {
         PMETHOD pMethod;

         pClass  = &s_pClasses[ uiClass - 1 ];
         pMethod = hb_clsFindMsg( pClass, pMsg );
         if( pMethod )
         {
            if( pStack )
            {
               pStack->uiMethod = ( USHORT ) ( pMethod - pClass->pMethods );
               return hb_clsValidScope( pMethod, pStack );
            }
            return pMethod->pFuncSym;
         }
      }
   }
   else if( HB_IS_BLOCK( pObject ) )
   {
      if( pMessage == &hb_symEval )
         return pMessage;
      else if( pMsg == s___msgEval.pDynSym )
         return &hb_symEval;
   }
   else if( HB_IS_BYREF( pObject ) )
   {
      if( pStack )
      {
         /* method of enumerator variable from FOR EACH statement
          */
         PHB_ITEM pEnum = hb_itemUnRefOnce( pObject );

         if( HB_IS_ENUM( pEnum ) )
         {
            /*
             * Do actions here - we already have unreferenced pEnum so
             * it will be a little bit faster but in the future it's
             * possible that I'll move it to separate function when
             * I'll add enumerators overloading. [druzus]
             */
            if( pMsg == s___msgEnumIndex.pDynSym )
            {
               hb_itemPutNL( hb_stackReturnItem(), pEnum->item.asEnum.offset );
               if( hb_pcount() > 0 && ISNUM( 1 ) )
                  pEnum->item.asEnum.offset = hb_itemGetNL( hb_param( 1, HB_IT_ANY ) );
               return &s___msgEnumIndex;
            }
            else if( pMsg == s___msgEnumBase.pDynSym )
            {
               hb_itemCopy( hb_stackReturnItem(), pEnum->item.asEnum.basePtr );
               if( hb_pcount() > 0 )
                  hb_itemCopy( pEnum->item.asEnum.basePtr,
                               hb_itemUnRef( hb_stackItemFromBase( 1 ) ) );
               return &s___msgEnumBase;
            }
            else if( pMsg == s___msgEnumValue.pDynSym )
            {
               pEnum = hb_itemUnRef( pEnum );
               hb_itemCopy( hb_stackReturnItem(), pEnum );
               if( hb_pcount() > 0 )
                  hb_itemCopy( pEnum, hb_itemUnRef( hb_stackItemFromBase( 1 ) ) );
               return &s___msgEnumValue;
            }
         }
      }
   }
   else if( HB_IS_SYMBOL( pObject ) )
   {
      if( pMsg == s___msgExec.pDynSym )
      {
         if( ! pObject->item.asSymbol.value->value.pFunPtr &&
               pObject->item.asSymbol.value->pDynSym )
            return pObject->item.asSymbol.value->pDynSym->pSymbol;
         else
            return pObject->item.asSymbol.value;
      }
      else if( pMsg == s___msgName.pDynSym )
      {
         hb_itemPutC( hb_stackReturnItem(),
                      pObject->item.asSymbol.value->szName );
         return &s___msgName;
      }
   }

   /* Default messages here */
   if( pMsg == s___msgWithObjectPush.pDynSym )
   {
      if( pStack )
      {
         PHB_ITEM pItem = hb_stackWithObjectItem();
         if( pItem )
         {
            /* push current WITH OBJECT object */
            hb_itemCopy( hb_stackReturnItem(), pItem );
            return &s___msgWithObjectPush;
         }
      }
   }
   else if( pMsg == s___msgWithObjectPop.pDynSym )
   {
      if( pStack )
      {
         PHB_ITEM pItem = hb_stackWithObjectItem();
         if( pItem )
         {
            /* replace current WITH OBJECT object */
            hb_itemCopy( pItem, hb_stackItemFromBase( 1 ) );
            hb_itemCopy( hb_stackReturnItem(), pItem );
            return &s___msgWithObjectPop;
         }
      }
   }

   else if( pMsg == s___msgClassName.pDynSym )
      return &s___msgClassName;

   else if( pMsg == s___msgClassH.pDynSym )
      return &s___msgClassH;

   else if( pMsg == s___msgClassSel.pDynSym )
      return &s___msgClassSel;

/*
   else if( pMsg == s___msgEval.pDynSym )
      return &s___msgEval;

   else if( pMsg == s___msgClsParent.pDynSym )
      return &s___msgClsParent;

   else if( pMsg == s___msgClass.pDynSym )
      return &s___msgClass;
*/
   if( pStack )
   {
      if( pClass && pClass->fHasOnError )
      {
         PMETHOD pMethod = hb_clsFindMsg( pClass, s___msgOnError.pDynSym );
         if( pMethod )
         {
            pStack->uiMethod = ( USHORT ) ( pMethod - pClass->pMethods );
            return pMethod->pFuncSym;
         }
      }

      /* remove this line if you want default HVM error message */
      return &s___msgNoMethod;
   }
   return NULL;
}

BOOL hb_objGetVarRef( PHB_ITEM pObject, PHB_SYMB pMessage,
                      PHB_STACK_STATE pStack )
{
   PHB_SYMB pExecSym;

   pExecSym = hb_objGetMethod( pObject, pMessage, pStack );

   if( pExecSym )
   {
      if( pExecSym->value.pFunPtr == hb___msgSetData )
      {
         USHORT uiObjClass = pObject->item.asArray.value->uiClass;
         PCLASS pClass     = &s_pClasses[ pStack->uiClass - 1 ];
         PMETHOD pMethod   = pClass->pMethods + pStack->uiMethod;
         ULONG ulIndex     = pMethod->uiData;

         if( pStack->uiClass != uiObjClass )
            ulIndex += hb_clsParentInstanceOffset( &s_pClasses[ uiObjClass - 1 ],
                                 s_pClasses[ pMethod->uiSprClass ].pClassSym );
         else
            ulIndex += pMethod->uiOffset;

         /* will arise only if the class has been modified after first instance */
         if( ulIndex > hb_arrayLen( pObject ) ) /* Resize needed */
            hb_arraySize( pObject, ulIndex );   /* Make large enough */

         return hb_arrayGetItemRef( pObject, ulIndex, hb_stackReturnItem() );
      }
      else if( pExecSym->value.pFunPtr == hb___msgSetClsData )
      {
         PCLASS pClass   = &s_pClasses[ pStack->uiClass ];
         PMETHOD pMethod = pClass->pMethods + pStack->uiMethod;

         return hb_arrayGetItemRef( pClass->pClassDatas, pMethod->uiData,
                                    hb_stackReturnItem() );
      }
      else if( pExecSym->value.pFunPtr == hb___msgSetShrData )
      {
         PCLASS pClass   = &s_pClasses[ pStack->uiClass - 1 ];
         PMETHOD pMethod = pClass->pMethods + pStack->uiMethod;

         return hb_arrayGetItemRef( s_pClasses[ pMethod->uiSprClass - 1 ].pSharedDatas,
                                    pMethod->uiData, hb_stackReturnItem() );
      }
      else if( pExecSym->value.pFunPtr == hb___msgScopeErr )
         hb___msgScopeErr();
   }

   return FALSE;
}

/*
 * Check if class has object destructors
 */
BOOL hb_clsHasDestructor( USHORT uiClass )
{
   if( uiClass && uiClass <= s_uiClasses )
      return s_pClasses[ uiClass - 1 ].fHasDestructor;
   else
      return FALSE;
}

/*
 * Call object destructor
 */
void hb_objDestructorCall( PHB_ITEM pObject )
{
   if( pObject->type == HB_IT_ARRAY &&
       pObject->item.asArray.value->uiClass != 0 &&
       pObject->item.asArray.value->uiClass <= s_uiClasses )
   {
      PCLASS pClass = &s_pClasses[ pObject->item.asArray.value->uiClass - 1 ];

      if( pClass->fHasDestructor )
      {
         USHORT uiAction;

         if( hb_vmRequestReenter( &uiAction ) )
         {
            hb_vmPushSymbol( &s___msgDestructor );
            hb_vmPush( pObject );
            hb_vmSend( 0 );
            hb_vmRequestRestore( uiAction );
         }
      }
   }
}

/*
 * Check if object has a given operator
 */
BOOL hb_objHasOperator( PHB_ITEM pObject, USHORT uiOperator )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_objHasOperator(%p,%hu)", pObject, uiOperator));

   if( pObject->type == HB_IT_ARRAY &&
       pObject->item.asArray.value->uiClass != 0 )
   {
      PCLASS pClass = &s_pClasses[ pObject->item.asArray.value->uiClass - 1 ];
      return ( pClass->ulOpFlags & ( 1UL << uiOperator ) ) != 0;
   }

   return FALSE;
}

/*
 * Call object operator. If pMsgArg is NULL then operator is unary.
 * Function return TRUE when object class overloads given operator
 * and FALSE otherwise. [druzus]
 */
BOOL hb_objOperatorCall( USHORT uiOperator, HB_ITEM_PTR pResult, PHB_ITEM pObject,
                         PHB_ITEM pMsgArg1, PHB_ITEM pMsgArg2 )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_objOperatorCall(%hu,%p,%p,%p,%p)", uiOperator, pResult, pObject, pMsgArg1, pMsgArg2));

   if( hb_objHasOperator( pObject, uiOperator ) )
   {
      hb_vmPushSymbol( s_opSymbols + uiOperator );
      hb_vmPush( pObject );
      hb_itemSetNil( hb_stackReturnItem() );
      if( pMsgArg1 )
      {
         hb_vmPush( pMsgArg1 );
         if( pMsgArg2 )
         {
            hb_vmPush( pMsgArg2 );
            hb_vmSend( 2 );
         }
         else
            hb_vmSend( 1 );
      }
      else
         hb_vmSend( 0 );

      /* store the return value */
      hb_itemMove( pResult, hb_stackReturnItem() );
      return TRUE;
   }
   return FALSE;
}

/*
 * return TRUE if object has a given message
 */
HB_EXPORT BOOL hb_objHasMessage( PHB_ITEM pObject, PHB_DYNS pMessage )
{
   return hb_objGetMethod( pObject, pMessage->pSymbol, NULL ) != NULL;
}

/*
 * <bool> = hb_objHasMsg( <pObject>, <szString> )
 *
 * Check whether <szString> is an existing message for object.
 *
 * <uPtr> should be read as a boolean
 */
HB_EXPORT BOOL hb_objHasMsg( PHB_ITEM pObject, char *szString )
{
   PHB_DYNS pDynSym;

   HB_TRACE(HB_TR_DEBUG, ("hb_objHasMsg(%p, %s)", pObject, szString));

   pDynSym = hb_dynsymFindName( szString );
   if( pDynSym )
   {
      return hb_objGetMethod( pObject, pDynSym->pSymbol, NULL ) != NULL;
   }
   else
   {
      return FALSE;
   }
}

HB_EXPORT void hb_objSendMessage( PHB_ITEM pObject, PHB_DYNS pMsgSym, ULONG ulArg, ... )
{
   if( pObject && pMsgSym )
   {
      hb_vmPushSymbol( pMsgSym->pSymbol );
      hb_vmPush( pObject );

      if( ulArg )
      {
         unsigned long i;
         va_list ap;

         va_start( ap, ulArg );
         for( i = 0; i < ulArg; i++ )
         {
            hb_vmPush( va_arg( ap, PHB_ITEM ) );
         }
         va_end( ap );
      }
      hb_vmSend( (USHORT) ulArg );
   }
   else
   {
      hb_errRT_BASE( EG_ARG, 3000, NULL, "__ObjSendMessage()", 0 );
   }
}

HB_EXPORT void hb_objSendMsg( PHB_ITEM pObject, char *sMsg, ULONG ulArg, ... )
{
   hb_vmPushSymbol( hb_dynsymGet( sMsg )->pSymbol );
   hb_vmPush( pObject );
   if( ulArg )
   {
      unsigned long i;
      va_list ap;

      va_start( ap, ulArg );
      for( i = 0; i < ulArg; i++ )
      {
         hb_vmPush( va_arg( ap, PHB_ITEM ) );
      }
      va_end( ap );
   }
   hb_vmSend( (USHORT) ulArg );
}

static PHB_SYMB hb_objFuncParam( int iParam )
{
   PHB_ITEM pItem = hb_param( iParam, HB_IT_SYMBOL | HB_IT_STRING );

   if( pItem )
   {
      if( HB_IS_SYMBOL( pItem ) )
         return pItem->item.asSymbol.value;
      else
      {
         PHB_DYNS pDynSym = hb_dynsymFindName( hb_itemGetCPtr( pItem ) );

         if( pDynSym && pDynSym->pSymbol->value.pFunPtr )
            return pDynSym->pSymbol;
      }
   }

   return NULL;
}

static PHB_DYNS hb_objMsgParam( int iParam )
{
   PHB_ITEM pMessage = hb_param( iParam, HB_IT_STRING | HB_IT_SYMBOL );
   PHB_DYNS pDynSym = NULL;

   if( pMessage )
   {
      char * szMsg = NULL;

      if( HB_IS_STRING( pMessage ) )
         szMsg = pMessage->item.asString.value;
      else
      {
         pDynSym = pMessage->item.asSymbol.value->pDynSym;
         if( !pDynSym )
            szMsg = pMessage->item.asSymbol.value->szName;
      }

      if( szMsg && *szMsg )
         pDynSym = hb_dynsymGet( szMsg );
   }

   return pDynSym;
}

static USHORT hb_clsUpdateScope( USHORT uiScope, BOOL fAssign )
{
   if( !fAssign )
      uiScope &= ~HB_OO_CLSTP_READONLY;

   else if( uiScope & HB_OO_CLSTP_READONLY &&
           !( uiScope & HB_OO_CLSTP_HIDDEN ) )
   {
      /* Class(y) does not allow to write to HIDDEN+READONLY
         instance variables, [druzus] */

      uiScope &= ~HB_OO_CLSTP_READONLY;
      uiScope |= uiScope & HB_OO_CLSTP_PROTECTED ?
                    HB_OO_CLSTP_HIDDEN : HB_OO_CLSTP_PROTECTED;
   }
   return uiScope;
}

/* ================================================ */

/*
 * __clsAddMsg( <hClass>, <cMessage>, <pFunction>, <nType>, [xInit], <uiScope>, <lPersistent> )
 *
 * Add a message to the class.
 *
 * <hClass>    Class handle
 * <cMessage>  Message
 * <pFunction> HB_OO_MSG_METHOD    : Pointer to function
 *             HB_OO_MSG_DATA      : \
 *             HB_OO_MSG_ASSIGN    :  > Index to instance area array
 *             HB_OO_MSG_ACCESS    : /
 *             HB_OO_MSG_CLASSDATA : \
 *             HB_OO_MSG_CLSASSIGN :  > Index class data array
 *             HB_OO_MSG_CLSACCESS : /
 *             HB_OO_MSG_INLINE    : Code block
 *             HB_OO_MSG_SUPER     : Handle of super class
 *             HB_OO_MSG_REALCLASS : Handle of real method class
 *
 * <nType>     see HB_OO_MSG_*
 *
 * <xInit>     HB_OO_MSG_DATA      : Optional initializer for DATA
 *             HB_OO_MSG_CLASSDATA : Optional initializer for DATA
 *             HB_OO_MSG_SUPER     : Superclass handle
 *
 * <uiScope>   HB_OO_CLSTP_EXPORTED        1 : default for data and method
 *             HB_OO_CLSTP_PROTECTED       2 : method or data protected
 *             HB_OO_CLSTP_HIDDEN          4 : method or data hidden
 *             HB_OO_CLSTP_CTOR            8 : method constructor
 *             HB_OO_CLSTP_READONLY       16 : data read only
 *             HB_OO_CLSTP_SHARED         32 : (method or) data shared
 *             HB_OO_CLSTP_CLASS          64 : message is the name of a superclass
 *             HB_OO_CLSTP_SUPER         128 : message is herited
 *             HB_OO_CLSTP_PERSIST       256 : message is persistent (PROPERTY)
 *             HB_OO_CLSTP_NONVIRTUAL    512 : Class method constructor
 *             HB_OO_CLSTP_OVERLOADED   1024 : Class method constructor
 *
 *             HB_OO_CLSTP_CLASSCTOR    2048 : Class method constructor
 *             HB_OO_CLSTP_CLASSMETH    4096 : Class method
 */

HB_FUNC( __CLSADDMSG )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );

   if( uiClass && uiClass <= s_uiClasses )
   {
      PCLASS   pClass   = &s_pClasses[ uiClass - 1 ];

      PHB_DYNS pMessage;
      PMETHOD  pNewMeth;
      USHORT   uiOperator, uiSprClass = 0, uiIndex = 0;
      PHB_SYMB pOpSym, pFuncSym = NULL;
      PHB_ITEM pBlock = NULL;
      BOOL     fOK;
      ULONG    ulOpFlags = 0;
      char *   szMessage = hb_parc( 2 );
      USHORT   nType     = ( USHORT ) hb_parni( 4 );
      USHORT   uiScope   = ( USHORT ) ( ISNUM( 6 ) ? hb_parni( 6 ) : HB_OO_CLSTP_EXPORTED );

      if( pClass->fLocked )
         return;

      if( hb_parl( 7 ) )
         uiScope |= HB_OO_CLSTP_PERSIST;

      /* translate names of operator overloading messages */
      if( nType == HB_OO_MSG_DESTRUCTOR )
         pMessage = s___msgDestructor.pDynSym;
      else if( nType == HB_OO_MSG_ONERROR )
         pMessage = s___msgOnError.pDynSym;
      else if (strcmp("+", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_PLUS )->pDynSym;
      else if (strcmp("-", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_MINUS )->pDynSym;
      else if (strcmp("*", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_MULT )->pDynSym;
      else if (strcmp("/", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_DIVIDE )->pDynSym;
      else if (strcmp("%", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_MOD )->pDynSym;
      else if (strcmp("^", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_POWER )->pDynSym;
      else if (strcmp("**", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_POWER )->pDynSym;
      else if (strcmp("++", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_INC )->pDynSym;
      else if (strcmp("--", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_DEC )->pDynSym;
      else if (strcmp("==", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_EXACTEQUAL )->pDynSym;
      else if (strcmp("=", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_EQUAL )->pDynSym;
      else if (strcmp("!=", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_NOTEQUAL )->pDynSym;
      else if (strcmp("<>", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_NOTEQUAL )->pDynSym;
      else if (strcmp("#", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_NOTEQUAL )->pDynSym;
      else if (strcmp("<", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_LESS )->pDynSym;
      else if (strcmp("<=", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_LESSEQUAL )->pDynSym;
      else if (strcmp(">", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_GREATER )->pDynSym;
      else if (strcmp(">=", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_GREATEREQUAL )->pDynSym;
      else if (strcmp(":=", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_ASSIGN )->pDynSym;
      else if (strcmp("$", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_INSTRING )->pDynSym;
      else if (strcmp("!", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_NOT )->pDynSym;
      else if (hb_stricmp(".NOT.", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_NOT )->pDynSym;
      else if (hb_stricmp(".AND.", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_AND )->pDynSym;
      else if (hb_stricmp(".OR.", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_OR )->pDynSym;
      else if( strcmp("[]", szMessage) == 0)
         pMessage = ( s_opSymbols + HB_OO_OP_ARRAYINDEX )->pDynSym;
      else
         pMessage = hb_dynsymGet( szMessage );

      for( uiOperator = 0, pOpSym = s_opSymbols;
           uiOperator <= HB_OO_MAX_OPERATOR; ++uiOperator, ++pOpSym )
      {
         if( pOpSym->pDynSym == pMessage )
         {
            ulOpFlags |= 1UL << uiOperator;
            break;
         }
      }

      /* basic parameter validation */
      switch( nType )
      {
         case HB_OO_MSG_METHOD:
         case HB_OO_MSG_ONERROR:
         case HB_OO_MSG_DESTRUCTOR:
            pFuncSym = hb_objFuncParam( 3 );
            fOK = pFuncSym != NULL;
            break;

         case HB_OO_MSG_INLINE:
            pBlock = hb_param( 3, HB_IT_BLOCK );
            fOK = pBlock != NULL;
            break;

         case HB_OO_MSG_SUPER:
            uiIndex = ( USHORT ) hb_parni( 3 );
            uiSprClass = ( USHORT ) hb_parni( 5 );
            fOK = uiSprClass && uiSprClass <= s_uiClasses &&
                  uiIndex <= pClass->uiDatas;
            break;

         case HB_OO_MSG_DATA:
            nType = pMessage->pSymbol->szName[ 0 ] == '_' ?
                    HB_OO_MSG_ASSIGN : HB_OO_MSG_ACCESS;
         case HB_OO_MSG_ASSIGN:
         case HB_OO_MSG_ACCESS:
            uiIndex = ( USHORT ) hb_parni( 3 );
            /* This validation can break buggy .prg code which wrongly
             * sets data offsets but IMHO it will help to clean the code.
             * [druzus]
             */
            fOK = uiIndex && uiIndex <= pClass->uiDatas - pClass->uiDataFirst;
            break;

         case HB_OO_MSG_CLASSDATA:
            nType = pMessage->pSymbol->szName[ 0 ] == '_' ?
                    HB_OO_MSG_CLSASSIGN : HB_OO_MSG_CLSACCESS;
         case HB_OO_MSG_CLSASSIGN:
         case HB_OO_MSG_CLSACCESS:
            uiIndex = ( USHORT ) hb_parni( 3 );
            fOK = uiIndex != 0;
            break;

         case HB_OO_MSG_REALCLASS:
         case HB_OO_MSG_VIRTUAL:
            fOK = TRUE;
            break;

         default:
            fOK = FALSE;
      }

      if( !fOK )
      {
         hb_errRT_BASE( EG_ARG, 3000, NULL, "__CLSADDMSG", HB_ERR_ARGS_BASEPARAMS );
         return;
      }

      pNewMeth = hb_clsAllocMsg( pClass, pMessage );
      if( ! pNewMeth )
         return;

#ifndef HB_VIRTUAL_HIDDEN
      if( uiScope & HB_OO_CLSTP_HIDDEN )
         uiScope |= HB_OO_CLSTP_NONVIRTUAL;
#endif

      if( ! pNewMeth->pMessage )
         pClass->uiMethods++;           /* One more message */
      else
      {
         BOOL fOverLoad = ( pNewMeth->uiScope & HB_OO_CLSTP_OVERLOADED ) ||
                          ( ( pNewMeth->uiScope & HB_OO_CLSTP_NONVIRTUAL ) &&
                            pNewMeth->uiSprClass != uiClass );

         if( ! hb_clsClearMsg( pClass, pNewMeth ) )
            return;

         memset( pNewMeth, 0, sizeof( METHOD ) );
         if( fOverLoad )
            uiScope |= HB_OO_CLSTP_OVERLOADED;
      }
      pNewMeth->pMessage = pMessage;
      pNewMeth->uiSprClass = uiClass  ; /* now used !! */

      switch( nType )
      {
         case HB_OO_MSG_METHOD:

            pNewMeth->pFuncSym = pFuncSym;
            pNewMeth->uiScope = uiScope;
            break;

         case HB_OO_MSG_ASSIGN:

            pNewMeth->uiScope = hb_clsUpdateScope( uiScope, TRUE );
            /* Class(y) does not allow to write to HIDDEN+READONLY
               instance variables, [druzus] */
            if( pNewMeth->uiScope & HB_OO_CLSTP_READONLY &&
                pNewMeth->uiScope & HB_OO_CLSTP_HIDDEN )
               pNewMeth->pFuncSym = &s___msgScopeErr;
            else
            {
               pNewMeth->pFuncSym = &s___msgSetData;
               pNewMeth->uiData = uiIndex;
               pNewMeth->uiOffset = pClass->uiDataFirst;
            }
            break;

         case HB_OO_MSG_ACCESS:

            pNewMeth->uiScope = hb_clsUpdateScope( uiScope, FALSE );
            pNewMeth->uiData = uiIndex;
            pNewMeth->uiOffset = pClass->uiDataFirst;
            hb_clsAddInitValue( pClass, hb_param( 5, HB_IT_ANY ), HB_OO_MSG_DATA,
                                pNewMeth->uiData, pNewMeth->uiOffset, uiClass );
            pNewMeth->pFuncSym = &s___msgGetData;
            break;

         case HB_OO_MSG_CLSASSIGN:

            pNewMeth->uiScope = hb_clsUpdateScope( uiScope, TRUE );
            pNewMeth->uiData = uiIndex;
            /* Class(y) does not allow to write to HIDDEN+READONLY
               instance variables, [druzus] */
            if( pNewMeth->uiScope & HB_OO_CLSTP_READONLY &&
                pNewMeth->uiScope & HB_OO_CLSTP_HIDDEN )
               pNewMeth->pFuncSym = &s___msgScopeErr;
            else if( pNewMeth->uiScope & HB_OO_CLSTP_SHARED )
            {
               if( hb_arrayLen( pClass->pSharedDatas ) < ( ULONG ) pNewMeth->uiData )
                  hb_arraySize( pClass->pSharedDatas, pNewMeth->uiData );
               pNewMeth->pFuncSym = &s___msgSetShrData;
            }
            else
            {
               if( hb_arrayLen( pClass->pClassDatas ) < ( ULONG ) pNewMeth->uiData )
                  hb_arraySize( pClass->pClassDatas, pNewMeth->uiData );
               pNewMeth->pFuncSym = &s___msgSetClsData;
            }
            break;

         case HB_OO_MSG_CLSACCESS:

            pNewMeth->uiScope = hb_clsUpdateScope( uiScope, FALSE );
            pNewMeth->uiData = uiIndex;
            if( pNewMeth->uiScope & HB_OO_CLSTP_SHARED )
            {
               PHB_ITEM pInit = hb_param( 5, HB_IT_ANY );

               if( hb_arrayLen( pClass->pSharedDatas ) < ( ULONG ) pNewMeth->uiData )
                  hb_arraySize( pClass->pSharedDatas, pNewMeth->uiData );

               if( pInit && ! HB_IS_NIL( pInit ) ) /* Initializer found */
               {
                  /* Shared Classdata need to be initialized only once
                   * ACCESS/ASSIGN methods will be inherited by subclasses
                   * and will operate on this value so it's not necessary
                   * to keep the init value. [druzus]
                   */
                  pInit = hb_itemClone( pInit );
                  hb_arraySet( pClass->pSharedDatas, pNewMeth->uiData, pInit );
                  hb_itemRelease( pInit );
               }
               pNewMeth->pFuncSym = &s___msgGetShrData;
            }
            else
            {
               if( hb_arrayLen( pClass->pClassDatas ) < ( ULONG ) pNewMeth->uiData )
                  hb_arraySize( pClass->pClassDatas, pNewMeth->uiData );
               pNewMeth->uiOffset = hb_clsAddInitValue( pClass,
                                 hb_param( 5, HB_IT_ANY ), HB_OO_MSG_CLASSDATA,
                                 pNewMeth->uiData, 0, uiClass );
               pNewMeth->pFuncSym = &s___msgGetClsData;
            }
            break;

         case HB_OO_MSG_INLINE:

            pNewMeth->pFuncSym = &s___msgEvalInline;
            pNewMeth->uiScope = uiScope;
            pNewMeth->uiData = ( USHORT ) ( hb_arrayLen( pClass->pInlines ) + 1 );
            hb_arraySize( pClass->pInlines, pNewMeth->uiData );
            hb_arraySet( pClass->pInlines, pNewMeth->uiData, pBlock );
            break;

         case HB_OO_MSG_VIRTUAL:

            pNewMeth->pFuncSym = &s___msgVirtual;
            pNewMeth->uiScope = uiScope;
            break;

         case HB_OO_MSG_SUPER:

            pNewMeth->uiSprClass = uiSprClass; /* store the super handel */
            pNewMeth->uiOffset = uiIndex; /* offset to instance area */
            pNewMeth->uiScope = uiScope;
            pNewMeth->pFuncSym = &s___msgSuper;
            break;

         case HB_OO_MSG_REALCLASS:
            pNewMeth->pFuncSym = &s___msgRealClass;
            pNewMeth->uiScope = uiScope;
            break;

         case HB_OO_MSG_ONERROR:

            pNewMeth->pFuncSym = pFuncSym;
            pClass->fHasOnError = TRUE;
            break;

         case HB_OO_MSG_DESTRUCTOR:

            pNewMeth->pFuncSym = pFuncSym;
            pClass->fHasDestructor = TRUE;
            break;

         default:

            hb_errInternal( HB_EI_CLSINVMETHOD, NULL, "__clsAddMsg", NULL );
            return;
      }

      pClass->ulOpFlags |= ulOpFlags;
   }
}


/*
 * <hClass> := __clsNew( <cClassName>, <nDatas>, [<ahSuper>], [<pClassFunc>], [<lModuleFriendly>] )
 *
 * Create a new class
 *
 * <cClassName> Name of the class
 * <nDatas>     Number of DATAs in the class
 * <ahSuper>    Optional array with handle(s) of superclass(es)
 * <pClassFunc> Class function symbol
 * <lModuleFriendly> when true all functions and classes from the same
 *                   module as pClassFunc are defined as friends
 */
HB_FUNC( __CLSNEW )
{
   PCLASS pNewCls;
   PMETHOD pMethod;
   PHB_ITEM pahSuper;
   USHORT ui, uiSuper, uiSuperCls;
   BOOL fModuleFriendly;
   PHB_SYMB pClassFunc;
   USHORT * puiClassData = NULL, uiClassDataSize = 0;

   pahSuper = hb_param( 3, HB_IT_ARRAY );
   uiSuper  = ( USHORT ) ( pahSuper ? hb_arrayLen( pahSuper ) : 0 );

   pClassFunc = hb_vmGetRealFuncSym( hb_itemGetSymbol( hb_param( 4, HB_IT_SYMBOL ) ) );
   fModuleFriendly = hb_parl( 5 );

   if( s_pClasses )
      s_pClasses = ( PCLASS ) hb_xrealloc( s_pClasses, sizeof( CLASS ) * ( s_uiClasses + 1 ) );
   else
      s_pClasses = ( PCLASS ) hb_xgrab( sizeof( CLASS ) );

   pNewCls = s_pClasses + s_uiClasses++;
   memset( pNewCls, 0, sizeof( CLASS ) );
   pNewCls->szName = hb_strdup( hb_parc( 1 ) );
   pNewCls->pClassSym = hb_dynsymGet( pNewCls->szName );
   if( !pClassFunc )
      pClassFunc = hb_vmGetRealFuncSym( pNewCls->pClassSym->pSymbol );
   pNewCls->pClassFuncSym = pClassFunc;
   if( fModuleFriendly )
      hb_vmFindModuleSymbols( pClassFunc, &pNewCls->pFriendModule,
                                          &pNewCls->uiFriendModule );

   for( ui = 1; ui <= uiSuper; ++ui )
   {
      uiSuperCls = ( USHORT ) hb_arrayGetNI( pahSuper, ui );
      if( uiSuperCls && uiSuperCls < s_uiClasses )
      {
         PCLASS pSprCls;

         pSprCls = &s_pClasses[ uiSuperCls - 1 ];
         if( !pNewCls->pMethods ) /* This is the first superclass */
         {
            hb_clsCopyClass( pNewCls, pSprCls );
         }
         else if( !hb_clsHasParent( pNewCls, pSprCls->pClassSym ) )
         {
            ULONG  ul, ulLimit;
            USHORT nLenClsDatas;

            /* create class data translation tables */
            nLenClsDatas  = ( USHORT ) hb_itemSize( pSprCls->pClassDatas );
            if( nLenClsDatas )
            {
               if( nLenClsDatas > uiClassDataSize )
               {
                  if( puiClassData )
                     puiClassData = ( USHORT * ) hb_xrealloc( puiClassData,
                                             sizeof( USHORT ) * nLenClsDatas );
                  else
                     puiClassData = ( USHORT * ) hb_xgrab( sizeof( USHORT ) *
                                                           nLenClsDatas );
                  uiClassDataSize = nLenClsDatas;
               }
               memset( puiClassData, 0, sizeof( USHORT ) * nLenClsDatas );
            }

            /* Copy super classs handles */
            ulLimit = hb_clsMthNum( pSprCls );
            for( ul = 0; ul < ulLimit; ++ul )
            {
               if( pSprCls->pMethods[ ul ].pMessage &&
                   pSprCls->pMethods[ ul ].pFuncSym == &s___msgSuper )
               {
                  PCLASS pCls = &s_pClasses[
                                    pSprCls->pMethods[ ul ].uiSprClass - 1 ];

                  pMethod = hb_clsAllocMsg( pNewCls,
                                            pSprCls->pMethods[ ul ].pMessage );
                  if( ! pMethod )
                     return;
                  if( pMethod->pMessage == NULL )
                  {
                     pNewCls->uiMethods++;
                     memcpy( pMethod, pSprCls->pMethods + ul, sizeof( METHOD ) );
                     pMethod->uiOffset = pNewCls->uiDatas;
                     pNewCls->uiDatas += pCls->uiDatas - pCls->uiDataFirst;
                  }
               }
            }

            /* add class casting if not exist */
            pMethod = hb_clsAllocMsg( pNewCls, pSprCls->pClassSym );
            if( ! pMethod )
               return;
            if( pMethod->pMessage == NULL )
            {
               pNewCls->uiMethods++;
               pMethod->pMessage = pSprCls->pClassSym;
               pMethod->uiSprClass = uiSuperCls;
               pMethod->uiScope = HB_OO_CLSTP_EXPORTED;
               pMethod->pFuncSym = &s___msgSuper;
               pMethod->uiOffset = pNewCls->uiDatas;
               pNewCls->uiDatas += pSprCls->uiDatas - pSprCls->uiDataFirst;
            }

            /* Copy instance area init data */
            if( pSprCls->uiInitDatas )
            {
               USHORT ui;
               for( ui = 0; ui < pSprCls->uiInitDatas; ++ui )
               {
                  if( pSprCls->pInitData[ ui ].uiType == HB_OO_MSG_DATA )
                  {
                     USHORT uiCls = pSprCls->pInitData[ ui ].uiSprClass;
                     hb_clsAddInitValue( pNewCls,
                        pSprCls->pInitData[ ui ].pInitValue, HB_OO_MSG_DATA,
                        pSprCls->pInitData[ ui ].uiData,
                        hb_clsParentInstanceOffset( pNewCls,
                                          s_pClasses[ uiCls - 1 ].pClassSym ),
                        uiCls );
                  }
               }
            }

            /* Now working on other methods */
            ulLimit = hb_clsMthNum( pSprCls );
            for( ul = 0; ul < ulLimit; ++ul )
            {
               if( pSprCls->pMethods[ ul ].pMessage )
               {
                  pMethod = hb_clsAllocMsg( pNewCls, pSprCls->pMethods[ ul ].pMessage );
                  if( ! pMethod )
                     return;

                  /* Ok, this bucket is empty */
                  if( pMethod->pMessage == NULL )
                  {
                     /* Now, we can increment the msg count */
                     pNewCls->uiMethods++;
                     memcpy( pMethod, pSprCls->pMethods + ul, sizeof( METHOD ) );

                     if( pMethod->pFuncSym == &s___msgEvalInline )
                     {
                        hb_arrayAdd( pNewCls->pInlines,
                           hb_arrayGetItemPtr( pSprCls->pInlines, pMethod->uiData ) );
                        pMethod->uiData = ( USHORT ) hb_arrayLen( pNewCls->pInlines );
                     }
                     else if( pMethod->pFuncSym == &s___msgSetClsData ||
                              pMethod->pFuncSym == &s___msgGetClsData )
                     {
                        if( pMethod->uiData > nLenClsDatas )
                           hb_errInternal( HB_EI_CLSINVMETHOD, NULL, "__clsNew", NULL );

                        if( puiClassData[ pMethod->uiData - 1 ] == 0 )
                        {
                           puiClassData[ pMethod->uiData - 1 ] = ( USHORT )
                                       hb_arrayLen( pNewCls->pClassDatas ) + 1;
                           hb_arraySize( pNewCls->pClassDatas,
                                         puiClassData[ pMethod->uiData - 1 ] );
                        }
                        if( pMethod->uiOffset )
                        {
                           pMethod->uiOffset = hb_clsAddInitValue( pNewCls,
                              pSprCls->pInitData[ pMethod->uiOffset - 1 ].pInitValue,
                              HB_OO_MSG_CLASSDATA, puiClassData[ pMethod->uiData - 1 ],
                              0, uiSuperCls );
                        }
                        pMethod->uiData = puiClassData[ pMethod->uiData - 1 ];
                     }
                     else if( pMethod->pFuncSym == &s___msgSetData ||
                              pMethod->pFuncSym == &s___msgGetData )
                     {
                        USHORT uiIndex = pMethod->uiData + pMethod->uiOffset;
                        if( uiIndex > pSprCls->uiDatas )
                           hb_errInternal( HB_EI_CLSINVMETHOD, NULL, "__clsNew", NULL );
                        pMethod->uiOffset = hb_clsParentInstanceOffset( pNewCls,
                              s_pClasses[ pMethod->uiSprClass - 1 ].pClassSym );
                     }
                     pMethod->uiScope |= HB_OO_CLSTP_SUPER;
                  }
                  else if( pSprCls->pMethods[ ul ].uiScope &
                           ( HB_OO_CLSTP_OVERLOADED | HB_OO_CLSTP_NONVIRTUAL ) )
                     pMethod->uiScope |= HB_OO_CLSTP_OVERLOADED;
               }
            }
            pNewCls->ulOpFlags |= pSprCls->ulOpFlags;
         }
      }
   }
   if( puiClassData )
      hb_xfree( puiClassData );

   if( !pNewCls->pMethods )
   {
      hb_clsDictInit( pNewCls, HASH_KEY );
      pNewCls->pClassDatas  = hb_itemArrayNew( 0 );
      pNewCls->pSharedDatas = hb_itemArrayNew( 0 );
      pNewCls->pInlines     = hb_itemArrayNew( 0 );
   }

   /* add self class casting */
   pMethod = hb_clsAllocMsg( pNewCls, pNewCls->pClassSym );
   if( ! pMethod )
      return;
   if( pMethod->pMessage == NULL )
   {
      pNewCls->uiMethods++;
      pMethod->pMessage = pNewCls->pClassSym;
      pMethod->uiSprClass = s_uiClasses;
      pMethod->uiScope = HB_OO_CLSTP_EXPORTED;
      pMethod->pFuncSym = &s___msgSuper;
      pMethod->uiOffset = pNewCls->uiDatas;
   }

   pNewCls->uiDataFirst = pNewCls->uiDatas;
   pNewCls->uiDatas += ( USHORT ) hb_parni( 2 );

   hb_retni( s_uiClasses );
}

/*
 *  __clsAddFriend( <hClass>, <pFyncSym> )
 *
 *  Add friend function
 */
HB_FUNC( __CLSADDFRIEND )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );

   if( uiClass && uiClass <= s_uiClasses )
   {
      PCLASS pClass = &s_pClasses[ uiClass - 1 ];

      if( !pClass->fLocked )
      {
         PHB_SYMB pSym = hb_vmGetRealFuncSym( hb_itemGetSymbol( hb_param( 2,
                                                         HB_IT_SYMBOL ) ) );
         if( pSym )
            hb_clsAddFriendSymbol( pClass, pSym );
      }
   }
}

/*
 * __clsDelMsg( <oObj>, <cMessage> )
 *
 * Delete message (only for INLINE and METHOD)
 *
 * <oObj>     Object
 * <cMessage> Message
 */
HB_FUNC( __CLSDELMSG )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );
   PHB_ITEM pString = hb_param( 2, HB_IT_STRING );

   if( uiClass && uiClass <= s_uiClasses && pString &&
       ! s_pClasses[ uiClass - 1 ].fLocked )
   {
      PHB_DYNS pMsg = hb_dynsymFindName( pString->item.asString.value );

      if( pMsg )
         hb_clsFreeMsg( &s_pClasses[ uiClass - 1 ], pMsg );
   }
}


/*
 * [<o(Super)Object>] := hb_clsInst( <hClass> )
 *
 * Create a (super)object from class definition <hClass>
 */
static PHB_ITEM hb_clsInst( USHORT uiClass )
{
   PHB_ITEM pSelf = NULL;

   if( uiClass && uiClass <= s_uiClasses )
   {
      PCLASS   pClass = &s_pClasses[ uiClass - 1 ];

      pSelf = hb_itemNew( NULL );
      hb_arrayNew( pSelf, pClass->uiDatas );
      pSelf->item.asArray.value->uiClass = uiClass;

      /* Initialise value if initialisation was requested */
      if( pClass->uiInitDatas )
      {
         PINITDATA pInitData = pClass->pInitData;
         USHORT ui = pClass->uiInitDatas;
         PHB_ITEM pDestItm;

         do
         {
            if( pInitData->uiType == HB_OO_MSG_DATA )
               pDestItm = hb_arrayGetItemPtr( pSelf,
                                    pInitData->uiData + pInitData->uiOffset );
            else if( pInitData->uiType == HB_OO_MSG_CLASSDATA )
            {
               pDestItm = hb_arrayGetItemPtr( pClass->pClassDatas,
                                              pInitData->uiData );
               /* do not initialize it again */
               pInitData->uiType = HB_OO_MSG_INITIALIZED;
            }
            else
               pDestItm = NULL;

            if( pDestItm )
            {
               PHB_ITEM pInit = hb_itemClone( pInitData->pInitValue );
               hb_itemMove( pDestItm, pInit );
               hb_itemRelease( pInit );
            }
            ++pInitData;
         }
         while( --ui );
      }
   }

   return pSelf;
}

/*
 * <oNewObject> := __clsInst( <hClass> )
 *
 * Create a new object from class definition <hClass>
 */
HB_FUNC( __CLSINST )
{
   PHB_ITEM pSelf = hb_clsInst( ( USHORT ) hb_parni( 1 ) );

   if( pSelf )
      hb_itemRelease( hb_itemReturn( pSelf ) );
}

/*
 * __clsLock( <hClass> )
 * Block farther class modifications
 */
HB_FUNC( __CLSLOCK )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );

   if( uiClass && uiClass <= s_uiClasses )
      s_pClasses[ uiClass - 1 ].fLocked = TRUE;
}

/*
 * __clsModMsg( <oObj>, <cMessage>, <pFunc> )
 *
 * Modify message (only for INLINE and METHOD)
 */
HB_FUNC( __CLSMODMSG )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );
   PHB_ITEM pString = hb_param( 2, HB_IT_STRING );

   if( uiClass && uiClass <= s_uiClasses && pString &&
       ! s_pClasses[ uiClass - 1 ].fLocked )
   {
      PHB_DYNS pMsg = hb_dynsymFindName( pString->item.asString.value );

      if( pMsg )
      {
         PCLASS  pClass  = &s_pClasses[ uiClass - 1 ];
         PMETHOD pMethod = hb_clsFindMsg( pClass, pMsg );

         if( pMethod )
         { 
            PHB_SYMB pFuncSym = pMethod->pFuncSym;

            if( pFuncSym == &s___msgSetData || pFuncSym == &s___msgGetData )
            {
               hb_errRT_BASE( EG_ARG, 3004, "Cannot modify a DATA item", "__CLSMODMSG", 0 );
            }
            else if( pFuncSym == &s___msgEvalInline )
            {
               PHB_ITEM pBlock = hb_param( 3, HB_IT_BLOCK );

               if( pBlock == NULL )
                  hb_errRT_BASE( EG_ARG, 3000, "Cannot modify INLINE method", "__CLSMODMSG", 0 );
               else
                  hb_arraySet( pClass->pInlines, pMethod->uiData, pBlock );
            }
            else                                      /* Modify METHOD */
            {
               PHB_SYMB pFuncSym = hb_objFuncParam( 3 );

               if( pFuncSym == NULL )
                  hb_errRT_BASE( EG_ARG, 3000, NULL, "__CLSADDMSG", 0 );
               else
                  pMethod->pFuncSym = pFuncSym;
            }
         }
      }
   }
}


/*
 * <cClassName> := ClassName( <hClass> )
 *
 * Returns class name of <hClass>
 */
HB_FUNC( __OBJGETCLSNAME )
{
   PHB_ITEM pObject = hb_param( 1, HB_IT_OBJECT );
   USHORT uiClass;

   if( pObject )
      uiClass = pObject->item.asArray.value->uiClass;
   else
      uiClass = ( USHORT ) hb_parni( 1 );

   hb_retc( hb_clsName( uiClass ) );
}


/*
 * <lRet> := __objHasMsg( <oObj>, <cSymbol> )
 *
 * Is <cSymbol> a valid message for the <oObj>
 */
HB_FUNC( __OBJHASMSG )
{
   PHB_DYNS pMessage = hb_objMsgParam( 2 );

   if( pMessage )
      hb_retl( hb_objHasMessage( hb_param( 1, HB_IT_ANY ), pMessage ) );
   else
      hb_errRT_BASE_SubstR( EG_ARG, 1099, NULL, "__OBJHASMSG", HB_ERR_ARGS_BASEPARAMS );
}

/*
 * <xRet> = __objSendMsg( <oObj>, <cSymbol>, <xArg,..>
 *
 * Send a message to an object
 */
HB_FUNC( __OBJSENDMSG )
{
   PHB_DYNS pMessage = hb_objMsgParam( 2 );

   if( pMessage )
   {
      USHORT uiPCount = hb_pcount();
      USHORT uiParam;

      hb_vmPushSymbol( pMessage->pSymbol );     /* Push message symbol */
      hb_vmPush( hb_param( 1, HB_IT_ANY ) );    /* Push object */

      for( uiParam = 3; uiParam <= uiPCount; ++uiParam )    /* Push arguments on stack */
      {
         hb_vmPush( hb_stackItemFromBase( uiParam ) );
      }
      hb_vmSend( ( USHORT ) ( uiPCount - 2 ) );             /* Execute message */
   }
   else
   {
      hb_errRT_BASE( EG_ARG, 3000, NULL, "__OBJSENDMSG", HB_ERR_ARGS_BASEPARAMS );
   }
}

/*
 * <oNew> := __objClone( <oOld> )
 *
 * Clone an object. Note the similarity with aClone ;-)
 */
HB_FUNC( __OBJCLONE )
{
   PHB_ITEM pSrcObject = hb_param( 1, HB_IT_OBJECT );
   PHB_ITEM pDstObject;

   if( pSrcObject )
   {
      pDstObject = hb_arrayClone( pSrcObject );
      hb_itemRelease( hb_itemReturn( pDstObject ) );
   }
   else
   {
      hb_errRT_BASE( EG_ARG, 3001, NULL, "__OBJCLONE", 0 );
   }
}

/*
 * <hClass> := __clsInstSuper( <cName> )
 *
 * Instance super class and return class handle
 */
HB_FUNC( __CLSINSTSUPER )
{
   char * szString = hb_parc( 1 );
   USHORT uiClassH = 0, uiClass;

   if( szString && *szString )
   {
      PHB_DYNS pDynSym = hb_dynsymFindName( szString );

      if( pDynSym )
      {
         for( uiClass = 0; uiClass < s_uiClasses; uiClass++ )
         {
            if( s_pClasses[ uiClass ].pClassSym == pDynSym )
            {
               uiClassH = uiClass + 1;
               break;
            }
         }

         if( uiClassH == 0 )
         {
            hb_vmPushSymbol( pDynSym->pSymbol );         /* Push function name       */
            hb_vmPushNil();
            hb_vmFunction( 0 );                          /* Execute super class      */

            if( hb_vmRequestQuery() == 0 )
            {
               PHB_ITEM pObject = hb_stackReturnItem();

               if( HB_IS_OBJECT( pObject ) )
               {
                  uiClass = pObject->item.asArray.value->uiClass;

                  if( s_pClasses[ uiClass - 1 ].pClassSym == pDynSym )
                     uiClassH = uiClass;
                  else
                  {
                     for( uiClass = 0; uiClass < s_uiClasses; uiClass++ )
                     { 
                        if( s_pClasses[ uiClass ].pClassSym == pDynSym )
                        {
                           uiClassH = uiClass + 1;
                           break;
                        }
                     }
                     /* still not found, try to send NEW() message */
                     if( uiClassH == 0 )
                     {
                        hb_vmPushSymbol( &s___msgNew );
                        hb_vmPush( pObject );
                        hb_vmSend( 0 );

                        pObject = hb_stackReturnItem();
                        if( HB_IS_OBJECT( pObject ) )
                        {
                           uiClass = pObject->item.asArray.value->uiClass;
                           if( s_pClasses[ uiClass - 1 ].pClassSym == pDynSym )
                              uiClassH = uiClass;
                        }
                     }
                  }

                  /* This disables destructor execution for this object */
                  if( uiClassH && HB_IS_OBJECT( pObject ) )
                     pObject->item.asArray.value->uiClass = 0;
               }
               else
               {
                  hb_errRT_BASE( EG_ARG, 3002, "Super class does not return an object", "__CLSINSTSUPER", 0 );
               }
            }
         }
      }
      else
      {
         hb_errRT_BASE( EG_ARG, 3003, "Cannot find super class", "__CLSINSTSUPER", 0 );
      }
   }

   hb_retni( uiClassH );
}

/*
 * <nSeq> = __ClsCntClasses()
 *
 * Return number of classes
 */
HB_FUNC( __CLSCNTCLASSES )
{
   hb_retni( ( int ) s_uiClasses );
}

/*
 * <nSeq> = __cls_CntClsData( <hClass> )
 *
 * Return number of class datas
 */
HB_FUNC( __CLS_CNTCLSDATA )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );

   hb_retni( uiClass && uiClass <= s_uiClasses ?
                  hb_arrayLen( s_pClasses[ uiClass - 1 ].pClassDatas ) : 0 );
}

/*
 * <nSeq> = __cls_CntShrData( <hClass> )
 *
 * Return number of class datas
 */
HB_FUNC( __CLS_CNTSHRDATA )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );

   hb_retni( uiClass && uiClass <= s_uiClasses ?
                  hb_arrayLen( s_pClasses[ uiClass - 1 ].pSharedDatas ) : 0 );
}

/*
 * <nSeq> = __cls_CntData( <hClass> )
 *
 * Return number of datas
 */
HB_FUNC( __CLS_CNTDATA )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );

   hb_retni( uiClass && uiClass <= s_uiClasses ?
             s_pClasses[ uiClass - 1 ].uiDatas : 0 );
}

/*
 * <nSeq> = __cls_DecData( <hClass> )
 *
 * Decrease number of datas and return new value
 */
HB_FUNC( __CLS_DECDATA )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );

   if( uiClass && uiClass <= s_uiClasses && s_pClasses[ uiClass - 1 ].uiDatas )
   {
      if( s_pClasses[ uiClass - 1 ].fLocked )
         hb_retni( s_pClasses[ uiClass - 1 ].uiDatas );
      else
         hb_retni( --s_pClasses[ uiClass - 1 ].uiDatas );
   }
   else
      hb_retni( 0 );
}

/*
 * <nSeq> = __cls_IncData( <hClass> )
 * Increase number of datas and return new value
 */
HB_FUNC( __CLS_INCDATA )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );

   if( uiClass && uiClass <= s_uiClasses )
   {
      if( s_pClasses[ uiClass - 1 ].fLocked )
         hb_retni( s_pClasses[ uiClass - 1 ].uiDatas );
      else
         hb_retni( ++s_pClasses[ uiClass - 1 ].uiDatas );
   }
   else
      hb_retni( 0 );
}

/* NOTE: Undocumented Clipper function */

/* see for parameter compatibility with Clipper. */
HB_FUNC( __CLASSNEW )
{
   HB_FUNC_EXEC( __CLSNEW );
}


/* NOTE: Undocumented Clipper function */

HB_FUNC( __CLASSINSTANCE )
{
   HB_FUNC_EXEC( __CLSINST );
}


/* NOTE: Undocumented Clipper function */

HB_FUNC( __CLASSADD )
{
   HB_FUNC_EXEC( __CLSADDMSG );
}


/* NOTE: Undocumented Clipper function */

HB_FUNC( __CLASSNAME )
{
   hb_retc( hb_clsName( ( USHORT ) hb_parni( 1 ) ) );
}

/* NOTE: Undocumented Clipper function */
/* NOTE: Based on hb___msgClsSel() */

HB_FUNC( __CLASSSEL )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );
   PHB_ITEM pReturn = hb_itemNew( NULL );

   if( uiClass && uiClass <= s_uiClasses )
   {
      PCLASS  pClass  = &s_pClasses[ uiClass - 1 ];
      PMETHOD pMethod = pClass->pMethods;
      ULONG   ulLimit = hb_clsMthNum( pClass ), ulPos = 0;

      hb_arrayNew( pReturn, pClass->uiMethods ); /* Create a transfer array */

      do
      {
         if( pMethod->pMessage )    /* Hash Entry used ? */
         {
            PHB_ITEM pItem = hb_arrayGetItemPtr( pReturn, ++ulPos );
            if( pItem )
               hb_itemPutC( pItem, pMethod->pMessage->pSymbol->szName );
            else
               break;  /* Generate internal error? */
         }
         ++pMethod;
      }
      while( --ulLimit );

      if( ulPos < ( ULONG ) pClass->uiMethods )
         hb_arraySize( pReturn, ulPos );
   }

   hb_itemRelease( hb_itemReturn( pReturn ) );
}

/* to be used from Classes ERROR HANDLER method */
HB_FUNC( __GETMESSAGE )
{
   hb_retc( hb_stackItem( hb_stackBaseItem()->item.asSymbol.stackstate->lBaseItem )->item.asSymbol.value->szName );
}

HB_FUNC( __CLSPARENT )
{
   hb_retl( hb_clsIsParent( hb_parni( 1 ) , hb_parc( 2 ) ) );
}

HB_FUNC( __SENDER )
{
   LONG lOffset = hb_stackBaseProcOffset( 2 );

   if( lOffset >= 0 )
   {
      PHB_ITEM pSelf = hb_stackItem( lOffset + 1 );

      /* Is it inline method? */
      if( lOffset > 0 && HB_IS_BLOCK( pSelf ) &&
          hb_stackItem( lOffset )->item.asSymbol.value == &hb_symEval )
      {
         pSelf = hb_stackItem( hb_stackItem( lOffset )->
                               item.asSymbol.stackstate->lBaseItem + 1 );
      }

      if( HB_IS_OBJECT( pSelf ) )
      {
         hb_itemReturn( pSelf );
      }
   }
}

/*
 * Added by R�C&JfL
 *
 * based on hb___msgClsH( void )
 */
HB_FUNC( __CLASSH )
{
   PHB_ITEM pObject = hb_param( 1, HB_IT_OBJECT );

   hb_retni( pObject ? pObject->item.asArray.value->uiClass : 0 );
}

/*
 * based on hb___msgEval( void )
 */
HB_FUNC( __EVAL )
{
   PHB_ITEM pObject = hb_param( 1, HB_IT_ANY );
   USHORT uiPCount = hb_pcount();

   if( pObject && HB_IS_BLOCK( pObject ) )
   {
      USHORT uiParam;

      hb_vmPushSymbol( &hb_symEval );
      hb_vmPush( pObject );                /* Push block              */
      for( uiParam = 1; uiParam <= uiPCount; ++uiParam )
         hb_vmPush( hb_stackItemFromBase( uiParam ) );

      hb_vmDo( ( USHORT ) uiPCount );
   }
   else
      hb_errRT_BASE_SubstR( EG_NOMETHOD, 1004, NULL, "EVAL", pObject ? 1 : 0, pObject );
}

/* ================================================ */

/*
 * <hClass> := <obj>:ClassH()
 *
 * Returns class handle of <obj>
 */
static HARBOUR hb___msgClsH( void )
{
   hb_retni( hb_stackBaseItem()->item.asSymbol.stackstate->uiClass );
}


/*
 * <cClassName> := <obj>:ClassName()
 *
 * Return class name of <obj>. Can also be used for all types.
 */
static HARBOUR hb___msgClsName( void )
{
   USHORT uiClass = hb_stackBaseItem()->item.asSymbol.stackstate->uiClass;

   if( uiClass )
      hb_retc( s_pClasses[ uiClass - 1 ].szName );
   else
      hb_retc( hb_objGetClsName( hb_stackSelfItem() ) );
}


/*
 * <aMessages> := <obj>:ClassSel()
 *
 * Returns all the messages in <obj>
 */
static HARBOUR hb___msgClsSel( void )
{
   USHORT uiClass = hb_stackBaseItem()->item.asSymbol.stackstate->uiClass;

   if( uiClass && uiClass <= s_uiClasses )
   {
      PHB_ITEM pReturn = hb_itemNew( NULL );
      PCLASS  pClass  = &s_pClasses[ uiClass - 1 ];
      PMETHOD pMethod = pClass->pMethods;
      ULONG ulLimit = hb_clsMthNum( pClass ), ulPos = 0;
      USHORT nParam;

      nParam = hb_pcount() > 0 ? ( USHORT ) hb_parni( 1 ) : HB_MSGLISTALL;
      hb_arrayNew( pReturn, pClass->uiMethods );

      do
      {
         if( pMethod->pMessage )  /* Hash Entry used ? */
         {
            if( ( nParam == HB_MSGLISTALL )  ||
                ( nParam == HB_MSGLISTCLASS &&
                  (
                    ( pMethod->pFuncSym == &s___msgSetClsData ) ||
                    ( pMethod->pFuncSym == &s___msgGetClsData ) ||
                    ( pMethod->pFuncSym == &s___msgSetShrData ) ||
                    ( pMethod->pFuncSym == &s___msgGetShrData )
                  )
                ) ||
                ( nParam == HB_MSGLISTPURE &&
                  !(
                    ( pMethod->pFuncSym == &s___msgSetClsData ) ||
                    ( pMethod->pFuncSym == &s___msgGetClsData ) ||
                    ( pMethod->pFuncSym == &s___msgSetShrData ) ||
                    ( pMethod->pFuncSym == &s___msgGetShrData )
                   )
                )
              )
            {
               hb_itemPutC( hb_arrayGetItemPtr( pReturn, ++ulPos ),
                            pMethod->pMessage->pSymbol->szName );
            }
         }
         ++pMethod;
      }
      while( --ulLimit && ulPos < ( ULONG ) pClass->uiMethods );

      if( ulPos < ( ULONG ) pClass->uiMethods )
         hb_arraySize( pReturn, ulPos );
      hb_itemRelease( hb_itemReturn( pReturn ) );
   }
}

#if 0

/*
 * __msgClass()
 *
 * Internal function to return Self at Self:Class call (classy compatibility)
 */
static HARBOUR hb___msgClass( void )
{
   hb_itemReturn( hb_stackSelfItem() );
}

/* Added by JfL&RaC
 * <logical> <= <obj>:IsDerivedFrom( xParam )
 *
 * Return true if <obj> is derived from xParam.
 * xParam can be either an obj or a classname
 */
static HARBOUR hb___msgClsParent( void )
{
   char * szParentName = NULL;
   PHB_ITEM pItem;
   USHORT uiClass;

   uiClass = hb_stackBaseItem()->item.asSymbol.stackstate->uiClass;
   pItemParam = hb_param( 1, HB_IT_ANY );

   if( pItemParam )
   {
      if( HB_IS_OBJECT( pItemParam ) )
         szParentName = hb_objGetClsName( pItemParam );
      else if( HB_IS_STRING( pItemParam ) )
         szParentName = hb_parc( pItemParam );
   }

   hb_retl( szParentName && hb_clsIsParent( uiClass , szParentName ) );
}

#endif


/*
 * __msgEvalInline()
 *
 * Internal function executed for inline methods
 */
static HARBOUR hb___msgEvalInline( void )
{
   PHB_STACK_STATE pStack = hb_stackBaseItem()->item.asSymbol.stackstate;
   PCLASS pClass   = &s_pClasses[ pStack->uiClass - 1 ];
   PMETHOD pMethod = pClass->pMethods + pStack->uiMethod;
   USHORT uiPCount = hb_pcount(), uiParam;
   PHB_ITEM pBlock;

   hb_vmPushSymbol( &hb_symEval );

   hb_vmPush( hb_arrayGetItemPtr( pClass->pInlines, pMethod->uiData ) );
   pBlock = hb_stackItemFromTop( -1 );    /* Push block */
   pBlock->item.asBlock.hclass = pStack->uiClass;
   pBlock->item.asBlock.method = pStack->uiMethod;

   hb_vmPush( hb_stackSelfItem() );       /* Push self as first argument */

   for( uiParam = 1; uiParam <= uiPCount; uiParam++ )
   {
      hb_vmPush( hb_stackItemFromBase( uiParam ) );
   }

   hb_vmDo( uiPCount + 1 );
}

/*
 * __msgEval()
 *
 * Internal function for the internal EVAL method.
 */
static HARBOUR hb___msgEval( void )
{
   HB_ITEM_PTR pSelf = hb_stackSelfItem();

   if( HB_IS_BLOCK( pSelf ) )
   {
      USHORT uiParam;
      USHORT uiPCount = hb_pcount();

      hb_vmPushSymbol( &hb_symEval );
      hb_vmPush( pSelf );
      for( uiParam = 1; uiParam <= uiPCount; uiParam++ )
         hb_vmPush( hb_stackItemFromBase( uiParam ) );

      hb_vmDo( ( USHORT ) uiPCount );
   }
   else
      hb_errRT_BASE_SubstR( EG_NOMETHOD, 1004, NULL, "EVAL", HB_ERR_ARGS_SELFPARAMS );
}

/*
 * __msgNoMethod()
 *
 * Internal function for generating error when not existing message is sent
 */
static HARBOUR hb___msgNoMethod( void )
{
   PHB_SYMB pSym = hb_itemGetSymbol( hb_stackBaseItem() );

#if 1  /* Clipper compatible error message */
   if( pSym->szName[ 0 ] == '_' )
      hb_errRT_BASE_SubstR( EG_NOVARMETHOD, 1005, NULL, pSym->szName + 1, HB_ERR_ARGS_SELFPARAMS );
   else
      hb_errRT_BASE_SubstR( EG_NOMETHOD, 1004, NULL, pSym->szName, HB_ERR_ARGS_SELFPARAMS );
#else
   char szDesc[ 128 ];

   if( pSym->szName[ 0 ] == '_' )
   {
      sprintf( szDesc, "Class: '%s' has no property", hb_objGetClsName( hb_stackSelfItem() ) );
      hb_errRT_BASE_SubstR( EG_NOVARMETHOD, 1005, szDesc, pSym->szName + 1, HB_ERR_ARGS_BASEPARAMS );
   }
   else
   {
      sprintf( szDesc, "Class: '%s' has no exported method", hb_objGetClsName( hb_stackSelfItem() ) );
      hb_errRT_BASE_SubstR( EG_NOMETHOD, 1004, szDesc, pSym->szName, HB_ERR_ARGS_BASEPARAMS );
   }
#endif
}

/*
 * __msgScopeErr()
 *
 * Internal function for generating error when not existing message is sent
 */
static HARBOUR hb___msgScopeErr( void )
{
   char szProcName[ HB_SYMBOL_NAME_LEN + HB_SYMBOL_NAME_LEN + 5 ];
   PHB_ITEM pObject = hb_stackSelfItem();
   PMETHOD pMethod = s_pClasses[
      hb_stackBaseItem()->item.asSymbol.stackstate->uiClass - 1 ].pMethods +
      hb_stackBaseItem()->item.asSymbol.stackstate->uiMethod;

   strcpy( szProcName, s_pClasses[
                           pObject->item.asArray.value->uiClass - 1 ].szName );
   strcat( szProcName, ":" );
   strcat( szProcName, pMethod->pMessage->pSymbol->szName );

   if( pMethod->uiScope & HB_OO_CLSTP_HIDDEN )
      hb_errRT_BASE( EG_NOMETHOD, 41, "Scope violation (hidden)", szProcName, 0 );
   else
      hb_errRT_BASE( EG_NOMETHOD, 42, "Scope violation (protected)", szProcName, 0 );
}

/*
 * __msgSuper()
 *
 * Internal function to return a superobject
 */
static HARBOUR hb___msgSuper( void )
{
   PHB_ITEM pObject = hb_stackSelfItem();
   PHB_ITEM pCopy = hb_itemArrayNew(1);
   PMETHOD pMethod = s_pClasses[
      hb_stackBaseItem()->item.asSymbol.stackstate->uiClass - 1 ].pMethods +
      hb_stackBaseItem()->item.asSymbol.stackstate->uiMethod;

   /* Now save the Self object as the 1st elem. */
   hb_arraySet( pCopy, 1, pObject );

   /* And transform it into a fake object */
   /* backup of actual handel */
   pCopy->item.asArray.value->uiPrevCls = pObject->item.asArray.value->uiClass;
   /* superclass handel casting */
   pCopy->item.asArray.value->uiClass = pMethod->uiSprClass;

   hb_itemRelease( hb_itemReturn( pCopy ) );
}

/*
 * __msgRealClass()
 *
 * Internal function to return a superobject of class where the method was
 * defined
 */
static HARBOUR hb___msgRealClass( void )
{
   PHB_ITEM pObject = hb_stackSelfItem();
   USHORT uiClass = hb_clsSenderMethodClasss();

   if( uiClass &&
       hb_clsSenderObjectClasss() == pObject->item.asArray.value->uiClass &&
       uiClass != pObject->item.asArray.value->uiClass )
   {
      PHB_ITEM pCopy = hb_itemArrayNew(1);

      /* Now save the Self object as the 1st elem. */
      hb_arraySet( pCopy, 1, pObject );

      /* And transform it into a fake object */
      /* backup of actual handel */
      pCopy->item.asArray.value->uiPrevCls = pObject->item.asArray.value->uiClass;
      /* superclass handel casting */
      pCopy->item.asArray.value->uiClass = uiClass;
      hb_itemRelease( hb_itemReturn( pCopy ) );
   }
   else
   {
      hb_itemReturn( pObject );
   }
}

/*
 * __msgGetClsData()
 *
 * Internal function to return a CLASSDATA
 */
static HARBOUR hb___msgGetClsData( void )
{
   PCLASS pClass   = &s_pClasses[
                  hb_stackBaseItem()->item.asSymbol.stackstate->uiClass - 1 ];
   PMETHOD pMethod = pClass->pMethods +
                  hb_stackBaseItem()->item.asSymbol.stackstate->uiMethod;

   hb_arrayGet( pClass->pClassDatas, pMethod->uiData, hb_stackReturnItem() );
}


/*
 * __msgSetClsData()
 *
 * Internal function to set a CLASSDATA
 */
static HARBOUR hb___msgSetClsData( void )
{
   PCLASS pClass   = &s_pClasses[
                  hb_stackBaseItem()->item.asSymbol.stackstate->uiClass - 1 ];
   PMETHOD pMethod = pClass->pMethods +
                  hb_stackBaseItem()->item.asSymbol.stackstate->uiMethod;
   PHB_ITEM pReturn = hb_stackItemFromBase( 1 );

   hb_arraySet( pClass->pClassDatas, pMethod->uiData, pReturn );
   hb_itemReturn( pReturn );
}

/*
 * __msgGetShrData()
 *
 * Internal function to return a SHAREDDATA
 */
static HARBOUR hb___msgGetShrData( void )
{
   PCLASS pClass   = &s_pClasses[
                  hb_stackBaseItem()->item.asSymbol.stackstate->uiClass - 1 ];
   PMETHOD pMethod = pClass->pMethods +
                  hb_stackBaseItem()->item.asSymbol.stackstate->uiMethod;

   hb_arrayGet( s_pClasses[ pMethod->uiSprClass - 1 ].pSharedDatas,
                pMethod->uiData, hb_stackReturnItem() );
}

/*
 * __msgSetShrData()
 *
 * Internal function to set a SHAREDDATA
 */
static HARBOUR hb___msgSetShrData( void )
{
   PCLASS pClass   = &s_pClasses[
                  hb_stackBaseItem()->item.asSymbol.stackstate->uiClass - 1 ];
   PMETHOD pMethod = pClass->pMethods +
                  hb_stackBaseItem()->item.asSymbol.stackstate->uiMethod;
   PHB_ITEM pReturn = hb_stackItemFromBase( 1 );

   hb_arraySet( s_pClasses[ pMethod->uiSprClass - 1 ].pSharedDatas,
                pMethod->uiData, pReturn );
   hb_itemReturn( pReturn );
}

/*
 * __msgGetData()
 *
 * Internal function to return a DATA
 */
static HARBOUR hb___msgGetData( void )
{
   PHB_ITEM pObject  = hb_stackSelfItem();
   USHORT uiObjClass = pObject->item.asArray.value->uiClass;
   USHORT uiClass    = hb_stackBaseItem()->item.asSymbol.stackstate->uiClass;
   PCLASS pClass     = &s_pClasses[ uiClass - 1 ];
   PMETHOD pMethod   = pClass->pMethods +
                       hb_stackBaseItem()->item.asSymbol.stackstate->uiMethod;
   ULONG ulIndex     = pMethod->uiData;

   if( uiClass != uiObjClass )
   {
      ulIndex += hb_clsParentInstanceOffset( &s_pClasses[ uiObjClass - 1 ],
                           s_pClasses[ pMethod->uiSprClass - 1 ].pClassSym );
   }
   else
   {
      ulIndex += pMethod->uiOffset;
   }

   /* will arise only if the class has been modified after first instance */
   if( ulIndex > hb_arrayLen( pObject ) ) /* Resize needed */
      hb_arraySize( pObject, ulIndex );   /* Make large enough */

   hb_arrayGet( pObject, ulIndex, hb_stackReturnItem() );
}

/*
 * __msgSetData()
 *
 * Internal function to set a DATA
 */
static HARBOUR hb___msgSetData( void )
{
   PHB_ITEM pReturn  = hb_stackItemFromBase( 1 );
   PHB_ITEM pObject  = hb_stackSelfItem();
   USHORT uiObjClass = pObject->item.asArray.value->uiClass;
   USHORT uiClass    = hb_stackBaseItem()->item.asSymbol.stackstate->uiClass;
   PCLASS pClass     = &s_pClasses[ uiClass - 1 ];
   PMETHOD pMethod   = pClass->pMethods +
                       hb_stackBaseItem()->item.asSymbol.stackstate->uiMethod;
   ULONG ulIndex     = pMethod->uiData;

   if( uiClass != uiObjClass )
   {
      ulIndex += hb_clsParentInstanceOffset( &s_pClasses[ uiObjClass - 1 ],
                           s_pClasses[ pMethod->uiSprClass - 1 ].pClassSym );
   }
   else
   {
      ulIndex += pMethod->uiOffset;
   }

   /* will arise only if the class has been modified after first instance */
   if( ulIndex > hb_arrayLen( pObject ) ) /* Resize needed ? */
      hb_arraySize( pObject, ulIndex );   /* Make large enough */

   hb_arraySet( pObject, ulIndex, pReturn );

   hb_itemReturn( pReturn );
}

/* No comment :-) */
static HARBOUR hb___msgVirtual( void )
{
   /* hb_ret(); */ /* NOTE: It's safe to comment this out */
   ;
}

static HARBOUR hb___msgNull( void )
{
   ;
}


/* NOTE: Used by the preprocessor to implement Classy compatibility to Harbour
         Receive an variable number of param and return an array of it.
         No param will return a NULL array */


HB_FUNC( __CLS_PARAM )
{
   PHB_ITEM array;
   USHORT uiParam = ( USHORT ) hb_pcount();
   USHORT n;

   if( uiParam >= 1 )
   {
      array = hb_itemArrayNew( uiParam );
      for( n = 1; n <= uiParam; n++ )
      {
         hb_arraySet( array, n, hb_param( n, HB_IT_ANY ) );
      }
   }
   else
   {
      array = hb_itemArrayNew( 1 );
      hb_itemPutC( hb_arrayGetItemPtr( array, 1 ), "HBObject" );
   }

   hb_itemRelease( hb_itemReturn( array ) );
}

/* This one is used when HB_NOTOBJECT is defined before HBCLASS.CH */
/* it will avoid any default object to be inherited */
HB_FUNC( __CLS_PAR00 )
{
   PHB_ITEM array;
   USHORT uiParam = ( USHORT ) hb_pcount();
   USHORT n;

   array = hb_itemArrayNew( uiParam );
   for( n = 1; n <= uiParam; n++ )
   {
      hb_arraySet( array, n, hb_param( n, HB_IT_ANY ) );
   }

   hb_itemRelease( hb_itemReturn( array ) );
}

#ifndef HB_NO_PROFILER
void hb_mthAddTime( ULONG ulClockTicks )
{
   PMETHOD pMethod =
            s_pClasses[ hb_stackSelfItem()->item.asArray.value->uiClass - 1 ].
            pMethods + hb_stackBaseItem()->item.asSymbol.stackstate->uiMethod;

   pMethod->ulCalls++;
   pMethod->ulTime += ulClockTicks;
}
#endif

HB_FUNC( __GETMSGPRF ) /* profiler: returns a method called and consumed times */
                       /* ( nClass, cMsg ) --> aMethodInfo { nTimes, nTime } */
{
#ifndef HB_NO_PROFILER
   USHORT uiClass = ( USHORT ) hb_parni( 1 );
   char * cMsg    = hb_parc( 2 );

   hb_reta( 2 );
   if( uiClass && uiClass <= s_uiClasses && cMsg && *cMsg )
   {
      PHB_DYNS pMsg = hb_dynsymFindName( cMsg );

      if( pMsg )
      {
         PMETHOD pMethod = hb_clsFindMsg( &s_pClasses[ uiClass - 1 ], pMsg );

         if( pMethod )
         {
            hb_stornl( pMethod->ulCalls, -1, 1 );
            hb_stornl( pMethod->ulTime, -1, 2 );
            return;
         }
      }
   }
#else
   hb_reta( 2 );
#endif
   hb_stornl( 0, -1, 1 );
   hb_stornl( 0, -1, 2 );
}

/* __ClsGetProperties( nClassHandle ) --> aPropertiesNames
 * Notice that this function works quite similar to __CLASSSEL()
 * except that just returns the name of the datas and methods
 * that have been declared as PROPERTY (or PERSISTENT) */

HB_FUNC( __CLSGETPROPERTIES )
{
   USHORT uiClass = ( USHORT ) hb_parni( 1 );
   PHB_ITEM pReturn = hb_itemNew( NULL );

   if( uiClass && uiClass <= s_uiClasses )
   {
      PCLASS  pClass  = &s_pClasses[ uiClass - 1 ];
      PMETHOD pMethod = pClass->pMethods;
      ULONG ulLimit = hb_clsMthNum( pClass );
      PHB_ITEM pItem = NULL;

      hb_arrayNew( pReturn, 0 );

      do
      {
         if( pMethod->pMessage && ( pMethod->uiScope & HB_OO_CLSTP_PERSIST ) )
         {
            pItem = hb_itemPutC( pItem, pMethod->pMessage->pSymbol->szName );
            hb_arrayAdd( pReturn, pItem );
         }
         ++pMethod;
      }
      while( --ulLimit );

      if( pItem )
         hb_itemRelease( pItem );
   }

   hb_itemRelease( hb_itemReturn( pReturn ) );
}

/* Real dirty function, though very usefull under certain circunstances:
 * It allows to change the class handle of an object into another class handle,
 * so the object behaves like a different Class of object.
 * Based on objects.lib SetClsHandle() */

HB_FUNC( HB_SETCLSHANDLE ) /* ( oObject, nClassHandle ) --> nPrevClassHandle */
{
   PHB_ITEM pObject = hb_param( 1, HB_IT_OBJECT );
   USHORT uiPrevClassHandle = 0;

   if( pObject )
   {
      uiPrevClassHandle = pObject->item.asArray.value->uiClass;
      pObject->item.asArray.value->uiClass = ( USHORT ) hb_parni( 2 );
   }

   hb_retnl( uiPrevClassHandle );
}

/* Harbour equivalent for Clipper internal __mdCreate() */
USHORT hb_clsCreate( USHORT usSize, char * szClassName )
{
   static PHB_DYNS pDynSym = NULL;

   if( pDynSym == NULL )
      pDynSym = hb_dynsymGet( "__CLSNEW" );

   hb_vmPushDynSym( pDynSym );
   hb_vmPushNil();
   hb_vmPushString( szClassName, strlen( szClassName ) );
   hb_vmPushLong( usSize );
   hb_vmFunction( 2 );

   return ( USHORT ) hb_parni( -1 );
}

/* Harbour equivalent for Clipper internal __mdAdd() */
void hb_clsAdd( USHORT usClassH, char * szMethodName, PHB_FUNC pFuncPtr )
{
   static PHB_DYNS pDynSym = NULL;
   PHB_SYMB pExecSym;

   /*
    * We can use empty name "" for this symbol in hb_symbolNew()
    * It's only envelop for function with additional execution
    * information for HVM not registered symbol. [druzus]
    */
   pExecSym = hb_symbolNew( "" );
   pExecSym->value.pFunPtr = pFuncPtr;

   if( pDynSym == NULL )
      pDynSym = hb_dynsymGet( "__CLSADDMSG" );

   hb_vmPushDynSym( pDynSym );
   hb_vmPushNil();
   hb_vmPushInteger( usClassH );
   hb_vmPushString( szMethodName, strlen( szMethodName ) );
   hb_vmPushSymbol( pExecSym );
   hb_vmFunction( 3 );
}

/* Harbour equivalent for Clipper internal __mdAssociate() */
void hb_clsAssociate( USHORT usClassH )
{
   PHB_ITEM pSelf = hb_clsInst( usClassH );

   if( pSelf )
      hb_itemRelease( hb_itemReturn( pSelf ) );
}

/*
 * This function is only for backward binary compatibility
 * It will be removed in the future so please do not use it.
 * Use hb_objHasMessage() instead.
 */
#if defined(__cplusplus)
   extern "C" BOOL hb_objGetpMethod( PHB_ITEM pObject, PHB_SYMB pMessage );
#endif
BOOL hb_objGetpMethod( PHB_ITEM pObject, PHB_SYMB pMessage )
{
   return hb_objHasMessage( pObject, pMessage->pDynSym );
}
