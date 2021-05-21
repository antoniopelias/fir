#include <string>
#include "targets/type_checker.h"
#include "ast/all.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void fir::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  // TODO 
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl);
}

//---------------------------------------------------------------------------

void fir::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void fir::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}


//---------------------------------------------------------------------------

void fir::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void fir::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void fir::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

void fir::type_checker::do_null_node(fir::null_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::reference_type::create(4, nullptr));
}

//---------------------------------------------------------------------------

void fir::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (node->argument()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (node->argument()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  else if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    // TODO unspec pra float?
    fir::read_node *read = dynamic_cast<fir::read_node*>(node->argument());
    if(read != nullptr) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else
    throw std::string("Wrong type in argument of unary expression (Integer or double expected).");
}

void fir::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void fir::type_checker::do_identity_node(fir::identity_node * const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void fir::type_checker::do_index_node(fir::index_node * const node, int lvl) {
  // TODO 
}

void fir::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (node->argument()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    fir::read_node *read = dynamic_cast<fir::read_node*>(node->argument());

    if(read != nullptr) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else
    throw std::string("Wrong type in argument of unary expression (Integer expected).");
}

//---------------------------------------------------------------------------

void fir::type_checker::processBinaryIntegerNode(cdk::expression_node *const node, int lvl){
  node->accept(this, lvl + 2);
  if(node->is_typed(cdk::TYPE_UNSPEC)) {
    fir::read_node *read = dynamic_cast<fir::read_node *>(node);

    if(read != nullptr)
       node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if(!node->is_typed(cdk::TYPE_INT))
    throw std::string("Integer expression expected in binary operators.");
}

void fir::type_checker::IExpression(cdk::binary_operation_node *const node, int lvl){
  processBinaryIntegerNode(node->left(), lvl);
  processBinaryIntegerNode(node->right(), lvl);
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void fir::type_checker::IDExpression(cdk::binary_operation_node *const node, int lvl){
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else {
    // TODO tipos nao definidos
  }
} 

void fir::type_checker::PIDExpression(cdk::binary_operation_node *const node, int lvl, int pp_allowed=0){
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(node->left()->type());
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER)) {
    node->type(node->right()->type());
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER)) {
    // TODO
  } else if (pp_allowed && node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    // TODO tipos nao definidos, pode estar incompleto ou errado
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else {
    throw std::string("wrong types in binary expression");
  }
}

void fir::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  PIDExpression(node, lvl);
}

void fir::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  PIDExpression(node, lvl, 1);
}

void fir::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  IDExpression(node, lvl);
}

void fir::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  IDExpression(node, lvl);
}

void fir::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  IExpression(node, lvl);
}
void fir::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  IDExpression(node, lvl); // TODO possivelmente errado
}
void fir::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  IDExpression(node, lvl); // TODO possivelmente errado
}

void fir::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  IDExpression(node, lvl); // TODO possivelmente errado
}

void fir::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  IDExpression(node, lvl); // TODO possivelmente errado
}

void fir::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  PIDExpression(node, lvl); // TODO errado
}

void fir::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  PIDExpression(node, lvl); // TODO errado
}


void fir::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  IDExpression(node, lvl); // TODO possivelmente errado
}

void fir::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  IDExpression(node, lvl); // TODO possivelmente errado
}

//---------------------------------------------------------------------------

void fir::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<fir::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw id;
  }
}

void fir::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

