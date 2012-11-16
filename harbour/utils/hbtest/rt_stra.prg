/*
 * $Id$
 */

/*
 * Harbour Project source code:
 * Regression tests for the runtime library (strings)
 *
 * Copyright 1999-2001 Viktor Szakats (harbour syenar.net)
 * www - http://harbour-project.org
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

#include "rt_main.ch"

/* Don't change the position of this #include. */
#include "rt_vars.ch"

PROCEDURE Main_STRA()

   /* Str() */

   TEST_LINE( Str(NIL)                        , "E 1 BASE 1099 Argument error (STR) OS:0 #:0 A:1:U:NIL F:S" )
   TEST_LINE( Str("A", 10, 2)                 , "E 1 BASE 1099 Argument error (STR) OS:0 #:0 A:3:C:A;N:10;N:2 F:S" )
   TEST_LINE( Str(100, 10, "A")               , "E 1 BASE 1099 Argument error (STR) OS:0 #:0 A:3:N:100;N:10;C:A F:S" )
   TEST_LINE( Str(100, 10, NIL)               , "E 1 BASE 1099 Argument error (STR) OS:0 #:0 A:3:N:100;N:10;U:NIL F:S" )
   TEST_LINE( Str(100, NIL, NIL)              , "E 1 BASE 1099 Argument error (STR) OS:0 #:0 A:3:N:100;U:NIL;U:NIL F:S" )
   IF TEST_DBFAvail()
   TEST_LINE( Str( w_TEST->TYPE_N_I )         , "        100"    )
   TEST_LINE( Str( w_TEST->TYPE_N_IE )        , "          0"    )
   TEST_LINE( Str( w_TEST->TYPE_N_D )         , "    101.127"    )
   TEST_LINE( Str( w_TEST->TYPE_N_DE )        , "      0.000"    )
   ENDIF
   TEST_LINE( Str(5000000000.0)               , "5000000000.0"   )
   TEST_LINE( Str(50000000)                   , "  50000000"     )
   TEST_LINE( Str(500000000)                  , " 500000000"     )
   TEST_LINE( Str(5000000000)                 , " 5000000000"    )
   TEST_LINE( Str(50000000000)                , " 50000000000"   )
   TEST_LINE( Str(-5000000000.0)              , "         -5000000000.0" )
   TEST_LINE( Str(-5000000000)                , "         -5000000000"   )
   TEST_LINE( Str(2.0000000000000001)         , "         2.0000000000000000" )
   TEST_LINE( Str(2.0000000000000009)         , "         2.0000000000000010" )
   TEST_LINE( Str(2.000000000000001)          , "         2.000000000000001"  )
   TEST_LINE( Str(2.000000000000009)          , "         2.000000000000009"  )
   TEST_LINE( Str(2.00000000000001)           , "         2.00000000000001"   )
   TEST_LINE( Str(2.00000000000009)           , "         2.00000000000009"   )
   TEST_LINE( Str(2.000000000001)             , "         2.000000000001"     )
   TEST_LINE( Str(2.00000000001)              , "         2.00000000001"      )
   TEST_LINE( Str(10)                         , "        10"     )
   TEST_LINE( Str(10.0)                       , "        10.0"   )
   TEST_LINE( Str(10.00)                      , "        10.00"  )
   TEST_LINE( Str(10.50)                      , "        10.50"  )
   TEST_LINE( Str(100000)                     , "    100000"     )
   TEST_LINE( Str(-10)                        , "       -10"     )
   TEST_LINE( Str(-10.0)                      , "       -10.0"   )
   TEST_LINE( Str(-10.00)                     , "       -10.00"  )
   TEST_LINE( Str(-10.50)                     , "       -10.50"  )
   TEST_LINE( Str(-100000)                    , "   -100000"     )
   TEST_LINE( Str(10, 5)                      , "   10"          )
   TEST_LINE( Str(10.0, 5)                    , "   10"          )
   TEST_LINE( Str(10.00, 5)                   , "   10"          )
   TEST_LINE( Str(10.50, 5)                   , "   11"          )
   TEST_LINE( Str(100000, 5)                  , "*****"          )
   TEST_LINE( Str(100000, 8)                  , "  100000"       )
   TEST_LINE( Str(-10, 5)                     , "  -10"          )
   TEST_LINE( Str(-10.0, 5)                   , "  -10"          )
   TEST_LINE( Str(-10.00, 5)                  , "  -10"          )
   TEST_LINE( Str(-10.50, 5)                  , "  -11"          )
   TEST_LINE( Str(-100000, 5)                 , "*****"          )
   TEST_LINE( Str(-100000, 6)                 , "******"         )
   TEST_LINE( Str(-100000, 8)                 , " -100000"       )
