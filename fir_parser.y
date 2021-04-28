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

%type <node> instruction file conditional iterative argdec
%type <type> data_type void_type
%type <sequence> list expressions argdecs declarations 
%type <expression> expr funcCall
%type <node> funcDeclaration funcDefinition declaration
%type <lvalue> lval
%type <s> string



%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%


file         : /* empty */                                       { compiler->ast($$ = new cdk::sequence_node(LINE)); }
             | declarations                                      { compiler->ast($$ = $1); }
             | list                                              {} // REMOVE THIS
             ;

declarations :              declaration                          { $$ = new cdk::sequence_node(LINE, $1);     }
             | declarations declaration                          { $$ = new cdk::sequence_node(LINE, $2, $1); }
             ;

declaration  : // vardec ';'                                     { $$ = $1; }
              funcDeclaration                                    { $$ = $1; }
             | funcDefinition                                    { $$ = $1; }
             ;


list : instruction	     { $$ = new cdk::sequence_node(LINE, $1); }
	| list instruction { $$ = new cdk::sequence_node(LINE, $2, $1); }
	;

instruction : expr ';'                                    { $$ = new fir::evaluation_node(LINE, $1); }
     | tWRITE expressions ';'                 {$$ = new fir::write_node(LINE, $2);}
     | tWRITELN expressions ';'               {$$ = new fir::writeln_node(LINE, $2);}
     | tLEAVE                                     {$$ = new fir::leave_node(LINE);}
     | tLEAVE tINTEGER                            {$$ = new fir::leave_node(LINE, new cdk::integer_node(LINE, $2));}
     | tRESTART                                   {$$ = new fir::restart_node(LINE);}
     | tRESTART tINTEGER                          {$$ = new fir::restart_node(LINE, new cdk::integer_node(LINE, $2));}
     | tRETURN                                    {$$ = new fir::return_node(LINE); }
     | conditional                                { $$ = $1; }
     | iterative                                  { $$ = $1; }
     | '{' list '}'                               { $$ = $2; }
     ;


conditional: tIF expr tTHEN instruction %prec tIFX             { $$ = new fir::if_node(LINE, $2, $4); }
           | tIF expr tTHEN instruction tELSE instruction             { $$ = new fir::if_else_node(LINE, $2, $4, $6); }

iterative: tWHILE expr tDO instruction                      { $$ = new fir::while_node(LINE, $2, $4); }
         | tWHILE expr tDO instruction tFINALLY instruction       { $$ = new fir::while_finally_node(LINE, $2, $4, $6); }



expressions: expr                        { $$  = new cdk::sequence_node(LINE, $1);}
           | expressions ',' expr                  {$$ = new cdk::sequence_node(LINE, $3, $1);}
           ;

string          : tSTRING                       { $$ = $1; }
                | string tSTRING                { $$ = $1; $$->append(*$2); delete $2; }

expr : tINTEGER                    { $$ = new cdk::integer_node(LINE, $1); }
     | tREAL                      { $$ = new cdk::double_node(LINE, $1); }
	| string                     { $$ = new cdk::string_node(LINE, $1); }
     | tNULL                       { $$ = new fir::null_node(LINE); }
     | '-' expr %prec tUNARY       { $$ = new cdk::neg_node(LINE, $2); }
     | '+' expr %prec tUNARY       { $$ = new fir::identity_node(LINE, $2); }
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

data_type : tTYPE_INT               {$$ = new cdk::primitive_type(4, cdk::TYPE_INT);}
          | tTYPE_REAL              {$$ = new cdk::primitive_type(8, cdk::TYPE_DOUBLE);}
          | tTYPE_STRING            {$$ = new cdk::primitive_type(4, cdk::TYPE_STRING);}
          | '<' data_type '>'  {$$ = new cdk::reference_type(4, std::shared_ptr<cdk::basic_type>($2));}
          ;

argdecs  : /* empty */         { $$ = new cdk::sequence_node(LINE);  }
         |             argdec  { $$ = new cdk::sequence_node(LINE, $1);     }
         | argdecs ',' argdec  { $$ = new cdk::sequence_node(LINE, $3, $1); }
         ;

argdec   : data_type tIDENTIFIER { $$ = new fir::variable_declaration_node(LINE, $1, 0, *$2, nullptr); }
         ;

