/*
 * $Id$
 */

/*
 * Harbour Project source code:
 * QT wrapper main header
 *
 * Copyright 2009 Pritpal Bedi <pritpal@vouchcac.com>
 *
 * Copyright 2009 Marcos Antonio Gambeta <marcosgambeta at gmail dot com>
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
/*----------------------------------------------------------------------*/


#include "hbclass.ch"


CREATE CLASS QModelIndex

   VAR     pParent
   VAR     pPtr

   METHOD  New()
   METHOD  Configure( xObject )
   METHOD  Destroy()                           INLINE  Qt_QModelIndex_destroy( ::pPtr )

   METHOD  child( nRow, nColumn )              INLINE  Qt_QModelIndex_child( ::pPtr, nRow, nColumn )
   METHOD  column()                            INLINE  Qt_QModelIndex_column( ::pPtr )
   METHOD  data( nRole )                       INLINE  Qt_QModelIndex_data( ::pPtr, nRole )
   METHOD  flags()                             INLINE  Qt_QModelIndex_flags( ::pPtr )
   METHOD  internalId()                        INLINE  Qt_QModelIndex_internalId( ::pPtr )
   METHOD  internalPointer()                   INLINE  Qt_QModelIndex_internalPointer( ::pPtr )
   METHOD  isValid()                           INLINE  Qt_QModelIndex_isValid( ::pPtr )
   METHOD  model()                             INLINE  Qt_QModelIndex_model( ::pPtr )
   METHOD  parent()                            INLINE  Qt_QModelIndex_parent( ::pPtr )
   METHOD  row()                               INLINE  Qt_QModelIndex_row( ::pPtr )
   METHOD  sibling( nRow, nColumn )            INLINE  Qt_QModelIndex_sibling( ::pPtr, nRow, nColumn )

   ENDCLASS

/*----------------------------------------------------------------------*/

METHOD New( pParent ) CLASS QModelIndex

   ::pParent := pParent

   ::pPtr := Qt_QModelIndex( pParent )

   RETURN Self

/*----------------------------------------------------------------------*/

METHOD Configure( xObject ) CLASS QModelIndex

   IF hb_isObject( xObject )
      ::pPtr := xObject:pPtr
   ELSEIF hb_isPointer( xObject )
      ::pPtr := xObject
   ENDIF

   RETURN Self

/*----------------------------------------------------------------------*/