#ifndef __XPP__ /* Internal structures corrupted */
   TEST_LINE( Str(10, -5)                     , "        10"     )
   TEST_LINE( Str(10.0, -5)                   , "        10"     )
   TEST_LINE( Str(10.00, -5)                  , "        10"     )
   TEST_LINE( Str(10.50, -5)                  , "        11"     )
   TEST_LINE( Str(100000, -5)                 , "    100000"     )
   TEST_LINE( Str(100000, -8)                 , "    100000"     )
   TEST_LINE( Str(-10, -5)                    , "       -10"     )
   TEST_LINE( Str(-10.0, -5)                  , "       -10"     )
   TEST_LINE( Str(-10.00, -5)                 , "       -10"     )
   TEST_LINE( Str(-10.50, -5)                 , "       -11"     )
   TEST_LINE( Str(-100000, -5)                , "   -100000"     )
   TEST_LINE( Str(-100000, -6)                , "   -100000"     )
   TEST_LINE( Str(-100000, -8)                , "   -100000"     )
#endif
   TEST_LINE( Str(10, 5, 0)                   , "   10"          )
   TEST_LINE( Str(10.0, 5, 0)                 , "   10"          )
   TEST_LINE( Str(10.00, 5, 0)                , "   10"          )
   TEST_LINE( Str(10.50, 5, 0)                , "   11"          )
   TEST_LINE( Str(100000, 5, 0)               , "*****"          )
   TEST_LINE( Str(-10, 5, 0)                  , "  -10"          )
   TEST_LINE( Str(-10.0, 5, 0)                , "  -10"          )
   TEST_LINE( Str(-10.00, 5, 0)               , "  -10"          )
   TEST_LINE( Str(-10.50, 5, 0)               , "  -11"          )
   TEST_LINE( Str(-100000, 5, 0)              , "*****"          )
   TEST_LINE( Str(-100000, 6, 0)              , "******"         )
   TEST_LINE( Str(-100000, 8, 0)              , " -100000"       )
   TEST_LINE( Str(10, 5, 1)                   , " 10.0"          )
   TEST_LINE( Str(10.0, 5, 1)                 , " 10.0"          )
   TEST_LINE( Str(10.00, 5, 1)                , " 10.0"          )
   TEST_LINE( Str(10.50, 5, 1)                , " 10.5"          )
   TEST_LINE( Str(100000, 5, 1)               , "*****"          )
   TEST_LINE( Str(-10, 5, 1)                  , "-10.0"          )
   TEST_LINE( Str(-10.0, 5, 1)                , "-10.0"          )
   TEST_LINE( Str(-10.00, 5, 1)               , "-10.0"          )
   TEST_LINE( Str(-10.50, 5, 1)               , "-10.5"          )
   TEST_LINE( Str(-100000, 5, 1)              , "*****"          )
   TEST_LINE( Str(-100000, 6, 1)              , "******"         )
   TEST_LINE( Str(-100000, 8, 1)              , "********"       )
#ifndef __XPP__ /* Internal structures corrupted */
   TEST_LINE( Str(10, 5, -1)                  , "   10"          )
   TEST_LINE( Str(10.0, 5, -1)                , "   10"          )
   TEST_LINE( Str(10.00, 5, -1)               , "   10"          )
   TEST_LINE( Str(10.50, 5, -1)               , "   11"          )
   TEST_LINE( Str(100000, 5, -1)              , "*****"          )
   TEST_LINE( Str(-10, 5, -1)                 , "  -10"          )
   TEST_LINE( Str(-10.0, 5, -1)               , "  -10"          )
   TEST_LINE( Str(-10.00, 5, -1)              , "  -10"          )
   TEST_LINE( Str(-10.50, 5, -1)              , "  -11"          )
   TEST_LINE( Str(-100000, 5, -1)             , "*****"          )
   TEST_LINE( Str(-100000, 6, -1)             , "******"         )
   TEST_LINE( Str(-100000, 8, -1)             , " -100000"       )
