/*
 * $Id$
 */

/*
 * Harbour Project source code:
 * Header file for compiler error codes
 *
 * Copyright 1999 Ryszard Glab <rglab@imid.med.pl>
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

#ifndef HB_ERRORS_H_
#define HB_ERRORS_H_

/*
 * Errors generated by Harbour compiler
 */
#define ERR_OUTSIDE                1
#define ERR_FUNC_DUPL              2
#define ERR_VAR_DUPL               3
#define ERR_FOLLOWS_EXEC           4
#define ERR_OUTER_VAR              5
#define ERR_NUMERIC_FORMAT         6
#define ERR_STRING_TERMINATOR      7
#define ERR_FUNC_RESERVED          8
#define ERR_ILLEGAL_INIT           9
#define ERR_ENDIF                  10
#define ERR_ENDDO                  11
#define ERR_ENDCASE                12
#define ERR_NEXTFOR                13
#define ERR_UNMATCHED_ELSE         14
#define ERR_UNMATCHED_ELSEIF       15
#define ERR_SYNTAX                 16
#define ERR_UNCLOSED_STRU          17
#define ERR_UNMATCHED_EXIT         18
#define ERR_SYNTAX2                19
#define ERR_INCOMPLETE_STMT        20
#define ERR_CHECKING_ARGS          21
#define ERR_INVALID_LVALUE         22
#define ERR_INVALID_REFER          23
#define ERR_PARAMETERS_NOT_ALLOWED 24
#define ERR_EXIT_IN_SEQUENCE       25
#define ERR_UNTERM_ARRAY_INDEX     26
#define ERR_MEMALLOC               27
#define ERR_MEMREALLOC             28
#define ERR_MEMFREE                29
#define ERR_YACC                   30
#define ERR_JUMP_TOO_LONG          31
#define ERR_CREATE_OUTPUT          32
#define ERR_CREATE_PPO             33
#define ERR_BADOPTION              34
#define ERR_BADPARAM               35
#define ERR_BADFILENAME            36
#define ERR_MAYHEM_IN_CASE         37
#define ERR_INVALID_TYPE           38
#define ERR_INVALID_ALIAS          39
#define ERR_INVALID_INDEX          40
#define ERR_INVALID_BOUND          41

#define WARN_AMBIGUOUS_VAR         1
#define WARN_MEMVAR_ASSUMED        2
#define WARN_VAR_NOT_USED          3
#define WARN_BLOCKVAR_NOT_USED     4
#define WARN_NO_RETURN_VALUE       5
#define WARN_PROC_RETURN_VALUE     6
#define WARN_FUN_WITH_NO_RETURN    7
#define WARN_ASSIGN_TYPE           8
#define WARN_LOGICAL_TYPE          9
#define WARN_NUMERIC_TYPE          10
#define WARN_OPERANDS_INCOMPATBLE  11
#define WARN_ASSIGN_SUSPECT        12
#define WARN_OPERAND_SUSPECT       13
#define WARN_LOGICAL_SUSPECT       14
#define WARN_NUMERIC_SUSPECT       15
#define WARN_MEANINGLESS           16

/*
 * Errors generated by Harbour preprocessor
 */
#define ERR_CANNOT_OPEN            1
#define ERR_DIRECTIVE_ELSE         2
#define ERR_DIRECTIVE_ENDIF        3
#define ERR_WRONG_NAME             4
#define ERR_DEFINE_ABSENT          5
#define ERR_COMMAND_DEFINITION     6
#define ERR_PATTERN_DEFINITION     7
#define ERR_RECURSE                8
#define ERR_WRONG_DIRECTIVE        9
#define ERR_EXPLICIT               10
#define ERR_PPMEMALLOC             11
#define ERR_PPMEMREALLOC           12
#define ERR_PPMEMFREE              13

#define WARN_NONDIRECTIVE          1

extern void hb_compGenError( char * _szErrors[], char, int, char *, char * ); /* generic parsing error management function */
extern void hb_compGenWarning( char * _szWarnings[], char, int, char *, char * ); /* generic parsing warning management function */

#endif /* HB_ERRORS_H_ */
