/*
 * $Id$
 */

/*
   Harbour Project source code

   This file contains the Harbour function(s) that maintain the list
   of set-keys (hot-keys).

   By: A White - awhite@user.rose.com
   www - http://www.Harbour-Project.org

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

   V 1.1  A White                 Fixed wrong parameter aClone() bug in SetKeySave()
                                  Added SetKeyCheck()
                                  Added SetKeyGet()
   V 1.0  A White                 Initial version, submitted to Harbour Projects
*/

// macro substitutions to access sub-array elements of aSetKeys[]
#define KEY        1
#define BLOCK      2
#define CONDITION  3

static aSetKeys := {}       // holds array of hot-key id, code-block, activation-block

/*  $DOC$
 *  $FUNCNAME$
 *      SetKey
 *  $CATEGORY$
 *      ?
 *  $ONELINER$
 *      Assign an action block to a key
 *  $SYNTAX$
 *      SETKEY( <anKey> [, <bAction> [, <bCondition> ] ] )
 *  $ARGUMENTS$
 *      <anKey> is either a numeric key value, or an array of such values
 *      <bAction> is an optional code-block to be assigned
 *      <bCondition> is an optional condition code-block
 *  $RETURNS$
 *      Current assigned action-block
 *  $DESCRIPTION$
 *      The SetKey() function returns the current code-block assigned to a
 *      key when called with only the key value.  If the action block (and
 *      optionally the condition block) are passed, the current block is
 *      returned, and the new code block and condition block are stored.
 *      A group of keys may be assigned the same code block/condition block
 *      by using an array of key values in place on the first parameter.
 *  $EXAMPLES$
 *      local bOldF10 := setKey( K_F10, {|| Yahoo() } )
 *      ... // some other processing
 *      SetKey( K_F10, bOldF10 )
 *      ... // some other processing
 *      bBlock := SetKey( K_SPACE )
 *      if bBlock != NIL ...
 *
 *      // make F10 exit current get, but only if in a get - ignores other
 *      // wait-states such as menus, achoices, etc...
 *      SetKey( K_F10, {|| GetActive():State := GE_WRITE }, {|| GetActive() != NIL } )
 *  $TESTS$
 *      None definable
 *  $STATUS$
 *      C
 *  $COMPLIANCE$
 *      SETKEY() is mostly CA-Clipper compliant. The only difference is the
 *      addition of the condition code-block parameter, allowing set-keys to
 *      be conditionally turned off or on.  This condition-block cannot be
 *      returned once set - see SetKeyGet()
 *  $SEEALSO$
 *      SETKEYSAVE()
 *  $END$
 */
Function SetKey( anKey, bBlock, bCondition )
  local nFound, bReturn, aKey

  if valType( anKey ) = "A"
    aEval( anKey, {|x| setKey( x, bBlock, bCondition ) } )

  elseif valType( anKey ) = "N" .and. anKey <> 0
    if ( nFound := aScan( aSetKeys, {|x| x[ KEY ] = anKey } ) ) = 0
      if valType( bBlock ) = "B"
        aAdd( aSetKeys, { anKey, bBlock, bCondition } )

      endif

    else
      aKey := aSetKeys[ nFound ]

      if aKey[ CONDITION ] = NIL .or. eval( aKey[ CONDITION ], anKey )
        bReturn := aKey[ BLOCK ]

      endif

      if valType( bBlock ) = "B"
        aKey[ BLOCK ]     := bBlock
        aKey[ CONDITION ] := bCondition

      elseif pcount() > 1 .and. bBlock = NIL
        aSize( aDel( aSetKeys, nFound ), len( aSetKeys ) - 1 )

      endif

    endif

  endif

return bReturn


/*  $DOC$
 *  $FUNCNAME$
 *      HB_SetKeyGet
 *  $CATEGORY$
 *      ?
 *  $ONELINER$
 *      Determine a set-key code block & condition-block
 *  $SYNTAX$
 *      HB_SETKEYGET( <nKey> [, <bConditionByRef> ] )
 *  $ARGUMENTS$
 *      <anKey> is an numeric key value
 *      <bConditionByRef> is an optional return-parameter
 *  $RETURNS$
 *      Current assigned action-block
 *  $DESCRIPTION$
 *      The HB_SetKeyGet() function returns the current code-block assigned to 
 *      a key, and optionally assignes the condition-block to the 
 *      return-parameter
 *  $EXAMPLES$
 *      local bOldF10, bOldF10Cond
 *      bOldF10 := HB_SetKeyGet( K_F10, @bOldF10Cond )
 *      ... // some other processing
 *      SetKey( K_F10, bOldF10, bOldF10Cond )
 *  $TESTS$
 *      See test code above
 *  $STATUS$
 *      C
 *  $COMPLIANCE$
 *      HB_SETKEYGET() is a new function and hence not CA-Clipper compliant.
 *  $SEEALSO$
 *      SETKEY(), HB_SETKEYSAVE(), HB_SETKEYCHECK()
 *  $END$
 */