#endif

   /* StrZero() */

#ifdef HB_CLP_STRICT
   TEST_LINE( StrZero(NIL)                    , "E 1 BASE 1099 Argument error (STR) OS:0 #:0 A:1:U:NIL F:S" )
   TEST_LINE( StrZero("A", 10, 2)             , "E 1 BASE 1099 Argument error (STR) OS:0 #:0 A:3:C:A;N:10;N:2 F:S" )
   TEST_LINE( StrZero(100, 10, "A")           , "E 1 BASE 1099 Argument error (STR) OS:0 #:0 A:3:N:100;N:10;C:A F:S" )
   TEST_LINE( StrZero(100, 10, NIL)           , "E 1 BASE 1099 Argument error (STR) OS:0 #:0 A:3:N:100;N:10;U:NIL F:S" )
   TEST_LINE( StrZero(100, NIL, NIL)          , "E 1 BASE 1099 Argument error (STR) OS:0 #:0 A:3:N:100;U:NIL;U:NIL F:S" )
#else
   TEST_LINE( StrZero(NIL)                    , "E 1 BASE 6003 Argument error (STRZERO) OS:0 #:0 A:1:U:NIL F:S" )
   TEST_LINE( StrZero("A", 10, 2)             , "E 1 BASE 6003 Argument error (STRZERO) OS:0 #:0 A:3:C:A;N:10;N:2 F:S" )
   TEST_LINE( StrZero(100, 10, "A")           , "E 1 BASE 6003 Argument error (STRZERO) OS:0 #:0 A:3:N:100;N:10;C:A F:S" )
   TEST_LINE( StrZero(100, 10, NIL)           , "E 1 BASE 6003 Argument error (STRZERO) OS:0 #:0 A:3:N:100;N:10;U:NIL F:S" )
   TEST_LINE( StrZero(100, NIL, NIL)          , "E 1 BASE 6003 Argument error (STRZERO) OS:0 #:0 A:3:N:100;U:NIL;U:NIL F:S" )
#endif
   TEST_LINE( StrZero(10)                     , "0000000010"     )
   TEST_LINE( StrZero(10.0)                   , "0000000010.0"   )
   TEST_LINE( StrZero(10.00)                  , "0000000010.00"  )
   TEST_LINE( StrZero(10.50)                  , "0000000010.50"  )
   TEST_LINE( StrZero(100000)                 , "0000100000"     )
   TEST_LINE( StrZero(-10)                    , "-000000010"     )
   TEST_LINE( StrZero(-10.0)                  , "-000000010.0"   )
   TEST_LINE( StrZero(-10.00)                 , "-000000010.00"  )
   TEST_LINE( StrZero(-10.50)                 , "-000000010.50"  )
   TEST_LINE( StrZero(-100000)                , "-000100000"     )
   TEST_LINE( StrZero(10, 5)                  , "00010"          )
   TEST_LINE( StrZero(10.0, 5)                , "00010"          )
   TEST_LINE( StrZero(10.00, 5)               , "00010"          )
   TEST_LINE( StrZero(10.50, 5)               , "00011"          )
   TEST_LINE( StrZero(100000, 5)              , "*****"          )
   TEST_LINE( StrZero(100000, 8)              , "00100000"       )
   TEST_LINE( StrZero(-10, 5)                 , "-0010"          )
   TEST_LINE( StrZero(-10.0, 5)               , "-0010"          )
   TEST_LINE( StrZero(-10.00, 5)              , "-0010"          )
   TEST_LINE( StrZero(-10.50, 5)              , "-0011"          )
   TEST_LINE( StrZero(-100000, 5)             , "*****"          )
   TEST_LINE( StrZero(-100000, 6)             , "******"         )
   TEST_LINE( StrZero(-100000, 8)             , "-0100000"       )
