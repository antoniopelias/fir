%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include "ast/all.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
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
};

%token <i> tINTEGER
%token <s> tIDENTIFIER tSTRING
%token <d> tREAL
%token tWHILE tIF tREAD tBEGIN tEND tLEAVE tRESTART tWRITE tWRITELN tDO
%token tTYPE_INT tTYPE_STRING tTYPE_REAL tTYPE_VOID tNULL
%token tAND tOR tSIZEOF tRETURN tARROW tEPILOGUE tTHEN

%nonassoc tIFX
%nonassoc tELSE tFINALLY

%right '='
%left tGE tLE tEQ tNE '>' '<'
%left '+' '-'
%left '*' '/' '%'
%nonassoc tUNARY

%type <node> stmt program
%type <sequence> list exprs
%type <expression> expr
%type <lvalue> lval
%type <s> str

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

program	: tBEGIN list tEND { compiler->ast(new cdk::sequence_node(LINE, $2)); }
	      ;

list : stmt	     { $$ = new cdk::sequence_node(LINE, $1); }
	   | list stmt { $$ = new cdk::sequence_node(LINE, $2, $1); }
	   ;

stmt : expr ';'                         { $$ = new fir::evaluation_node(LINE, $1); }
     | tWRITE exprs ';'                 {$$ = new fir::write_node(LINE, $2);}
     | tWRITELN exprs ';'               {$$ = new fir::writeln_node(LINE, $2);}
 //  | tLEAVE                         {$$ = new fir::leave_node(LINE);}
//   | tLEAVE integerliteral          {$$ = new fir::leave_node(LINE, $2);}
 //  | tRESTART                         {$$ = new fir::restart_node(LINE);}
//   | tRESTART integerliteral          {$$ = new fir::restart_node(LINE, $2);}
//   | tPRINT expr ';'                  { $$ = new fir::print_node(LINE, $2); }
     | tREAD lval ';'                   { $$ = new fir::read_node(LINE, $2); }
     | tWHILE  expr tDO stmt         { $$ = new fir::while_node(LINE, $2, $4); }
     | tWHILE  expr tDO stmt tFINALLY stmt       { $$ = new fir::while_finally_node(LINE, $2, $4, $6); }
//   | tIF '(' expr ')' stmt %prec tIFX { $$ = new fir::if_node(LINE, $3, $5); }
     | tIF '(' expr ')' stmt tELSE stmt { $$ = new fir::if_else_node(LINE, $3, $5, $7); }
     | '{' list '}'                     { $$ = $2; }
     ;
// integerliteral:;
exprs: expr                        { $$  = new cdk::expression_node(LINE, $1);}
     | exprs expr                  {$$ = new cdk::sequence_node(LINE, $2, $1);}

str  : tSTRING                     { $$ = new cdk::string_node(LINE, $1); }
     | str tSTRING                 { $$ = new cdk::sequence_node(LINE, $2, $1); }

expr : tINTEGER                    { $$ = new cdk::integer_node(LINE, $1); }
     | tREAL                      { $$ = new cdk::double_node(LINE, $1); }
	| tSTRING                     { $$ = new cdk::string_node(LINE, $1); }
     | '-' expr %prec tUNARY       { $$ = new cdk::neg_node(LINE, $2); }
     | expr '+' expr	         { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr	         { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr	         { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr	         { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr	         { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '<' expr	         { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr '>' expr	         { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr	         { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tLE expr               { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tNE expr	         { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tEQ expr	         { $$ = new cdk::eq_node(LINE, $1, $3); }
     | '(' expr ')'                { $$ = $2; }
     | '[' expr ']'                { $$ = new fir::stack_alloc_node(LINE, $2); }
     | lval                        { $$ = new cdk::rvalue_node(LINE, $1); }  //FIXME
     | lval '=' expr               { $$ = new cdk::assignment_node(LINE, $1, $3); }
     ;

lval : tIDENTIFIER             { $$ = new cdk::variable_node(LINE, $1); }
     | tIDENTIFIER '[' expr ']'         { $$ = new fir::index_node(LINE, $1, $3); }
     ;

%%