void_type   : tTYPE_VOID { $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID);   }
            ;

funcDeclaration:     data_type tIDENTIFIER '(' argdecs ')'                {$$ = new fir::function_declaration_node(LINE, $1, 0, *$2, $4); } // tPRIVATE = 0
               | '?' data_type tIDENTIFIER '(' argdecs ')'                 { $$ = new fir::function_declaration_node(LINE, $2, '*',  *$3, $5); }
               | '*' data_type tIDENTIFIER '(' argdecs ')'                 { $$ = new fir::function_declaration_node(LINE,  $2, '*', *$3, $5); }
               |     void_type tIDENTIFIER '(' argdecs ')'                {$$ = new fir::function_declaration_node(LINE, $1, 0, *$2, $4); } // tPRIVATE = 0
               | '?' void_type tIDENTIFIER'(' argdecs ')'                  { $$ = new fir::function_declaration_node(LINE, $2, '*', *$3, $5); }
               | '*' void_type tIDENTIFIER '(' argdecs ')'                      { $$ = new fir::function_declaration_node(LINE, $2, '*', *$3, $5); }
               ;

funcDefinition:;


/*
variables : variables ',' localVariable {$$ = new cdk::sequence_node(LINE, $3, $1); }
          | localVariable               {$$ = new cdk::sequence_node(LINE, $1); }
          ;

variable : accessType type identifier  {$$ = new fir::variable_declaration_node(LINE,$1, $2, *$3, nullptr); delete $3; }  
         | accessType type identifier  '=' expr { $$ = new og::variable_declaration_node(LINE,$1, $2, *$3, $5); delete $3;}
         | accessType auto identifiers '=' exprs { $$ = new og::variable_declaration_node(LINE,$1, $2, *$3, new og::tuple_node(LINE,$5)); delete $3; }
         | tREQUIRE type identifier  {$$ = new og::variable_declaration_node(LINE,$1, $2, *$3, nullptr); delete $3; }  
         | tREQUIRE type identifier  '=' expr { $$ = new og::variable_declaration_node(LINE,$1, $2, *$3, $5); delete $3;}
         ;

auto : tAUTO  { $$ = new cdk::primitive_type(4, cdk::typename_type::TYPE_STRUCT); }
     ;

voidType : tPROCEDURE { $$ = new cdk::primitive_type(0, cdk::typename_type::TYPE_VOID); }
         ;
         
accessType : tPUBLIC {$$ = tPUBLIC;}
           |         {$$ = '\0';}
           ;

functionDef : accessType type identifier '(' functionArgs')' block       {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, $7); delete $3; }
            | accessType auto identifier '(' functionArgs')' block       {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, $7); delete $3; }
            | accessType voidType identifier '(' functionArgs ')' block   {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, $7); delete $3; }
            ;

localVariable  : type identifier  { $$ = new og::variable_declaration_node(LINE,'\0', $1, *$2, nullptr); delete $2; }  
               | type identifier  '=' expr { $$ = new og::variable_declaration_node(LINE,'\0', $1, *$2, $4); delete $2;}
               | auto identifiers '=' exprs { $$ = new og::variable_declaration_node(LINE,'\0', $1, *$2, new og::tuple_node(LINE,$4)); delete $2; }
               ;


blockDecs : blockDecs localVariable ';'      { $$ = new cdk::sequence_node(LINE, $2, $1); }
          | localVariable  ';'               { $$ = new cdk::sequence_node(LINE, $1); }
          ;

block : '{' blockDecs blockInstructions '}'  {$$ = new og::block_node(LINE, $2, $3);}
      | '{' blockInstructions '}'            {$$ = new og::block_node(LINE, nullptr, $2);}
      ;

blockInstructions : instructions       {$$ = $1; }
                  |                    {$$ = nullptr; }
                  ;

instructions : instructions instruction   { $$ = new cdk::sequence_node(LINE, $2, $1); }
             | instruction                { $$ = new cdk::sequence_node(LINE, $1); }
             ;

*/




lval : tIDENTIFIER                 { $$ = new cdk::variable_node(LINE, *$1); delete $1;}
     | lval '[' expr ']'     { $$ = new fir::index_node(LINE, new cdk::rvalue_node(LINE, $1), $3); }  
     ;

%%