#ifndef __XPP__ /* Internal structures corrupted */
   TEST_LINE( StrZero(10, -5)                 , "0000000010"     )
   TEST_LINE( StrZero(10.0, -5)               , "0000000010"     )
   TEST_LINE( StrZero(10.00, -5)              , "0000000010"     )
   TEST_LINE( StrZero(10.50, -5)              , "0000000011"     )
   TEST_LINE( StrZero(100000, -5)             , "0000100000"     )
   TEST_LINE( StrZero(100000, -8)             , "0000100000"     )
   TEST_LINE( StrZero(-10, -5)                , "-000000010"     )
   TEST_LINE( StrZero(-10.0, -5)              , "-000000010"     )
   TEST_LINE( StrZero(-10.00, -5)             , "-000000010"     )
   TEST_LINE( StrZero(-10.50, -5)             , "-000000011"     )
   TEST_LINE( StrZero(-100000, -5)            , "-000100000"     )
   TEST_LINE( StrZero(-100000, -6)            , "-000100000"     )
   TEST_LINE( StrZero(-100000, -8)            , "-000100000"     )
#endif
   TEST_LINE( StrZero(10, 5, 0)               , "00010"          )
   TEST_LINE( StrZero(10.0, 5, 0)             , "00010"          )
   TEST_LINE( StrZero(10.50, 5, 0)            , "00011"          )
   TEST_LINE( StrZero(100000, 5, 0)           , "*****"          )
   TEST_LINE( StrZero(-10, 5, 0)              , "-0010"          )
   TEST_LINE( StrZero(-10.0, 5, 0)            , "-0010"          )
   TEST_LINE( StrZero(-10.00, 5, 0)           , "-0010"          )
   TEST_LINE( StrZero(-10.50, 5, 0)           , "-0011"          )
   TEST_LINE( StrZero(-100000, 5, 0)          , "*****"          )
   TEST_LINE( StrZero(-100000, 6, 0)          , "******"         )
   TEST_LINE( StrZero(-100000, 8, 0)          , "-0100000"       )
   TEST_LINE( StrZero(10, 5, 1)               , "010.0"          )
   TEST_LINE( StrZero(10.0, 5, 1)             , "010.0"          )
   TEST_LINE( StrZero(10.50, 5, 1)            , "010.5"          )
   TEST_LINE( StrZero(100000, 5, 1)           , "*****"          )
   TEST_LINE( StrZero(-10, 5, 1)              , "-10.0"          )
   TEST_LINE( StrZero(-10.0, 5, 1)            , "-10.0"          )
   TEST_LINE( StrZero(-10.00, 5, 1)           , "-10.0"          )
   TEST_LINE( StrZero(-10.50, 5, 1)           , "-10.5"          )
   TEST_LINE( StrZero(-100000, 5, 1)          , "*****"          )
   TEST_LINE( StrZero(-100000, 6, 1)          , "******"         )
   TEST_LINE( StrZero(-100000, 8, 1)          , "********"       )
#ifndef __XPP__ /* Internal structures corrupted */
   TEST_LINE( StrZero(10, 5, -1)              , "00010"          )
   TEST_LINE( StrZero(10.0, 5, -1)            , "00010"          )
   TEST_LINE( StrZero(10.50, 5, -1)           , "00011"          )
   TEST_LINE( StrZero(100000, 5, -1)          , "*****"          )
   TEST_LINE( StrZero(-10, 5, -1)             , "-0010"          )
   TEST_LINE( StrZero(-10.0, 5, -1)           , "-0010"          )
   TEST_LINE( StrZero(-10.00, 5, -1)          , "-0010"          )
   TEST_LINE( StrZero(-10.50, 5, -1)          , "-0011"          )
   TEST_LINE( StrZero(-100000, 5, -1)         , "*****"          )
   TEST_LINE( StrZero(-100000, 6, -1)         , "******"         )
   TEST_LINE( StrZero(-100000, 8, -1)         , "-0100000"       )
#endif

   RETURN

