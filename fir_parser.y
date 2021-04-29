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
  //cdk::typed_node      *typed;
  fir::block_node       *block;
  fir::body_node         *body;
  fir::prologue_node     *prologue;
};

%token <i> tINTEGER
%token <s> tIDENTIFIER tSTRING
%token <d> tREAL
%token tWHILE tIF tREAD tBEGIN tEND tLEAVE tRESTART tWRITE tWRITELN tDO
%token tTYPE_INT tTYPE_STRING tTYPE_REAL tTYPE_VOID tNULL
%token tPRIVATE tPUBLIC 
%token tAND tOR tSIZEOF tRETURN tARROW tEPILOGUE tTHEN

%nonassoc tIFX tWHILEX
%nonassoc tELSE tFINALLY

%right '='
%left tOR
%left tAND
%nonassoc '~'
%left tEQ tNE
%left tGE tLE '>' '<'
%left '+' '-'
%left '*' '/' '%'
%nonassoc tUNARY
%nonassoc '(' ')' '[' ']'

%type <node> instruction file conditional iterative argdec
%type <type> data_type void_type
%type <sequence> instructions expressions argdecs vardecs declarations opt_vardecs opt_instructions 
%type <expression> expr funcCall opt_initializer def_retval
%type <node> funcDeclaration funcDefinition declaration vardec
%type <block> block epilogue
%type <prologue> prologue
%type <body> body
%type <lvalue> lval
%type <s> string



%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%


file         : /* empty */                                       { compiler->ast($$ = new cdk::sequence_node(LINE)); }
             | declarations                                      { compiler->ast($$ = $1); }
             ;

declarations :              declaration                          { $$ = new cdk::sequence_node(LINE, $1);     }
             | declarations declaration                          { $$ = new cdk::sequence_node(LINE, $2, $1); }
             ;

declaration  : vardec ';'                                        { $$ = $1; }
             | funcDeclaration                                   { $$ = $1; }
             | funcDefinition                                    { $$ = $1; }
             ;

vardec       :   data_type '?' tIDENTIFIER                          { $$ = new fir::variable_declaration_node(LINE, $1, tPUBLIC, *$3, nullptr); }
             |   data_type  '*' tIDENTIFIER opt_initializer          { $$ = new fir::variable_declaration_node(LINE, $1, tPUBLIC, *$3, $4); }
             |   data_type      tIDENTIFIER opt_initializer          { $$ = new fir::variable_declaration_node(LINE, $1, tPRIVATE, *$2, $3); }
             ;

opt_initializer  : /* empty */         { $$ = nullptr; }
                 | '=' expr      { $$ = $2; }
                 ;

instructions : instruction	                                             { $$ = new cdk::sequence_node(LINE, $1); }
	| instructions instruction                                          { $$ = new cdk::sequence_node(LINE, $2, $1); }
	;

instruction : expr ';'                                           { $$ = new fir::evaluation_node(LINE, $1); }
     | tWRITE expressions ';'                                    { $$ = new fir::write_node(LINE, $2);}
     | tWRITELN expressions ';'                                  { $$ = new fir::writeln_node(LINE, $2);}
     | tLEAVE                                                    { $$ = new fir::leave_node(LINE);}
     | tLEAVE tINTEGER                                           { $$ = new fir::leave_node(LINE, new cdk::integer_node(LINE, $2));}
     | tRESTART                                                  { $$ = new fir::restart_node(LINE);}
     | tRESTART tINTEGER                                         { $$ = new fir::restart_node(LINE, new cdk::integer_node(LINE, $2));}
     | tRETURN                                                   { $$ = new fir::return_node(LINE); }
     | conditional                                               { $$ = $1; }
     | iterative                                                 { $$ = $1; }
     | '{' instructions '}'                                              { $$ = $2; }
     ;


conditional: tIF expr tTHEN instruction %prec tIFX               { $$ = new fir::if_node(LINE, $2, $4); }
           | tIF expr tTHEN instruction tELSE instruction        { $$ = new fir::if_else_node(LINE, $2, $4, $6); }