void fir::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->rvalue()->accept(this, lvl + 2);
  if(node->lvalue()->is_typed(cdk::TYPE_UNSPEC))
    throw std::string("Left value must have a type.");

  if(node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
    fir::read_node *readr = dynamic_cast<fir::read_node *>(node->rvalue());
    fir::stack_alloc_node *stackr = dynamic_cast<fir::stack_alloc_node *>(node->rvalue());

    if(readr != nullptr) {
      if(node->lvalue()->is_typed(cdk::TYPE_INT) || node->lvalue()->is_typed(cdk::TYPE_DOUBLE))
        node->rvalue()->type(node->lvalue()->type());
      else
        throw std::string("Invalid expression for lvalue node.");
    }
    else if(stackr != nullptr) {
      if(node->lvalue()->is_typed(cdk::TYPE_POINTER))
        node->rvalue()->type(node->lvalue()->type());
      else 
        throw std::string("A pointer is required to allocate.");
    }
    else
      throw std::string("Unknown node with unspecified type");
  }

  if(node->lvalue()->is_typed(cdk::TYPE_INT) && node->rvalue()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }
  else if(node->lvalue()->is_typed(cdk::TYPE_DOUBLE) && (node->rvalue()->is_typed(cdk::TYPE_DOUBLE) || node->rvalue()->is_typed(cdk::TYPE_INT))) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  }
  else if(node->lvalue()->is_typed(cdk::TYPE_STRING) && node->rvalue()->is_typed(cdk::TYPE_STRING)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
  }
  else if(node->lvalue()->is_typed(cdk::TYPE_POINTER) && node->rvalue()->is_typed(cdk::TYPE_POINTER)) {
    //fir::null_node *n = dynamic_cast<fir::null_node *>(node->rvalue());

    //if(n == nullptr)
      //TODO: check pointer level     
      //typeOfPointer(cdk::reference_type_cast(node->lvalue()->type()), cdk::reference_type_cast(node->rvalue()->type()));
    node->type(node->lvalue()->type());
  }
  else {
    throw std::string("wrong types in assignment");
  }
}

//---------------------------------------------------------------------------


void fir::type_checker::do_evaluation_node(fir::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}


//---------------------------------------------------------------------------

void fir::type_checker::do_read_node(fir::read_node *const node, int lvl) {
  try {
    node->accept(this, lvl);
  } catch (const std::string &id) {
    //TODO erro correto?
    throw "undeclared variable '" + id + "'";
  }
}

//---------------------------------------------------------------------------

