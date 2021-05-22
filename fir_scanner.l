%option c++ prefix="fir_scanner_" outfile="fir_scanner.cpp"
%option stack noyywrap yylineno 8bit
%{ 
// make relevant includes before including the parser's tab file
#include <stdlib.h>
#include <string>
#include <cmath>
#include <climits>
#include <sstream>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "fir_parser.tab.h"

// don't change this
#define yyerror LexerError

float fpvalue;
long  intvalue;

// output stream for building string literals
static std::ostringstream strlit; 

%}

IDENTIFIER   [A-Za-z]([A-Za-z_]|[[:digit:]])*

%x X_STRING X_COMMENT X_NULLSTRING X_OCTAL_INT X_TILDE
%%

  /* ====================================================================== */
  /* ====[                      3.2 - COMMENTS                        ]==== */
  /* ====================================================================== */

"!!".*                  ; /* ignore comments */
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
[<>]                    return *yytext;

"sizeof"                return tSIZEOF;


  /* ====================================================================== */
  /* ====[              5   - Funcoes            ]==== */
  /* ====================================================================== */


"return"                return tRETURN;
"->"                    return tARROW;

"write"                 return tWRITE;
"writeln"               return tWRITELN;

  /* ====================================================================== */
  /* ====[              6   - Instrucoes            ]==== */
  /* ====================================================================== */

"while"                 return tWHILE; 
"do"                    return tDO;
"finally"               return tFINALLY;

"if"                    return tIF; 
"then"                  return tTHEN;
"else"                  return tELSE; 

"leave"                 return tLEAVE;
"restart"               return tRESTART;


  /* ====================================================================== */
  /* ====[             3.8.2 - Reais em vírgula flutuante                 ]==== */
  /* ====================================================================== */

([0-9]*\.[0-9]+|[0-9]+\.[0-9]*)([eE][-+]?[0-9]+)? { 
                                                    try { yylval.d = std::stod(yytext, nullptr); 
                                                          return tREAL; 
                                                        } catch (std::out_of_range& e) { yyerror("Real out of range."); }
                                                  }
[0-9]+([eE][-+]?[0-9]+) { 
                                                    try { yylval.d = std::stod(yytext, nullptr); 
                                                          return tREAL; 
                                                        } catch (std::out_of_range& e) { yyerror("Real out of range."); }
                                                  }
  /* ====================================================================== */
  /* ====[                   3.8.1 - inteiros                         ]==== */
  /* ====================================================================== */

[1-9][0-9]*                 {try { 
                                  yylval.i = std::stoi(yytext, nullptr, 10);
                                  return tINTEGER;
                                } catch (std::out_of_range& e) { yyerror("Overflow error."); }
                              }

[0][0-7]*                   {try { 
                                  if(std::stoi(yytext, nullptr, 8) == 0 && std::strlen(yytext) > 1){
                                    yyerror("Error with number 0");
                                  }
                                  yylval.i = std::stoi(yytext, nullptr, 8);
                                  return tINTEGER;
                                } catch (std::out_of_range& e) { yyerror("Overflow error."); }
                              }

  /* ====================================================================== */
  /* ====[                   3.5 - Operadores de expressões           ]==== */
  /* ====[              3.6   - Delimitadores e separadores           ]==== */
  /* ====================================================================== */

[()\[\]+-?*%~=,;{}@]          return *yytext;

">="                   return tGE; 
"<="                   return tLE; 
"=="                   return tEQ; 
"!="                   return tNE;
"&&"                    return tAND;
"||"                    return tOR;

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
<X_STRING>\n                  yyerror("Error in string.");
<X_STRING>.                    strlit << *yytext;

<X_TILDE>n                 strlit << '\n'; yy_pop_state();
<X_TILDE>r                 strlit << '\r'; yy_pop_state();
<X_TILDE>t                 strlit << '\t'; yy_pop_state();
<X_TILDE>~                strlit << '~'; yy_pop_state();
<X_TILDE>\'               strlit << '\''; yy_pop_state();
<X_TILDE>[[:xdigit:]]{1,2} {
                                strlit << (char)(unsigned char)strtoul(yytext, nullptr, 16);
                                yy_pop_state();
                              }

<X_TILDE>.                 strlit << *yytext; yy_pop_state();


  /* ====================================================================== */
  /* ====[                3.7 - Identificadores (nomes)               ]==== */
  /* ====================================================================== */

{IDENTIFIER}  yylval.s = new std::string(yytext); return tIDENTIFIER;


[ \t\n\r]+               ; /* ignore whitespace */

.                      yyerror("Unknown character");

%%
