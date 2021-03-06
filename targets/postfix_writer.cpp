#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/frame_size_calculator.h"
#include "targets/postfix_writer.h"
#include "ast/all.h"  // all.h is automatically generated
#include "fir_parser.tab.h"

//---------------------------------------------------------------------------

void fir::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void fir::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
void fir::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  if (_inFunctionBody) {
    _pf.DOUBLE(node->value()); // load number to the stack
  } else {
    _pf.SDOUBLE(node->value());    // double is on the DATA segment
  }
}

void fir::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
  _pf.INT(0);
  _pf.EQ();
}

void fir::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JZ(mklbl(lbl));
  node->right()->accept(this, lvl + 2);
  _pf.AND();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

void fir::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JNZ(mklbl(lbl));
  node->right()->accept(this, lvl + 2);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  if (_inFunctionBody) {
    _pf.INT(node->value()); // integer literal is on the stack: push an integer
  } else {
    _pf.SINT(node->value()); // integer literal is on the DATA segment
  }
}

void fir::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl1;

  /* generate the string */
  _pf.RODATA(); // strings are DATA readonly
  _pf.ALIGN(); // make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
  _pf.SSTRING(node->value()); // output string characters

  if (_function) {
    // local variable initializer
    _pf.TEXT();
    _pf.ADDR(mklbl(lbl1));
  } else {
    // global variable initializer
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  _pf.NEG(); // 2-complement
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {
    std::shared_ptr<cdk::basic_type> t;
    t = node->type();
    while (t->name() == cdk::TYPE_POINTER) {
      t = cdk::reference_type::cast(t)->referenced();
    }
    if (t->name() == cdk::TYPE_DOUBLE)
    _pf.INT(3);
    else
    _pf.INT(2);
    _pf.SHTL();
  }

  node->right()->accept(this, lvl + 2);
  if ( node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    std::shared_ptr<cdk::basic_type> t;
    t = node->type();
    while (t->name() == cdk::TYPE_POINTER) {
      t = cdk::reference_type::cast(t)->referenced();
    }
    if (t->name() == cdk::TYPE_DOUBLE)
    _pf.INT(3);
    else
    _pf.INT(2);
    _pf.SHTL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DADD();
  else
    _pf.ADD();
}

void fir::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    std::shared_ptr<cdk::basic_type> t;
    t = node->type();
    while (t->name() == cdk::TYPE_POINTER) {
      t = cdk::reference_type::cast(t)->referenced();
    }
    if (t->name() == cdk::TYPE_DOUBLE)
    _pf.INT(3);
    else
    _pf.INT(2);
    _pf.SHTL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DSUB();
  else
    _pf.SUB();
}

void fir::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->is_typed(cdk::TYPE_INT)) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->is_typed(cdk::TYPE_INT)) 
    _pf.I2D();

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DMUL();
  else
    _pf.MUL();
}

void fir::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
   _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
   _pf.I2D();

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DDIV();
  else
    _pf.DIV();
}

void fir::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->left()->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.I2D();  
  
  _pf.MOD();
}

void fir::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->left()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();  

  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)){
    _pf.DCMP();
    _pf.INT(0);
  }
  
  _pf.LT();
}

void fir::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->left()->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.I2D();

  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)){
    _pf.DCMP();
    _pf.INT(0);
  }  
  
  _pf.LE();
}

void fir::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->left()->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.I2D();

  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)){
    _pf.DCMP();
    _pf.INT(0);
  }  
  
  _pf.GE();
}

void fir::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->left()->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.I2D();  
  
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)){
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.GT();
}

void fir::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->left()->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.I2D();  

  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)){
    _pf.DCMP();
    _pf.INT(0);
  }
  
  _pf.NE();
}

void fir::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->left()->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.I2D();  
  
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)){
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.EQ();
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  std::string id = node-> name();
  auto symbol = _symtab.find(id);
  if (symbol->global() && !symbol->isFunction()) {
    _pf.ADDR(symbol->name());
  } else {
    _pf.LOCAL(symbol->offset());
  }
}

