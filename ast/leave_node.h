#ifndef __FIR_AST_LEAVE_NODE_H__
#define __FIR_AST_LEAVE_NODE_H__

#include <cdk/ast/basic_node.h>
#include <cdk/ast/integer_node.h>

namespace fir {

  /**
   * Class for describing leave nodes.
   */
  class leave_node: public cdk::basic_node {
    cdk::integer_node *_literal;

  public: 
    inline leave_node(int lineno) :
        cdk::basic_node(lineno), _literal(new cdk::integer_node(lineno, 1)) {
    }

    inline leave_node(int lineno, cdk::integer_node *literal) :
        cdk::basic_node(lineno), _literal(literal) {
    }

  public:
    cdk::integer_node* literal() {
      return _literal;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_leave_node(this, level);
    }

  };

} // fir

#endif