iterative: tWHILE expr tDO instruction  %prec tWHILEX                       { $$ = new fir::while_node(LINE, $2, $4); }
         | tWHILE expr tDO instruction tFINALLY instruction      { $$ = new fir::while_finally_node(LINE, $2, $4, $6); }

expressions: expr                                                { $$  = new cdk::sequence_node(LINE, $1);}
           | expressions ',' expr                                { $$ = new cdk::sequence_node(LINE, $3, $1);}
           ;

string          : tSTRING                                        { $$ = $1; }
                | string tSTRING                                 { $$ = $1; $$->append(*$2); delete $2; }

expr : tINTEGER                                                  { $$ = new cdk::integer_node(LINE, $1); }
     | tREAL                                                     { $$ = new cdk::double_node(LINE, $1); }
	| string                                                    { $$ = new cdk::string_node(LINE, $1); }
     | tNULL                                                     { $$ = new fir::null_node(LINE); }
     | '-' expr %prec tUNARY                                     { $$ = new cdk::neg_node(LINE, $2); }
     | '+' expr %prec tUNARY                                     { $$ = new fir::identity_node(LINE, $2); }
     | expr '+' expr	                                        { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr	                                        { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr	                                        { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr	                                        { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr	                                        { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '<' expr	                                        { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr '>' expr	                                        { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr	                                        { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tLE expr                                             { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tNE expr	                                        { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tEQ expr	                                        { $$ = new cdk::eq_node(LINE, $1, $3); }
     | expr tOR expr                    { $$ = new cdk::or_node(LINE, $1, $3); }
     | expr tAND expr                   { $$ = new cdk::and_node(LINE, $1, $3); }
     | '~' expr                         { $$ = new cdk::not_node(LINE, $2); }
     | '(' expr ')'                { $$ = $2; }
     | '[' expr ']'                { $$ = new fir::stack_alloc_node(LINE, $2); }
     | lval                        { $$ = new cdk::rvalue_node(LINE, $1); }  //FIXME
     | lval '?'                    { $$ = new fir::address_of_node(LINE, $1);}
     | lval '=' expr               { $$ = new cdk::assignment_node(LINE, $1, $3); }
     | tSIZEOF '(' expr ')'            { $$ = new fir::sizeof_node(LINE, $3); }
     | funcCall                    { $$ = $1;}
     | '@'                           { $$ = new fir::read_node(LINE); }
     ;


funcCall: tIDENTIFIER'(' expressions ')'   { $$ = new fir::function_call_node(LINE, *$1, $3); delete $1; }
        | tIDENTIFIER'(' ')'               { $$ = new fir::function_call_node(LINE, *$1, nullptr); delete $1; }
        ;

data_type : tTYPE_INT               {$$ = cdk::primitive_type::create(4, cdk::TYPE_INT);}
          | tTYPE_REAL              {$$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE);}
          | tTYPE_STRING            {$$ = cdk::primitive_type::create(4, cdk::TYPE_STRING);}
          | '<' data_type '>'  {$$ = cdk::reference_type::create(4, $2);}
          ;

argdecs  : /* empty */         { $$ = new cdk::sequence_node(LINE);  }
         |             argdec  { $$ = new cdk::sequence_node(LINE, $1);     }
         | argdecs ',' argdec  { $$ = new cdk::sequence_node(LINE, $3, $1); }
         ;

argdec   : data_type tIDENTIFIER { $$ = new fir::variable_declaration_node(LINE, $1, tPRIVATE, *$2, nullptr); }
         ;

void_type   : tTYPE_VOID { $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID);   }
            ;

funcDeclaration: data_type     tIDENTIFIER '(' argdecs ')'                {$$ = new fir::function_declaration_node(LINE, $1, tPRIVATE, *$2, $4); }
               | data_type '?' tIDENTIFIER '(' argdecs ')'                 { $$ = new fir::function_declaration_node(LINE, $1, tPUBLIC,  *$3, $5); }
               | data_type '*' tIDENTIFIER '(' argdecs ')'                 { $$ = new fir::function_declaration_node(LINE,  $1, tPUBLIC, *$3, $5); }
               | void_type     tIDENTIFIER '(' argdecs ')'                {$$ = new fir::function_declaration_node(LINE, $1, tPRIVATE, *$2, $4); }
               | void_type '?' tIDENTIFIER '(' argdecs ')'                  { $$ = new fir::function_declaration_node(LINE, $1, tPUBLIC, *$3, $5); }
               | void_type '*' tIDENTIFIER '(' argdecs ')'                      { $$ = new fir::function_declaration_node(LINE, $1, tPUBLIC, *$3, $5); }
               ;

funcDefinition: data_type     tIDENTIFIER '(' argdecs ')' body { $$ = new fir::function_definition_node(LINE, $1, tPRIVATE, *$2, $4, $6); }
              | data_type '*' tIDENTIFIER '(' argdecs ')' body { $$ = new fir::function_definition_node(LINE, $1, tPUBLIC, *$3, $5, $7); }
              | data_type     tIDENTIFIER '(' argdecs ')' def_retval body { $$ = new fir::function_definition_node(LINE, $1, tPRIVATE, *$2, $4, $6, $7); }
              | data_type '*' tIDENTIFIER '(' argdecs ')' def_retval body { $$ = new fir::function_definition_node(LINE, $1, tPUBLIC, *$3, $5, $7, $8); }
              | void_type     tIDENTIFIER '(' argdecs ')' body { $$ = new fir::function_definition_node(LINE, $1, tPRIVATE, *$2, $4, $6); }
              | void_type '*' tIDENTIFIER '(' argdecs ')' body { $$ = new fir::function_definition_node(LINE, $1, tPUBLIC, *$3, $5, $7); }
              ;

def_retval: tARROW tINTEGER                                      { $$ = new cdk::integer_node(LINE, $2); }
          | tARROW tSTRING                                        { $$ = new cdk::string_node(LINE, $2); }
          | tARROW tREAL                                           { $$ = new cdk::double_node(LINE, $2); }
          ;

body: prologue block epilogue                                    { $$ = new fir::body_node(LINE, $1, $2, $3); }
    | prologue block                                             { $$ = new fir::body_node(LINE, $1, $2, nullptr); }
    | prologue       epilogue                                    { $$ = new fir::body_node(LINE, $1, nullptr, $2); }
    |          block epilogue                                    { $$ = new fir::body_node(LINE, nullptr, $1, $2); }
    | prologue                                                    { $$ = new fir::body_node(LINE, $1, nullptr, nullptr); }
    |          block                                                    { $$ = new fir::body_node(LINE, nullptr, $1, nullptr); }
    |                epilogue                                                    { $$ = new fir::body_node(LINE, nullptr, nullptr, $1); }
     ;
     
prologue: '@' '{' opt_vardecs opt_instructions'}'                   { $$ = new fir::prologue_node(LINE, $3, $4); }
     ;

block: '{' opt_vardecs opt_instructions'}'                          { $$ = new fir::block_node(LINE, $2, $3); }
     ;

opt_vardecs: /* empty */ { $$ = new cdk::sequence_node(LINE); }
                | vardecs { $$ = $1; }
                ;

vardecs      : vardec ';'          { $$ = new cdk::sequence_node(LINE, $1);     }
             | vardecs vardec ';' { $$ = new cdk::sequence_node(LINE, $2, $1); }
             ;
             
opt_instructions: /* empty */  { $$ = new cdk::sequence_node(LINE); }
                | instructions { $$ = $1; }
                ;

epilogue: '>' '>' block       { $$ = $3; }
        ;

lval : tIDENTIFIER                 { $$ = new cdk::variable_node(LINE, *$1); delete $1;}
     | lval '[' expr ']'     { $$ = new fir::index_node(LINE, new cdk::rvalue_node(LINE, $1), $3); }  
     ;

%%