void fir::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDDOUBLE();
  } else {
    // integers, pointers, and strings
    _pf.LDINT();
  }
}

void fir::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // TODO VER ENUNCIADO E APAGAR SE INUTIL
  // node->rvalue()->accept(this, lvl); // determine the new value
  // _pf.DUP32();
  // if (new_symbol() == nullptr) {
  //   node->lvalue()->accept(this, lvl); // where to store the value
  // } else {
  //     std::cout << "ELSE ENTERED" << std::endl << std::flush;
  //   _pf.DATA(); // variables are all global and live in DATA
  //   _pf.ALIGN(); // make sure we are aligned
  //   _pf.LABEL(new_symbol()->name()); // name variable location
  //   reset_new_symbol();
  //   _pf.SINT(0); // initialize it to 0 (zero)
  //   _pf.TEXT(); // return to the TEXT segment
  //   node->lvalue()->accept(this, lvl);  //DAVID: bah!
  // }
  // _pf.STINT(); // store the value at address


  node->rvalue()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->rvalue()->is_typed(cdk::TYPE_INT))
      _pf.I2D();
    _pf.DUP64();
  } else {
    _pf.DUP32();
  }

  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.STDOUBLE();
  } else {
    _pf.STINT();
  }
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_evaluation_node(fir::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.TRASH(node->argument()->type()->size());
}

