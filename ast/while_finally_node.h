#ifndef __FIR_AST_WHILE_FINALLY_NODE_H__
#define __FIR_AST_WHILE_FINALLY_NODE_H__

#include <cdk/ast/expression_node.h>

namespace fir {

  /**
   * Class for describing while-cycle nodes.
   */
  class while_finally_node: public cdk::basic_node {
    cdk::expression_node *_condition;
    cdk::basic_node *_theninstr, *_finallyinstr;

  public:
    inline while_finally_node(int lineno, cdk::expression_node *condition, cdk::basic_node *theninstr,
    cdk::basic_node *finallyinstr) :
        basic_node(lineno), _condition(condition), _theninstr(theninstr), _finallyinstr(finallyinstr) {
    }

  public:
    inline cdk::expression_node *condition() {
      return _condition;
    }
    inline cdk::basic_node *theninstr() {
      return _theninstr;
    }
    inline cdk::basic_node *finallyinstr() {
      return _finallyinstr;
    }
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_while_finally_node(this, level);
    }

  };

} // fir

#endif