Function HB_SetKeyGet( nKey, bCondition )
  local nFound

  if valType( nKey ) = "N" .and. nKey <> 0
    if ( nFound := aScan( aSetKeys, {|x| x[ KEY ] = nKey } ) ) = 0
      bCondition := NIL

    else
      bCondition := aSetKeys[ nFound, CONDITION ]
      return        aSetKeys[ nFound, BLOCK ]

    endif

  endif

return NIL //bReturn


/*  $DOC$
 *  $FUNCNAME$
 *      HB_SetKeySave
 *  $CATEGORY$
 *      ?
 *  $ONELINER$
 *      Returns a copy of internal set-key list, optionally overwriting
 *  $SYNTAX$
 *      HB_SETKEYSAVE( [ <OldKeys> ] )
 *  $ARGUMENTS$
 *      <OldKeys> is an optional set-key list from a previous call to
 *      HB_SetKeySave(), or NIL to clear current set-key list
 *  $RETURNS$
 *      Current set-key list
 *  $DESCRIPTION$
 *      HB_SetKeySave() is designed to act like the set() function which 
 *      returns the current state of an environment setting, and optionally 
 *      assigning a new value.  In this case, the "environment setting" is the 
 *      internal set-key list, and the optional new value is either a value 
 *      returned from a previous call to SetKeySave() - to restore that list, 
 *      or the value of NIL to clear the current list.
 *  $EXAMPLES$
 *      local aKeys := HB_SetKeySave( NIL )  // removes all current set=keys
 *      ... // some other processing
 *      HB_SetKeySave( aKeys )
 *  $TESTS$
 *      None definable
 *  $STATUS$
 *      C
 *  $COMPLIANCE$
 *      HB_SETKEYSAVE() is new.
 *  $SEEALSO$
 *      SETKEY()
 *  $END$
 */
Function HB_SetKeySave( OldKeys )
  local aReturn := aClone( aSetKeys )

  if pcount() != 0 .or. valtype( OldKeys ) = "A"
    if OldKeys = NIL
      aSetKeys := {}

    else
      aSetKeys := aClone( OldKeys )

    endif

  endif

return aReturn


/*  $DOC$
 *  $FUNCNAME$
 *      HB_SetKeyCheck
 *  $CATEGORY$
 *      ?
 *  $ONELINER$
 *      Impliments common hot-key activation code
 *  $SYNTAX$
 *      HB_SetKeyCheck( <nKey> [, <p1> ][, <p2> ][, <p3> ] )
 *  $ARGUMENTS$
 *      <nKey> is a numeric key value to be tested
 *      code-block, if executed
 *      <p1>..<p3> are optional parameters that will be passed to the code-block
 *  $RETURNS$
 *      True if there is a hot-key associated with <nKey> and it was executed;
 *      otherwise False
 *      If there is a hot-key association (before checking any condition):
 *        - if there is a condition-block, it is passed one parameter - <nKey>
 *        - when the hot-key code-block is called, it is passed 1 to 4 parameters,
 *          depending on the parameters passed to HB_SetKeyCheck().  Any 
 *          parameters so passed are directly passed to the code-block, with an 
 *          additional parameter being <nKey>
 *  $DESCRIPTION$
 *      HB_SetKeyCheck() is intended as a common interface to the SetKey()
 *      functionality for such functions as ACHOICE(), DBEDIT(), MEMOEDIT(),
 *      ACCEPT, INPUT, READ, and WAIT
 *  $EXAMPLES$
 *      // within ReadModal()
 *      if HB_SetKeyCheck( K_ALT_X, GetActive() )
 *      ... // some other processing
 *      endif
 *      // within TBrowse handler
 *      case HB_SetKeyCheck( nInkey, oTBrowse )
 *        return
 *      case nInKey == K_ESC
 *      ... // some other processing
 *  $TESTS$
 *      None definable
 *  $STATUS$
 *      C
 *  $COMPLIANCE$
 *      HB_SetKeyCheck() is new.
 *  $SEEALSO$
 *      SETKEY(), HB_SETKEYSAVE()
 *  $END$
 */
Function HB_SetKeyCheck( nKey, p1, p2, p3 )
  local nFound, aKey, bBlock

  if ( nFound := aScan( aSetKeys, {|x| x[ KEY ] = nKey } ) ) > 0
    aKey   := aSetKeys[ nFound ]
    bBLock := aKey[ BLOCK ]

    if aKey[ CONDITION ] = NIL .or. eval( aKey[ CONDITION ], nKey )

      // is this overkill?  if a code-block checks its own pcount(),
      // passing nil parameters would skew the count!

      do case
      case pcount() = 1  ;  eval( bBlock, nKey )
      case pcount() = 2  ;  eval( bBlock, p1, nKey )
      case pcount() = 3  ;  eval( bBlock, p1, p2, nKey )
      otherwise          ;  eval( bBlock, p1, p2, p3, nKey )
      end case

      return .t.

    endif

  endif

return .f.