//---------------------------------------------------------------------------
void fir::postfix_writer::do_read_node(fir::read_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _functions_to_declare.insert("readd");
    _pf.CALL("readd");
    _pf.LDFVAL64();
  } else if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_UNSPEC)) {
    _functions_to_declare.insert("readi");
    _pf.CALL("readi");
    _pf.LDFVAL32();
  }  else {
    error(node->lineno(), "Cannot read type");
  }
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_while_node(fir::while_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  _whileCond.push(++_lbl);
  _whileEnd.push(++_lbl);

  _pf.ALIGN();
  _pf.LABEL(mklbl(_whileCond.top()));
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(_whileEnd.top()));

  
  node->block()->accept(this, lvl + 2);
  _pf.JMP(mklbl(_whileCond.top()));
  
  _pf.ALIGN();
  _pf.LABEL(mklbl(_whileEnd.top()));
  
  _whileEnd.pop();
  _whileCond.pop();
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_restart_node(fir::restart_node * const node, int lvl) {
  int restarts = node -> literal() -> value();
  std::stack<int> cond = _whileCond;
  if (cond.size() != 0) {
    if (restarts >= 1) {
      while(restarts > 1){
          cond.pop();
          restarts--;
        }
        _pf.JMP(mklbl(cond.top()));
    }
    else {
      error(node->lineno(), "wrong argument in restart");
    }
  } else
    error(node->lineno(),  "'restart' outside 'while'");
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_leave_node(fir::leave_node * const node, int lvl) {
  int leaves = node -> literal() -> value();
  std::stack<int> end = _whileEnd;
  if (end.size() != 0) {
    if (leaves >= 1) {
      while(leaves > 1){
          end.pop();
          leaves--;
        }
        _pf.JMP(mklbl(end.top()));
    }
    else {
      error(node ->lineno(), "wrong argument in leave");
    }
  } else
      error(node ->lineno(), "'leave' outside 'while'");
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_if_node(fir::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_if_else_node(fir::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1 = lbl2));
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_return_node(fir::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (!_inFunctionEpilogue)
    _pf.JMP(_currentBodyEpilogueLabel);
  else 
    _pf.JMP(_currentBodyEndLabel); 
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_block_node(fir::block_node * const node, int lvl) {
  _symtab.push();
  node -> declarations() -> accept(this, lvl + 4);
  node -> instructions() -> accept(this, lvl + 4);
  _symtab.pop();
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_body_node(fir::body_node * const node, int lvl) {
  _currentBodyEpilogueLabel = mklbl(++_lbl);
  _currentBodyEndLabel = mklbl(++_lbl);
  _symtab.push();
  if (node -> prologue())
    node -> prologue() -> accept(this, lvl + 2);
  if (node -> block())
    node -> block() -> accept(this, lvl + 2);
  _pf.ALIGN();
  _pf.LABEL(_currentBodyEpilogueLabel);
  _inFunctionEpilogue = true;
  if (node -> epilogue())
    node -> epilogue() -> accept(this, lvl + 2);
  _inFunctionEpilogue = false;
  _pf.LABEL(_currentBodyEndLabel);
  _symtab.pop();
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_function_declaration_node(fir::function_declaration_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunctionBody || _offset >= 8) {
    error(node->lineno(), "cannot declare function in body or in args");
    return;
  }

  if (!new_symbol()) return;

  auto function = new_symbol();
  _functions_to_declare.insert(function->name());
  reset_new_symbol();

}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_function_definition_node(fir::function_definition_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if (_inFunctionBody || _offset >= 8) {
    error(node->lineno(), "cannot define function in body or in arguments");
    return;
  }

  _function = new_symbol();
  _function -> set_offset(-node->type()->size());
  _functions_to_declare.erase(_function->name());  // just in case
  reset_new_symbol();

  _offset = 8; // prepare for arguments (4: remember to account for return address)
  _symtab.push(); // scope of args

  if (node->arguments()->size() > 0) {
    for (size_t ix = 0; ix < node->arguments()->size(); ix++) {
      cdk::basic_node *arg = node->arguments()->node(ix);
      if (arg == nullptr) break; // this means an empty sequence of arguments
      arg->accept(this, 0); // the function symbol is at the top of the stack
    }
  }

  _pf.TEXT();
  _pf.ALIGN();
  if (node->qualifier() == tPUBLIC)
    _pf.GLOBAL(_function->name(), _pf.FUNC());
  _pf.LABEL(_function->name());

  // compute stack size to be reserved for local variables
  frame_size_calculator lsc(_compiler, _symtab, _function);
  node->accept(&lsc, lvl);
  _pf.ENTER(lsc.localsize()); // total stack size reserved for local variables

  _offset = -node->type()->size(); // prepare for local variable

  // the following flag is a slight hack: it won't work with nested functions
  _inFunctionBody = true;

  if (node->def_retval()) {
    node->def_retval()->accept(this, lvl);
    if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_STRING) || node->is_typed(cdk::TYPE_POINTER)) {
      _pf.LOCAL(_offset);
      _pf.STINT();
    } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
      if (node->def_retval()->type()->name() == cdk::TYPE_INT)
         _pf.I2D();
      _pf.LOCAL(_offset);
      _pf.STDOUBLE();
    } else {
      error(node->lineno(), "cannot initialize default return value");
    }
  } else {
    if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER)) {
      _pf.INT(0);
      _pf.LOCAL(_offset);
      _pf.STINT();
    } 
  }

  os() << "        ;; before body " << std::endl;
  node -> body() -> accept(this, lvl + 4);
  os() << "        ;; after body " << std::endl;
  _inFunctionBody = false;
  // _returnSeen = false;

  if(!node->is_typed(cdk::TYPE_VOID)){
    _pf.LOCAL(-node->type()->size());
    if(!node->is_typed(cdk::TYPE_DOUBLE)){
      _pf.LDINT();
      _pf.STFVAL32();
    }
    else{
      _pf.LDDOUBLE();
      _pf.STFVAL64();
    }
  }
  _pf.LEAVE();
  _pf.RET();

  _symtab.pop(); // scope of arguments
  
  if (node->identifier() == "fir") {
    // declare external functions
    for (std::string s : _functions_to_declare)
      _pf.EXTERN(s);
  }
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_variable_declaration_node(fir::variable_declaration_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto id = node->identifier();

  int offset = 0, typesize = node->type()->size(); // in bytes
  if (_inFunctionBody) {
    _offset -= typesize;
    offset = _offset;
  } else if (_offset >= 8) {
    offset = _offset;
    _offset += typesize;
  } else {
    offset = 0; // global variable
  }

  auto symbol = new_symbol();
  if (symbol) {
    symbol->set_offset(offset);
    reset_new_symbol();
  }

  if (_inFunctionBody) {
    // if we are dealing with local variables, then no action is needed
    // unless an initializer exists
    if (node->initializer()) {
      node->initializer()->accept(this, lvl);
      if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_STRING) || node->is_typed(cdk::TYPE_POINTER)) {
        _pf.LOCAL(symbol->offset());
        _pf.STINT();
      } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->initializer()->is_typed(cdk::TYPE_INT))
          _pf.I2D();
        _pf.LOCAL(symbol->offset());
        _pf.STDOUBLE();
      } else {
        error(node->lineno(), "cannot initialize");
      }
    }
  } else {
    if (!_function) {
      if (node->initializer() == nullptr) {
        _pf.BSS();
        _pf.ALIGN();
          if (node->qualifier() == tPUBLIC){
            _pf.GLOBAL(id, _pf.OBJ());
          }else if (node->qualifier() == tIMPORT){
            _pf.EXTERN(id);
            return;
          }
        _pf.LABEL(id);
        _pf.SALLOC(typesize);
      } else {

        if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE) || node->is_typed(cdk::TYPE_POINTER)) {
          if (node->constant()) {
            _pf.RODATA();
          } else {
            _pf.DATA();
          }
          _pf.ALIGN();
          if (node->qualifier() == tPUBLIC){
             _pf.GLOBAL(id, _pf.OBJ());
          }
          _pf.LABEL(id);

          if (node->is_typed(cdk::TYPE_INT)) {
            node->initializer()->accept(this, lvl);
          } else if (node->is_typed(cdk::TYPE_POINTER)) {
            node->initializer()->accept(this, lvl);
          } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
            if (node->initializer()->is_typed(cdk::TYPE_DOUBLE)) {
              node->initializer()->accept(this, lvl);
            } else if (node->initializer()->is_typed(cdk::TYPE_INT)) {
              cdk::integer_node *dclini = dynamic_cast<cdk::integer_node*>(node->initializer());
              cdk::double_node ddi(dclini->lineno(), dclini->value());
              ddi.accept(this, lvl);
            } else {
              error(node->lineno(), "bad initializer for real value");
            }
          }
        } else if (node->is_typed(cdk::TYPE_STRING)) {
          if (node->constant()) {
            int litlbl;
            // HACK!!! string literal initializers must be emitted before the string identifier
            _pf.RODATA();
            _pf.ALIGN();
            _pf.LABEL(mklbl(litlbl = ++_lbl));
            _pf.SSTRING(dynamic_cast<cdk::string_node*>(node->initializer())->value());
            _pf.ALIGN();
            _pf.LABEL(id);
            _pf.SADDR(mklbl(litlbl));
          } else {
            _pf.DATA();
            _pf.ALIGN();
            _pf.LABEL(id);
            node->initializer()->accept(this, lvl);
          }
        } else {
          error(node->lineno(), "unexpected initializer");
        }

      }

    }
  }

}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_while_finally_node(fir::while_finally_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  _whileCond.push(++_lbl);
  _whileEnd.push(++_lbl);

  _pf.ALIGN();
  _pf.LABEL(mklbl(_whileCond.top()));
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(_whileEnd.top()));

  
  node->theninstr()->accept(this, lvl + 2);
  _pf.JMP(mklbl(_whileCond.top()));
  
  _pf.ALIGN();
  _pf.LABEL(mklbl(_whileEnd.top()));
  
  _whileEnd.pop();
  _whileCond.pop();

  node->finallyinstr()->accept(this, lvl + 2);

}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_null_node(fir::null_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunctionBody) {
    _pf.INT(0);
  } else {
    _pf.SINT(0);
  }
}
//---------------------------------------------------------------------------

