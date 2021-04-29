#ifndef __FIR_AST_FUNCTION_DEFINITION_H__
#define __FIR_AST_FUNCTION_DEFINITION_H__


#include <string>
#include <cdk/ast/typed_node.h>
#include <cdk/ast/sequence_node.h>
#include <ast/body_node.h>
#include <cdk/ast/expression_node.h>

namespace fir {

  class function_definition_node: public cdk::typed_node {
    int _qualifier;
    std::string _identifier;
    cdk::sequence_node *_arguments;
    cdk::expression_node *_def_retval;
    fir::body_node *_body;

  public:
    function_definition_node(int lineno, std::shared_ptr<cdk::basic_type> funType, int qualifier, const std::string &identifier,
                              cdk::sequence_node *arguments, fir::body_node *body) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _arguments(arguments), _def_retval(nullptr),
        _body(body){
      type(funType);
    }

    function_definition_node(int lineno, std::shared_ptr<cdk::basic_type> funType, int qualifier, const std::string &identifier,
                               cdk::sequence_node *arguments, cdk::expression_node *def_retval, fir::body_node *body) :
          cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _arguments(arguments), _def_retval(def_retval),
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
    cdk::expression_node* def_retval() {
      return _def_retval;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_definition_node(this, level);
    }

  };

} // fir

#endif
