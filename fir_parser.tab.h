/* A Bison parser, made by GNU Bison 3.7.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_FIR_PARSER_TAB_H_INCLUDED
# define YY_YY_FIR_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    tINTEGER = 258,                /* tINTEGER  */
    tIDENTIFIER = 259,             /* tIDENTIFIER  */
    tSTRING = 260,                 /* tSTRING  */
    tREAL = 261,                   /* tREAL  */
    tWHILE = 262,                  /* tWHILE  */
    tIF = 263,                     /* tIF  */
    tREAD = 264,                   /* tREAD  */
    tBEGIN = 265,                  /* tBEGIN  */
    tEND = 266,                    /* tEND  */
    tLEAVE = 267,                  /* tLEAVE  */
    tRESTART = 268,                /* tRESTART  */
    tWRITE = 269,                  /* tWRITE  */
    tWRITELN = 270,                /* tWRITELN  */
    tDO = 271,                     /* tDO  */
    tTYPE_INT = 272,               /* tTYPE_INT  */
    tTYPE_STRING = 273,            /* tTYPE_STRING  */
    tTYPE_REAL = 274,              /* tTYPE_REAL  */
    tTYPE_VOID = 275,              /* tTYPE_VOID  */
    tNULL = 276,                   /* tNULL  */
    tAND = 277,                    /* tAND  */
    tOR = 278,                     /* tOR  */
    tSIZEOF = 279,                 /* tSIZEOF  */
    tRETURN = 280,                 /* tRETURN  */
    tARROW = 281,                  /* tARROW  */
    tEPILOGUE = 282,               /* tEPILOGUE  */
    tTHEN = 283,                   /* tTHEN  */
    tIFX = 284,                    /* tIFX  */
    tELSE = 285,                   /* tELSE  */
    tFINALLY = 286,                /* tFINALLY  */
    tGE = 287,                     /* tGE  */
    tLE = 288,                     /* tLE  */
    tEQ = 289,                     /* tEQ  */
    tNE = 290,                     /* tNE  */
    tUNARY = 291                   /* tUNARY  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 17 "fir_parser.y"

  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!

  int                   i;	/* integer value */
  double                d; 
  std::string          *s;	/* symbol name or string literal */
  cdk::basic_node      *node;	/* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;

#line 119 "fir_parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (std::shared_ptr<cdk::compiler> compiler);

#endif /* !YY_YY_FIR_PARSER_TAB_H_INCLUDED  */
