/*
 * $Id$
 */

/*
 * Harbour Project source code:
 * QT wrapper main header
 *
 * Copyright 2009 Marcos Antonio Gambeta <marcosgambeta at gmail dot com>
 * Copyright 2009 Pritpal Bedi <pritpal@vouchcac.com>
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

#include "hbapi.h"
#include "hbqt.h"

/*----------------------------------------------------------------------*/
#if QT_VERSION >= 0x040500
/*----------------------------------------------------------------------*/

/*
 *  Constructed[ 20/25 [ 80.00% ] ]
 *
 *  *** Unconvered Prototypes ***
 *  -----------------------------
 *
 *  QList<int> pointSizes ( const QString & family, const QString & style = QString() )
 *  QList<int> smoothSizes ( const QString & family, const QString & style )
 *  QList<WritingSystem> writingSystems () const
 *  QList<WritingSystem> writingSystems ( const QString & family ) const
 *  QList<int> standardSizes ()
 */


#include <QtGui/QFontDatabase>


/*
 * QFontDatabase ()
 */
HB_FUNC( QT_QFONTDATABASE )
{
   hb_retptr( ( QFontDatabase* ) new QFontDatabase() );
}

/*
 * bool bold ( const QString & family, const QString & style ) const
 */
HB_FUNC( QT_QFONTDATABASE_BOLD )
{
   hb_retl( hbqt_par_QFontDatabase( 1 )->bold( hbqt_par_QString( 2 ), hbqt_par_QString( 3 ) ) );
}

/*
 * QStringList families ( WritingSystem writingSystem = Any ) const
 */
HB_FUNC( QT_QFONTDATABASE_FAMILIES )
{
   hb_retptr( new QStringList( hbqt_par_QFontDatabase( 1 )->families( ( HB_ISNUM( 2 ) ? ( QFontDatabase::WritingSystem ) hb_parni( 2 ) : ( QFontDatabase::WritingSystem ) QFontDatabase::Any ) ) ) );
}

/*
 * QFont font ( const QString & family, const QString & style, int pointSize ) const
 */
HB_FUNC( QT_QFONTDATABASE_FONT )
{
   hb_retptr( new QFont( hbqt_par_QFontDatabase( 1 )->font( hbqt_par_QString( 2 ), hbqt_par_QString( 3 ), hb_parni( 4 ) ) ) );
}

/*
 * bool isBitmapScalable ( const QString & family, const QString & style = QString() ) const
 */
HB_FUNC( QT_QFONTDATABASE_ISBITMAPSCALABLE )
{
   hb_retl( hbqt_par_QFontDatabase( 1 )->isBitmapScalable( hbqt_par_QString( 2 ), hbqt_par_QString( 3 ) ) );
}

/*
 * bool isFixedPitch ( const QString & family, const QString & style = QString() ) const
 */
HB_FUNC( QT_QFONTDATABASE_ISFIXEDPITCH )
{
   hb_retl( hbqt_par_QFontDatabase( 1 )->isFixedPitch( hbqt_par_QString( 2 ), hbqt_par_QString( 3 ) ) );
}

/*
 * bool isScalable ( const QString & family, const QString & style = QString() ) const
 */
HB_FUNC( QT_QFONTDATABASE_ISSCALABLE )
{
   hb_retl( hbqt_par_QFontDatabase( 1 )->isScalable( hbqt_par_QString( 2 ), hbqt_par_QString( 3 ) ) );
}

/*
 * bool isSmoothlyScalable ( const QString & family, const QString & style = QString() ) const
 */
HB_FUNC( QT_QFONTDATABASE_ISSMOOTHLYSCALABLE )
{
   hb_retl( hbqt_par_QFontDatabase( 1 )->isSmoothlyScalable( hbqt_par_QString( 2 ), hbqt_par_QString( 3 ) ) );
}

/*
 * bool italic ( const QString & family, const QString & style ) const
 */
HB_FUNC( QT_QFONTDATABASE_ITALIC )
{
   hb_retl( hbqt_par_QFontDatabase( 1 )->italic( hbqt_par_QString( 2 ), hbqt_par_QString( 3 ) ) );
}