PROCEDURE Comp_Str()
   LOCAL old_exact := Set( _SET_EXACT, .F. )

   TEST_LINE( "ABC" == "", .F. )
   TEST_LINE( "ABC" = "", .T. )
   TEST_LINE( "ABC" != "", .F. )
   TEST_LINE( "ABC" < "", .F. )
   TEST_LINE( "ABC" <= "", .T. )
   TEST_LINE( "ABC" > "", .F. )
   TEST_LINE( "ABC" >= "", .T. )
   TEST_LINE( "" == "ABC", .F. )
   TEST_LINE( "" = "ABC", .F. )
   TEST_LINE( "" != "ABC", .T. )
   TEST_LINE( "" < "ABC", .T. )
   TEST_LINE( "" <= "ABC", .T. )
   TEST_LINE( "" > "ABC", .F. )
   TEST_LINE( "" >= "ABC", .F. )
   TEST_LINE( "ABC" == " ", .F. )
   TEST_LINE( "ABC" = " ", .F. )
   TEST_LINE( "ABC" != " ", .T. )
   TEST_LINE( "ABC" < " ", .F. )
   TEST_LINE( "ABC" <= " ", .F. )
   TEST_LINE( "ABC" > " ", .T. )
   TEST_LINE( "ABC" >= " ", .T. )
   TEST_LINE( " " == "ABC", .F. )
   TEST_LINE( " " = "ABC", .F. )
   TEST_LINE( " " != "ABC", .T. )
   TEST_LINE( " " < "ABC", .T. )
   TEST_LINE( " " <= "ABC", .T. )
   TEST_LINE( " " > "ABC", .F. )
   TEST_LINE( " " >= "ABC", .F. )
   TEST_LINE( "ABC" == "ABC", .T. )
   TEST_LINE( "ABC" = "ABC", .T. )
   TEST_LINE( "ABC" != "ABC", .F. )
   TEST_LINE( "ABC" < "ABC", .F. )
   TEST_LINE( "ABC" <= "ABC", .T. )
   TEST_LINE( "ABC" > "ABC", .F. )
   TEST_LINE( "ABC" >= "ABC", .T. )
   TEST_LINE( "ABC" == "ABCD", .F. )
   TEST_LINE( "ABC" = "ABCD", .F. )
   TEST_LINE( "ABC" != "ABCD", .T. )
   TEST_LINE( "ABC" < "ABCD", .T. )
   TEST_LINE( "ABC" <= "ABCD", .T. )
   TEST_LINE( "ABC" > "ABCD", .F. )
   TEST_LINE( "ABC" >= "ABCD", .F. )
   TEST_LINE( "ABCD" == "ABC", .F. )
   TEST_LINE( "ABCD" = "ABC", .T. )
   TEST_LINE( "ABCD" != "ABC", .F. )
   TEST_LINE( "ABCD" < "ABC", .F. )
   TEST_LINE( "ABCD" <= "ABC", .T. )
   TEST_LINE( "ABCD" > "ABC", .F. )
   TEST_LINE( "ABCD" >= "ABC", .T. )
   TEST_LINE( "ABC" == "ABC ", .F. )
   TEST_LINE( "ABC" = "ABC ", .F. )
   TEST_LINE( "ABC" != "ABC ", .T. )
   TEST_LINE( "ABC" < "ABC ", .T. )
   TEST_LINE( "ABC" <= "ABC ", .T. )
   TEST_LINE( "ABC" > "ABC ", .F. )
   TEST_LINE( "ABC" >= "ABC ", .F. )
   TEST_LINE( "ABC " == "ABC", .F. )
   TEST_LINE( "ABC " = "ABC", .T. )
   TEST_LINE( "ABC " != "ABC", .F. )
   TEST_LINE( "ABC " < "ABC", .F. )
   TEST_LINE( "ABC " <= "ABC", .T. )
   TEST_LINE( "ABC " > "ABC", .F. )
   TEST_LINE( "ABC " >= "ABC", .T. )
   TEST_LINE( "ABC" == "DEF", .F. )
   TEST_LINE( "ABC" = "DEF", .F. )
   TEST_LINE( "ABC" != "DEF", .T. )
   TEST_LINE( "ABC" < "DEF", .T. )
   TEST_LINE( "ABC" <= "DEF", .T. )
   TEST_LINE( "ABC" > "DEF", .F. )
   TEST_LINE( "ABC" >= "DEF", .F. )
   TEST_LINE( "DEF" == "ABC", .F. )
   TEST_LINE( "DEF" = "ABC", .F. )
   TEST_LINE( "DEF" != "ABC", .T. )
   TEST_LINE( "DEF" < "ABC", .F. )
   TEST_LINE( "DEF" <= "ABC", .F. )
   TEST_LINE( "DEF" > "ABC", .T. )
   TEST_LINE( "DEF" >= "ABC", .T. )
   TEST_LINE( "ABC" == "DEFG", .F. )
   TEST_LINE( "ABC" = "DEFG", .F. )
   TEST_LINE( "ABC" != "DEFG", .T. )
   TEST_LINE( "ABC" < "DEFG", .T. )
   TEST_LINE( "ABC" <= "DEFG", .T. )
   TEST_LINE( "ABC" > "DEFG", .F. )
   TEST_LINE( "ABC" >= "DEFG", .F. )
   TEST_LINE( "DEFG" == "ABC", .F. )
   TEST_LINE( "DEFG" = "ABC", .F. )
   TEST_LINE( "DEFG" != "ABC", .T. )
   TEST_LINE( "DEFG" < "ABC", .F. )
   TEST_LINE( "DEFG" <= "ABC", .F. )
   TEST_LINE( "DEFG" > "ABC", .T. )
   TEST_LINE( "DEFG" >= "ABC", .T. )
   TEST_LINE( "ABCD" == "DEF", .F. )
   TEST_LINE( "ABCD" = "DEF", .F. )
   TEST_LINE( "ABCD" != "DEF", .T. )
   TEST_LINE( "ABCD" < "DEF", .T. )
   TEST_LINE( "ABCD" <= "DEF", .T. )
   TEST_LINE( "ABCD" > "DEF", .F. )
   TEST_LINE( "ABCD" >= "DEF", .F. )
   TEST_LINE( "DEF" == "ABCD", .F. )
   TEST_LINE( "DEF" = "ABCD", .F. )
   TEST_LINE( "DEF" != "ABCD", .T. )
   TEST_LINE( "DEF" < "ABCD", .F. )
   TEST_LINE( "DEF" <= "ABCD", .F. )
   TEST_LINE( "DEF" > "ABCD", .T. )
   TEST_LINE( "DEF" >= "ABCD", .T. )

   Set( _SET_EXACT, old_exact )
   RETURN

