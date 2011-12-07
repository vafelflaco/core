/*
 * $Id$
 */

/*
 * Harbour Project source code:
 * Source file for the Wvg*Classes
 *
 * Copyright 2008 Pritpal Bedi <pritpal@vouchcac.com>
 * http://harbour-project.org
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
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*
 *                                EkOnkar
 *                          ( The LORD is ONE )
 *
 *                 Xbase++ xbpPushButton Compatible Class
 *
 *                  Pritpal Bedi <pritpal@vouchcac.com>
 *                               05Dec2008
 */
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#include "hbclass.ch"
#include "common.ch"
#include "inkey.ch"
#include "hbgtinfo.ch"

#include "hbgtwvg.ch"
#include "wvtwin.ch"
#include "wvgparts.ch"

/*----------------------------------------------------------------------*/

#ifndef __DBG_PARTS__
#xtranslate hb_traceLog( [<x,...>] ) =>
#endif

/*----------------------------------------------------------------------*/

CLASS WvgCheckBox  INHERIT  WvgWindow, DataRef

   DATA     autosize                              INIT .F.
   DATA     caption                               INIT ""
   DATA     pointerFocus                          INIT .T.
   DATA     selection                             INIT .F.

   METHOD   new( oParent, oOwner, aPos, aSize, aPresParams, lVisible )
   METHOD   create( oParent, oOwner, aPos, aSize, aPresParams, lVisible )
   METHOD   configure( oParent, oOwner, aPos, aSize, aPresParams, lVisible )
   METHOD   destroy()
   METHOD   handleEvent( nMessage, aNM )

   #if 0
   METHOD   editBuffer()                          INLINE ( WVG_Button_GetCheck( ::hWnd ) == BST_CHECKED )
   METHOD   getData()                             INLINE ( WVG_Button_GetCheck( ::hWnd ) == BST_CHECKED )
   METHOD   setData( lCheck )                     INLINE ::sendMessage( BM_SETCHECK, IF( lCheck, BST_CHECKED, BST_UNCHECKED ), 0 )
   #endif
   METHOD   setCaption( xCaption )

   ACCESS   selected                              INLINE ::sl_lbClick
   ASSIGN   selected( bBlock )                    INLINE ::sl_lbClick := bBlock

   ENDCLASS
/*----------------------------------------------------------------------*/

METHOD new( oParent, oOwner, aPos, aSize, aPresParams, lVisible ) CLASS WvgCheckBox

   ::wvgWindow:new( oParent, oOwner, aPos, aSize, aPresParams, lVisible )

   ::style       := WS_CHILD + BS_PUSHBUTTON + BS_AUTOCHECKBOX /*+ BS_NOTIFY */
   ::className   := "BUTTON"
   ::objType     := objTypeCheckBox

   RETURN Self

/*----------------------------------------------------------------------*/

METHOD create( oParent, oOwner, aPos, aSize, aPresParams, lVisible ) CLASS WvgCheckBox

   ::wvgWindow:create( oParent, oOwner, aPos, aSize, aPresParams, lVisible )

   ::oParent:AddChild( SELF )

   ::createControl()

   IF ::isParentCrt()
      ::SetWindowProcCallback()
   ENDIF

   IF ::visible
      ::show()
   ENDIF
   ::setPosAndSize()

   ::setCaption( ::caption )
   IF ::selection
      ::sendMessage( BM_SETCHECK, BST_CHECKED, 0 )
   ENDIF
   ::editBuffer := ( WVG_Button_GetCheck( ::hWnd ) == BST_CHECKED )

   RETURN Self

/*----------------------------------------------------------------------*/

METHOD handleEvent( nMessage, aNM ) CLASS WvgCheckBox
   LOCAL hDC

   hb_traceLog( "       %s:handleEvent( %i )", __ObjGetClsName( self ), nMessage )

   DO CASE

   CASE nMessage == HB_GTE_RESIZED
      ::sendMessage( WM_SIZE, 0, 0 )
      IF ::isParentCrt()
         ::rePosition()
      ENDIF

   CASE nMessage == HB_GTE_COMMAND
      IF aNM[ NMH_code ] == BN_CLICKED
         ::editBuffer := ( WVG_Button_GetCheck( ::hWnd ) == BST_CHECKED )
         IF hb_isBlock( ::sl_lbClick )
            eval( ::sl_lbClick, ::editBuffer, NIL, self )
            RETURN 0
         ENDIF
      ENDIF

   CASE nMessage == HB_GTE_CTLCOLOR
      hDC := aNM[ 1 ]
      IF hb_isNumeric( ::clr_FG )
         WVG_SetTextColor( hDC, ::clr_FG )
      ENDIF
      IF hb_isNumeric( ::hBrushBG )
         WVG_SetBkMode( hDC, 1 )
         RETURN ::hBrushBG
      ELSE
         RETURN WVG_GetCurrentBrush( hDC )
      ENDIF

   CASE nMessage == HB_GTE_ANY
      IF ::isParentCrt()
         IF aNM[ 1 ] == WM_LBUTTONUP
            IF hb_isBlock( ::sl_lbClick )
               ::oParent:setFocus()
               ::sendMessage( BM_SETCHECK, iif( WVG_Button_GetCheck( ::hWnd ) == BST_CHECKED, BST_UNCHECKED, BST_CHECKED ), 0 )
               ::editBuffer := ( WVG_Button_GetCheck( ::hWnd ) == BST_CHECKED )
               eval( ::sl_lbClick, NIL, NIL, Self )
            ENDIF
         ENDIF
      ENDIF

   ENDCASE

   RETURN EVENT_UNHANDELLED

/*----------------------------------------------------------------------*/

METHOD destroy() CLASS WvgCheckBox

   hb_traceLog( "          %s:destroy()", __objGetClsName() )

   ::wvgWindow:destroy()

   RETURN NIL

/*----------------------------------------------------------------------*/

METHOD configure( oParent, oOwner, aPos, aSize, aPresParams, lVisible ) CLASS WvgCheckBox

   ::Initialize( oParent, oOwner, aPos, aSize, aPresParams, lVisible )

   RETURN Self

/*----------------------------------------------------------------------*/

METHOD setCaption( xCaption ) CLASS WvgCheckBox

   IF hb_isChar( xCaption )
      ::caption := xCaption
      WVG_SendMessageText( ::hWnd, WM_SETTEXT, 0, ::caption )
   ENDIF

   RETURN Self

/*----------------------------------------------------------------------*/