void fir::type_checker::do_while_node(fir::while_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void fir::type_checker::do_while_finally_node(fir::while_finally_node * const node, int lvl) {
  node->condition()->accept(this, lvl + 4); 
}

//---------------------------------------------------------------------------

void fir::type_checker::do_if_node(fir::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  
  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {
    fir::read_node *read = dynamic_cast<fir::read_node *>(node->condition());

    if(read != nullptr) {
      node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else {
      throw std::string("Unknown node with unspecified type.");
    }

  }
  else if (!node->condition()->is_typed(cdk::TYPE_INT)) 
    throw std::string("Expected integer condition.");
}

void fir::type_checker::do_if_else_node(fir::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  
  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {
    fir::read_node *read = dynamic_cast<fir::read_node *>(node->condition());

    if(read != nullptr) {
      node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else {
      throw std::string("Unknown node with unspecified type.");
    }

  }
  else if (!node->condition()->is_typed(cdk::TYPE_INT)) 
    throw std::string("Expected integer condition.");
}

//---------------------------------------------------------------------------

void fir::type_checker::do_return_node(fir::return_node *const node, int lvl) {
  //TODO acho que e empty mas o ruben nao
}

//---------------------------------------------------------------------------

void fir::type_checker::do_restart_node(fir::restart_node *const node, int lvl) {
  //EMPTY
}

//---------------------------------------------------------------------------

void fir::type_checker::do_leave_node(fir::leave_node *const node, int lvl) {
  //EMPTY
}

//---------------------------------------------------------------------------

void fir::type_checker::do_block_node(fir::block_node *const node, int lvl) {
  // TODO talvez vazio
}

//---------------------------------------------------------------------------

void fir::type_checker::do_body_node(fir::body_node * const node, int lvl) {
  // TODO talvez vazio
}

//---------------------------------------------------------------------------

void fir::type_checker::do_function_declaration_node(fir::function_declaration_node * const node, int lvl) {

}
//---------------------------------------------------------------------------

void fir::type_checker::do_function_definition_node(fir::function_definition_node * const node, int lvl) {

}
//---------------------------------------------------------------------------

void fir::type_checker::do_variable_declaration_node(fir::variable_declaration_node * const node, int lvl) {
  if (node->initializer() != nullptr) {
    node->initializer()->accept(this, lvl + 2);
    if (node->initializer()->is_typed(cdk::TYPE_UNSPEC)){
      fir::read_node *read = dynamic_cast<fir::read_node*>(node->initializer());
      fir::stack_alloc_node *stack = dynamic_cast<fir::stack_alloc_node*>(node->initializer());
      if(read != nullptr) {
        if(node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE))
          node->initializer()->type(node->type());
        else
          throw std::string("Unable to read input.");
      }
      else if (stack != nullptr) { // TODO what is this
        if (node->is_typed(cdk::TYPE_POINTER))
          node->initializer()->type(node->type());
      }
      else
        throw std::string("Unknown node with unspecified type.");
    }
    else if (node->is_typed(cdk::TYPE_INT)) {
      if (!node->initializer()->is_typed(cdk::TYPE_INT))
        throw std::string("Wrong type for initializer (integer expected).");
    }
    else if (node->is_typed(cdk::TYPE_DOUBLE)) {
      if (!node->initializer()->is_typed(cdk::TYPE_DOUBLE) && !node->initializer()->is_typed(cdk::TYPE_INT))
        throw std::string("Wrong type for initializer (integer or double expected).");
    }
    else if (node->is_typed(cdk::TYPE_STRING)) {
      if (!node->initializer()->is_typed(cdk::TYPE_STRING))
        throw std::string("Wrong type for initializer (string expected).");
    }
    else if (node->is_typed(cdk::TYPE_POINTER)) {
      if (!node->initializer()->is_typed(cdk::TYPE_POINTER))
        throw std::string("Wrong type for initializer (pointer expected).");
    }else
      throw std::string("Unknown type for variable initializer.");
  }
}

//---------------------------------------------------------------------------

void fir::type_checker::do_address_of_node(fir::address_of_node * const node, int lvl) {
  //TODO verificar se e mesmo assim
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->type(cdk::reference_type::create(4, node->lvalue()->type()));
}

//---------------------------------------------------------------------------

void fir::type_checker::do_sizeof_node(fir::sizeof_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->expression()->accept(this, lvl + 2);
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}
//---------------------------------------------------------------------------

void fir::type_checker::do_stack_alloc_node(fir::stack_alloc_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in allocation expression");
  }
  //RUBEN TODO FIXME
  auto mytype = cdk::reference_type::create(4, cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  node->type(mytype);
}

//---------------------------------------------------------------------------

void fir::type_checker::do_write_node(fir::write_node * const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void fir::type_checker::do_writeln_node(fir::writeln_node * const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void fir::type_checker::do_prologue_node(fir::prologue_node * const node, int lvl) {
  //EMPTY
}

//---------------------------------------------------------------------------

void fir::type_checker::do_function_call_node(fir::function_call_node * const node, int lvl) {
  ASSERT_UNSPEC;

  // const std::string &id = node->identifier();
  // auto symbol = _symtab.find(id);
  // if (symbol == nullptr) throw std::string("symbol '" + id + "' is undeclared.");
  // if (!symbol->isFunction()) throw std::string("symbol '" + id + "' is not a function.");

  // node->type(symbol->type());

  // if (node->arguments()->size() == symbol->number_of_arguments()) {
  //   node->arguments()->accept(this, lvl + 4);
  //   for (size_t ax = 0; ax < node->arguments()->size(); ax++) {
  //     if (node->argument(ax)->type() == symbol->argument_type(ax)) continue;
  //     if (symbol->argument_is_typed(ax, cdk::TYPE_DOUBLE) && node->argument(ax)->is_typed(cdk::TYPE_INT)) continue;
  //     throw std::string("type mismatch for argument " + std::to_string(ax + 1) + " of '" + id + "'.");
  //   }
  // } else {
  //   throw std::string(
  //       "number of arguments in call (" + std::to_string(node->arguments()->size()) + ") must match declaration ("
  //           + std::to_string(symbol->number_of_arguments()) + ").");
  // }
}