PROCEDURE Exact_Str()
   LOCAL old_exact := Set( _SET_EXACT, .T. )

   TEST_LINE( "ABC" == "", .F. )
   TEST_LINE( "ABC" = "", .F. )
   TEST_LINE( "ABC" != "", .T. )
   TEST_LINE( "ABC" < "", .F. )
   TEST_LINE( "ABC" <= "", .F. )
   TEST_LINE( "ABC" > "", .T. )
   TEST_LINE( "ABC" >= "", .T. )
   TEST_LINE( "" == "ABC", .F. )
   TEST_LINE( "" = "ABC", .F. )
   TEST_LINE( "" != "ABC", .T. )
   TEST_LINE( "" < "ABC", .T. )
   TEST_LINE( "" <= "ABC", .T. )
   TEST_LINE( "" > "ABC", .F. )
   TEST_LINE( "" >= "ABC", .F. )
   TEST_LINE( "ABC" == " ", .F. )
   TEST_LINE( "ABC" = " ", .F. )
   TEST_LINE( "ABC" != " ", .T. )
   TEST_LINE( "ABC" < " ", .F. )
   TEST_LINE( "ABC" <= " ", .F. )
   TEST_LINE( "ABC" > " ", .T. )
   TEST_LINE( "ABC" >= " ", .T. )
   TEST_LINE( " " == "ABC", .F. )
   TEST_LINE( " " = "ABC", .F. )
   TEST_LINE( " " != "ABC", .T. )
   TEST_LINE( " " < "ABC", .T. )
   TEST_LINE( " " <= "ABC", .T. )
   TEST_LINE( " " > "ABC", .F. )
   TEST_LINE( " " >= "ABC", .F. )
   TEST_LINE( "ABC" == "ABC", .T. )
   TEST_LINE( "ABC" = "ABC", .T. )
   TEST_LINE( "ABC" != "ABC", .F. )
   TEST_LINE( "ABC" < "ABC", .F. )
   TEST_LINE( "ABC" <= "ABC", .T. )
   TEST_LINE( "ABC" > "ABC", .F. )
   TEST_LINE( "ABC" >= "ABC", .T. )
   TEST_LINE( "ABC" == "ABCD", .F. )
   TEST_LINE( "ABC" = "ABCD", .F. )
   TEST_LINE( "ABC" != "ABCD", .T. )
   TEST_LINE( "ABC" < "ABCD", .T. )
   TEST_LINE( "ABC" <= "ABCD", .T. )
   TEST_LINE( "ABC" > "ABCD", .F. )
   TEST_LINE( "ABC" >= "ABCD", .F. )
   TEST_LINE( "ABCD" == "ABC", .F. )
   TEST_LINE( "ABCD" = "ABC", .F. )
   TEST_LINE( "ABCD" != "ABC", .T. )
   TEST_LINE( "ABCD" < "ABC", .F. )
   TEST_LINE( "ABCD" <= "ABC", .F. )
   TEST_LINE( "ABCD" > "ABC", .T. )
   TEST_LINE( "ABCD" >= "ABC", .T. )
   TEST_LINE( "ABC" == "ABC ", .F. )
   TEST_LINE( "ABC" = "ABC ", .T. )
   TEST_LINE( "ABC" != "ABC ", .F. )
   TEST_LINE( "ABC" < "ABC ", .F. )
   TEST_LINE( "ABC" <= "ABC ", .T. )
   TEST_LINE( "ABC" > "ABC ", .F. )
   TEST_LINE( "ABC" >= "ABC ", .T. )
   TEST_LINE( "ABC " == "ABC", .F. )
   TEST_LINE( "ABC " = "ABC", .T. )
   TEST_LINE( "ABC " != "ABC", .F. )
   TEST_LINE( "ABC " < "ABC", .F. )
   TEST_LINE( "ABC " <= "ABC", .T. )
   TEST_LINE( "ABC " > "ABC", .F. )
   TEST_LINE( "ABC " >= "ABC", .T. )
   TEST_LINE( "ABC" == "DEF", .F. )
   TEST_LINE( "ABC" = "DEF", .F. )
   TEST_LINE( "ABC" != "DEF", .T. )
   TEST_LINE( "ABC" < "DEF", .T. )
   TEST_LINE( "ABC" <= "DEF", .T. )
   TEST_LINE( "ABC" > "DEF", .F. )
   TEST_LINE( "ABC" >= "DEF", .F. )
   TEST_LINE( "DEF" == "ABC", .F. )
   TEST_LINE( "DEF" = "ABC", .F. )
   TEST_LINE( "DEF" != "ABC", .T. )
   TEST_LINE( "DEF" < "ABC", .F. )
   TEST_LINE( "DEF" <= "ABC", .F. )
   TEST_LINE( "DEF" > "ABC", .T. )
   TEST_LINE( "DEF" >= "ABC", .T. )
   TEST_LINE( "ABC" == "DEFG", .F. )
   TEST_LINE( "ABC" = "DEFG", .F. )
   TEST_LINE( "ABC" != "DEFG", .T. )
   TEST_LINE( "ABC" < "DEFG", .T. )
   TEST_LINE( "ABC" <= "DEFG", .T. )
   TEST_LINE( "ABC" > "DEFG", .F. )
   TEST_LINE( "ABC" >= "DEFG", .F. )
   TEST_LINE( "DEFG" == "ABC", .F. )
   TEST_LINE( "DEFG" = "ABC", .F. )
   TEST_LINE( "DEFG" != "ABC", .T. )
   TEST_LINE( "DEFG" < "ABC", .F. )
   TEST_LINE( "DEFG" <= "ABC", .F. )
   TEST_LINE( "DEFG" > "ABC", .T. )
   TEST_LINE( "DEFG" >= "ABC", .T. )
   TEST_LINE( "ABCD" == "DEF", .F. )
   TEST_LINE( "ABCD" = "DEF", .F. )
   TEST_LINE( "ABCD" != "DEF", .T. )
   TEST_LINE( "ABCD" < "DEF", .T. )
   TEST_LINE( "ABCD" <= "DEF", .T. )
   TEST_LINE( "ABCD" > "DEF", .F. )
   TEST_LINE( "ABCD" >= "DEF", .F. )
   TEST_LINE( "DEF" == "ABCD", .F. )
   TEST_LINE( "DEF" = "ABCD", .F. )
   TEST_LINE( "DEF" != "ABCD", .T. )
   TEST_LINE( "DEF" < "ABCD", .F. )
   TEST_LINE( "DEF" <= "ABCD", .F. )
   TEST_LINE( "DEF" > "ABCD", .T. )
   TEST_LINE( "DEF" >= "ABCD", .T. )

   Set( _SET_EXACT, old_exact )

   RETURN

