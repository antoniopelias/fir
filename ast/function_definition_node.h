#ifndef __FIR_AST_FUNCTION_DEFINITION_H__
#define __FIR_AST_FUNCTION_DEFINITION_H__


#include <string>
#include <cdk/ast/typed_node.h>
#include <cdk/ast/sequence_node.h>
#include <ast/body_node.h>

namespace fir {

  class function_definition_node: public cdk::typed_node {
    std::string _identifier;
    int _qualifier;
    cdk::sequence_node *_arguments;
    fir::body_node *_body;

  public:
    function_definition_node(int lineno, std::shared_ptr<cdk::basic_type> funType, int qualifier, const std::string &identifier,
                              cdk::sequence_node *arguments, fir::body_node *body) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _arguments(arguments),
        _body(body){
      type(funType);
    }

  public:
    int qualifier() {
      return _qualifier;
    }
    const std::string& identifier() const {
      return _identifier;
    }
    cdk::typed_node* argument(size_t ax) {
      return dynamic_cast<cdk::typed_node*>(_arguments->node(ax));
    }
    cdk::sequence_node* arguments() {
      return _arguments;
    }
    fir::body_node* body(){
      return _body;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_definition_node(this, level);
    }

  };

} // fir

#endif