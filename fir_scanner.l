%option c++ prefix="fir_scanner_" outfile="fir_scanner.cpp"
%option stack noyywrap yylineno 8bit
%{ 
// make relevant includes before including the parser's tab file
#include <stdlib.h>
#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "fir_parser.tab.h"

// don't change this
#define yyerror LexerError

float fpvalue;

// output stream for building string literals
static std::ostringstream strlit; 

%}

IDENTIFIER   [A-Za-z]([A-Za-z_]|[[:digit:]])*

%x X_STRING X_COMMENT X_NULLSTRING X_OCTAL_INT X_TILDE
%%

  /* ====================================================================== */
  /* ====[                      3.2 - COMMENTS                        ]==== */
  /* ====================================================================== */

"!!".*                 ; /* ignore comments */
"(*"                    yy_push_state(X_COMMENT);  
<X_COMMENT>"*)"         yy_pop_state();
<X_COMMENT>.            ;
<X_COMMENT>\n           ;

  /* ====================================================================== */
  /* ====[                  3.4   - Tipos de dados                    ]==== */
  /* ====================================================================== */

"int"                   return tTYPE_INT;
"float"                 return tTYPE_REAL;
"string"                return tTYPE_STRING;
"void"                  return tTYPE_VOID;
"null"                  return tNULL;
[<>]                     return *yytext;

"sizeof"                return tSIZEOF;


  /* ====================================================================== */
  /* ====[                   3.5 - Operadores de expressões           ]==== */
  /* ====[              3.6   - Delimitadores e separadores           ]==== */
  /* ====================================================================== */

[()\[\]+-?*%~=,;{}@]         return *yytext;

">="                   return tGE; 
"<="                   return tLE; 
"=="                   return tEQ; 
"!="                   return tNE;
"&&"                    return tAND;
"||"                    return tOR;

  /* ====================================================================== */
  /* ====[              5   - Funcoes            ]==== */
  /* ====================================================================== */


"return"                return tRETURN;
"->"                    return tARROW;
">>"                    return tEPILOGUE;

"write"                 return tWRITE;
"writeln"               return tWRITELN;

  /* ====================================================================== */
  /* ====[              6   - Instrucoes            ]==== */
  /* ====================================================================== */

"while"                return tWHILE; 
"do"                    return tDO;
"finally"               return tFINALLY;

"if"                   return tIF; 
"then"                  return tTHEN;
"else"                 return tELSE; 

"leave"                 return tLEAVE;
"restart"               return tRESTART;


  /* ====================================================================== */
  /* ====[             3.8.2 - Reais em vírgula flutuante                 ]==== */
  /* ====================================================================== */

([0-9]*\.[0-9]+|[0-9]+\.[0-9]*)([eE][-+]?[0-9]+)? { 
                                                    fpvalue = strtod(yytext, nullptr);
                                                    if (errno == ERANGE && (fpvalue == HUGE_VALF || fpvalue == HUGE_VALL))
                                                      return yyerror("Overflow error.");
                                                    
                                                    yylval.d = fpvalue; 
                                                    return tREAL; 
                                                    }
  /* ====================================================================== */
  /* ====[                   3.8.1 - inteiros                         ]==== */
  /* ====================================================================== */

[1-9][0-9]*                 yylval.i = strtol(yytext, nullptr, 10); return tINTEGER;

[0][0-7]*                   yylval.i = strtol(yytext, NULL, 8); return tINTEGER;

  /* ====================================================================== */
  /* ====[              3.8.3 - Cadeias de caracteres                 ]==== */
  /* ====================================================================== */
\'                          yy_push_state(X_STRING);
<X_STRING>~                   yy_push_state(X_TILDE);
<X_STRING>\'                 {
                                 yylval.s = new std::string(strlit.str());
                                 strlit.str("");
                                 yy_pop_state();
                                 return tSTRING;
                               }
<X_STRING>.                    strlit << *yytext;

<X_TILDE>n                 strlit << '\n'; yy_pop_state();
<X_TILDE>r                 strlit << '\r'; yy_pop_state();
<X_TILDE>t                 strlit << '\t'; yy_pop_state();
<X_TILDE>~                strlit << '~'; yy_pop_state();
<X_TILDE>0                yy_pop_state(); yy_push_state(X_NULLSTRING);

<X_TILDE>.                 strlit << *yytext; yy_pop_state();

<X_NULLSTRING>\'           yy_pop_state(); yy_pop_state(); return tSTRING;
<X_NULLSTRING>~\'         ;
<X_NULLSTRING>.            ;


  /* ====================================================================== */
  /* ====[                3.7 - Identificadores (nomes)               ]==== */
  /* ====================================================================== */

{IDENTIFIER}  yylval.s = new std::string(yytext); return tIDENTIFIER;


[ \t\n\r]+               ; /* ignore whitespace */

.                      yyerror("Unknown character");

%%
