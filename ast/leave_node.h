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

  public: // integer_node ??
    inline leave_node(int lineno, cdk::integer_node *literal = nullptr) :
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