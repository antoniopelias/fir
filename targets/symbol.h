#ifndef __FIR_TARGETS_SYMBOL_H__
#define __FIR_TARGETS_SYMBOL_H__

#include <string>
#include <memory>
#include <cdk/types/basic_type.h>

namespace fir {

  class symbol {
    std::shared_ptr<cdk::basic_type> _type;
    std::string _name;
    long _value; // hack!

    bool _function;

  public:
    symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, long value) :
        _type(type), _name(name), _value(value), _function(false) {
    }

    symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, long value, bool function) :
        _type(type), _name(name), _value(value), _function(function) {
    }

    virtual ~symbol() {
      // EMPTY
    }

    std::shared_ptr<cdk::basic_type> type() const {
      return _type;
    }
    bool is_typed(cdk::typename_type name) const {
      return _type->name() == name;
    }
    const std::string &name() const {
      return _name;
    }
    long value() const {
      return _value;
    }
    long value(long v) {
      return _value = v;
    }
    bool isFunction() const {
      return _function;
    }
  };

} // fir

#endif