/*
 * QString styleString ( const QFont & font )
 */
HB_FUNC( QT_QFONTDATABASE_STYLESTRING )
{
   hb_retc( hbqt_par_QFontDatabase( 1 )->styleString( *hbqt_par_QFont( 2 ) ).toLatin1().data() );
}

/*
 * QString styleString ( const QFontInfo & fontInfo )
 */
HB_FUNC( QT_QFONTDATABASE_STYLESTRING_1 )
{
   hb_retc( hbqt_par_QFontDatabase( 1 )->styleString( *hbqt_par_QFontInfo( 2 ) ).toLatin1().data() );
}

/*
 * QStringList styles ( const QString & family ) const
 */
HB_FUNC( QT_QFONTDATABASE_STYLES )
{
   hb_retptr( new QStringList( hbqt_par_QFontDatabase( 1 )->styles( hbqt_par_QString( 2 ) ) ) );
}

/*
 * int weight ( const QString & family, const QString & style ) const
 */
HB_FUNC( QT_QFONTDATABASE_WEIGHT )
{
   hb_retni( hbqt_par_QFontDatabase( 1 )->weight( hbqt_par_QString( 2 ), hbqt_par_QString( 3 ) ) );
}

/*
 * int addApplicationFont ( const QString & fileName )
 */
HB_FUNC( QT_QFONTDATABASE_ADDAPPLICATIONFONT )
{
   hb_retni( hbqt_par_QFontDatabase( 1 )->addApplicationFont( hbqt_par_QString( 2 ) ) );
}

/*
 * int addApplicationFontFromData ( const QByteArray & fontData )
 */
HB_FUNC( QT_QFONTDATABASE_ADDAPPLICATIONFONTFROMDATA )
{
   hb_retni( hbqt_par_QFontDatabase( 1 )->addApplicationFontFromData( *hbqt_par_QByteArray( 2 ) ) );
}

/*
 * QStringList applicationFontFamilies ( int id )
 */
HB_FUNC( QT_QFONTDATABASE_APPLICATIONFONTFAMILIES )
{
   hb_retptr( new QStringList( hbqt_par_QFontDatabase( 1 )->applicationFontFamilies( hb_parni( 2 ) ) ) );
}

/*
 * bool removeAllApplicationFonts ()
 */
HB_FUNC( QT_QFONTDATABASE_REMOVEALLAPPLICATIONFONTS )
{
   hb_retl( hbqt_par_QFontDatabase( 1 )->removeAllApplicationFonts() );
}

/*
 * bool removeApplicationFont ( int id )
 */
HB_FUNC( QT_QFONTDATABASE_REMOVEAPPLICATIONFONT )
{
   hb_retl( hbqt_par_QFontDatabase( 1 )->removeApplicationFont( hb_parni( 2 ) ) );
}

/*
 * bool supportsThreadedFontRendering ()
 */
HB_FUNC( QT_QFONTDATABASE_SUPPORTSTHREADEDFONTRENDERING )
{
   hb_retl( hbqt_par_QFontDatabase( 1 )->supportsThreadedFontRendering() );
}

/*
 * QString writingSystemName ( WritingSystem writingSystem )
 */
HB_FUNC( QT_QFONTDATABASE_WRITINGSYSTEMNAME )
{
   hb_retc( hbqt_par_QFontDatabase( 1 )->writingSystemName( ( QFontDatabase::WritingSystem ) hb_parni( 2 ) ).toLatin1().data() );
}

/*
 * QString writingSystemSample ( WritingSystem writingSystem )
 */
HB_FUNC( QT_QFONTDATABASE_WRITINGSYSTEMSAMPLE )
{
   hb_retc( hbqt_par_QFontDatabase( 1 )->writingSystemSample( ( QFontDatabase::WritingSystem ) hb_parni( 2 ) ).toLatin1().data() );
}


/*----------------------------------------------------------------------*/
#endif             /* #if QT_VERSION >= 0x040500 */
/*----------------------------------------------------------------------*/

