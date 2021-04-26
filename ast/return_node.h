#ifndef __FIR_AST_RETURN_NODE_H__
#define __FIR_AST_RETURN_NODE_H__

#include <cdk/ast/basic_node.h>

namespace fir {

  /**
   * Class for describing return nodes.
   */
  class return_node: public cdk::basic_node {
    int _level;

  public:
    inline return_node(int lineno, int level = 1) :
        cdk::basic_node(lineno), _level(level) {
    }

  public:
    inline int level() {
      return _level;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_return_node(this, level);
    }

  };

} // fir

#endif