PROCEDURE New_STRINGS()

#ifdef __HARBOUR__

   TEST_LINE( hb_ValToStr( 4 )                     , "         4"    )
   TEST_LINE( hb_ValToStr( 4.0 / 2 )               , "         2.00" )
   TEST_LINE( hb_ValToStr( "String" )              , "String"        )
   TEST_LINE( hb_ValToStr( hb_SToD( "20010101" ) ) , "2001.01.01"    )
   TEST_LINE( hb_ValToStr( NIL )                   , "NIL"           )
   TEST_LINE( hb_ValToStr( .F. )                   , ".F."           )
   TEST_LINE( hb_ValToStr( .T. )                   , ".T."           )

   TEST_LINE( hb_StrShrink()                       , ""              )
   TEST_LINE( hb_StrShrink( NIL )                  , ""              )
   TEST_LINE( hb_StrShrink( "" )                   , ""              )
   TEST_LINE( hb_StrShrink( "", -1 )               , ""              )
   TEST_LINE( hb_StrShrink( "", 0 )                , ""              )
   TEST_LINE( hb_StrShrink( "", 1 )                , ""              )
   TEST_LINE( hb_StrShrink( "", 10 )               , ""              )
   TEST_LINE( hb_StrShrink( "a" )                  , ""              )
   TEST_LINE( hb_StrShrink( "a", -1 )              , "a"             )
   TEST_LINE( hb_StrShrink( "a", 0 )               , "a"             )
   TEST_LINE( hb_StrShrink( "a", 1 )               , ""              )
   TEST_LINE( hb_StrShrink( "a", 10 )              , ""              )
   TEST_LINE( hb_StrShrink( "ab" )                 , "a"             )
   TEST_LINE( hb_StrShrink( "ab", -1 )             , "ab"            )
   TEST_LINE( hb_StrShrink( "ab", 0 )              , "ab"            )
   TEST_LINE( hb_StrShrink( "ab", 1 )              , "a"             )
   TEST_LINE( hb_StrShrink( "ab", 10 )             , ""              )
   TEST_LINE( hb_StrShrink( "ab" )                 , "a"             )
   TEST_LINE( hb_StrShrink( "ab", -2 )             , "ab"            )
   TEST_LINE( hb_StrShrink( "ab", 2 )              , ""              )
   TEST_LINE( hb_StrShrink( "hello" )              , "hell"          )
   TEST_LINE( hb_StrShrink( "hello", -1 )          , "hello"         )
   TEST_LINE( hb_StrShrink( "hello", 0 )           , "hello"         )
   TEST_LINE( hb_StrShrink( "hello", 1 )           , "hell"          )
   TEST_LINE( hb_StrShrink( "hello", 2 )           , "hel"           )
   TEST_LINE( hb_StrShrink( "hello", 3 )           , "he"            )
   TEST_LINE( hb_StrShrink( "hello", 4 )           , "h"             )
   TEST_LINE( hb_StrShrink( "hello", 5 )           , ""              )
   TEST_LINE( hb_StrShrink( "hello", 6 )           , ""              )
   TEST_LINE( hb_StrShrink( "hello", 7 )           , ""              )

#endif

   RETURN

PROCEDURE Long_STRINGS()

   TEST_LINE( Right( Space( 64 * 1024 - 5 ) + "12345 7890", 10                      ), "12345 7890"                                 )
   TEST_LINE( Len( Space( 81910 ) + "1234567890"                                    ), 81920                                        )
   TEST_LINE( ( "1234567890" + Space( 810910 ) ) - ( "1234567890" + Space( 810910 ) ), "12345678901234567890" + Space( 810910 * 2 ) )

   RETURN

/* Don't change the position of this #include. */
#include "rt_init.ch"