void fir::postfix_writer::do_address_of_node(fir::address_of_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->lvalue()->accept(this, lvl);
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_index_node(fir::index_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (node->base()) {
    node->base()->accept(this, lvl);
  } else {
    if (_function) {
      _pf.LOCV(-_function->type()->size());
    } else {
      error(node->lineno(), "trying to use return value outside function");
    }
  }

  node->index()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.INT(3);
  else
    _pf.INT(2);
  _pf.SHTL();
  _pf.ADD(); // add pointer and index

}
//---------------------------------------------------------------------------
void fir::postfix_writer::do_sizeof_node(fir::sizeof_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(node->expression()->type()->size());
}

//---------------------------------------------------------------------------
void fir::postfix_writer::do_stack_alloc_node(fir::stack_alloc_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  if(cdk::reference_type::cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE)
    _pf.INT(3);
  else
    _pf.INT(2);

  _pf.SHTL();
  _pf.ALLOC();    
  _pf.SP();
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_write_node(fir::write_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  for (size_t ix = 0; ix < node->argument()->size(); ix++) {
    auto child = dynamic_cast<cdk::expression_node*>(node->argument()->node(ix));

    std::shared_ptr<cdk::basic_type> etype = child->type();
    child->accept(this, lvl); // expression to print
    if (etype->name() == cdk::TYPE_INT || etype->name() == cdk::TYPE_UNSPEC) {
      _functions_to_declare.insert("printi");
      _pf.CALL("printi");
      _pf.TRASH(4); // trash int
    } else if (etype->name() == cdk::TYPE_DOUBLE) {
      _functions_to_declare.insert("printd");
      _pf.CALL("printd");
      _pf.TRASH(8); // trash double
    } else if (etype->name() == cdk::TYPE_STRING) {
      _functions_to_declare.insert("prints");
      _pf.CALL("prints");
      _pf.TRASH(4); // trash char pointer
    } else {
      error(node->lineno(), "cannot print expression of unknown type");
    }
  }
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_writeln_node(fir::writeln_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  for (size_t ix = 0; ix < node->argument()->size(); ix++) {
    auto child = dynamic_cast<cdk::expression_node*>(node->argument()->node(ix));

    std::shared_ptr<cdk::basic_type> etype = child->type();
    child->accept(this, lvl); // expression to print
    if (etype->name() == cdk::TYPE_INT || etype->name() == cdk::TYPE_UNSPEC) {
      _functions_to_declare.insert("printi");
      _pf.CALL("printi");
      _pf.TRASH(4); // trash int
    } else if (etype->name() == cdk::TYPE_DOUBLE) {
      _functions_to_declare.insert("printd");
      _pf.CALL("printd");
      _pf.TRASH(8); // trash double
    } else if (etype->name() == cdk::TYPE_STRING) {
      _functions_to_declare.insert("prints");
      _pf.CALL("prints");
      _pf.TRASH(4); // trash char pointer
    } else {
      error(node->lineno(), "cannot print expression of unknown type");
    }

  }

  _functions_to_declare.insert("println");
  _pf.CALL("println");
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_prologue_node(fir::prologue_node * const node, int lvl) {
  node -> declarations() -> accept(this, lvl + 4);
  node -> instructions() -> accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_function_call_node(fir::function_call_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto symbol = _symtab.find(node->identifier());

  size_t argsSize = 0;
  if (node->arguments() && node->arguments()->size() > 0) {

    for (int ax = node->arguments()->size() - 1; ax >= 0; ax--) {
      cdk::expression_node *arg = dynamic_cast<cdk::expression_node*>(node->arguments()->node(ax));
      arg->accept(this, lvl + 2);
      if (symbol->argument_is_typed(ax, cdk::TYPE_DOUBLE) && arg->is_typed(cdk::TYPE_INT)) {
        _pf.I2D();
      }
      argsSize += symbol->argument_size(ax);
    }

  }
  _pf.CALL(node->identifier());
  if (argsSize != 0) {
    _pf.TRASH(argsSize);
  }

  if (symbol->is_typed(cdk::TYPE_INT) || symbol->is_typed(cdk::TYPE_POINTER) || symbol->is_typed(cdk::TYPE_STRING)) {
    _pf.LDFVAL32();

  } else if (symbol->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDFVAL64();

  } 
}

//---------------------------------------------------------------------------

void fir::postfix_writer::do_identity_node(fir::identity_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
}

