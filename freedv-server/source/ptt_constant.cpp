#include "drivers.h"

// PTT driver that is constant transmit or constant receive. For testing.
namespace FreeDV {
  class PTTConstant : public PTTInput {
  public:
    			PTTConstant(const char * parameter);
    virtual		~PTTConstant();
    
  };

  PTTConstant::PTTConstant(const char * parameter)
  {
  }

  PTTConstant::~PTTConstant()
  {
  }

#ifndef NO_INITIALIZERS
  static PTTInput *
  creator(const char * parameter)
  {
    return new PTTConstant(parameter);
  }

  static bool
  initializer()
  {
    init_driver_manager().register_ptt_input("constant", creator);
    return true;
  }
  static const bool initialized = initializer();
#endif
}
