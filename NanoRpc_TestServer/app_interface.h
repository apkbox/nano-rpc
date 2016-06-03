#if !defined(APP_INTERFACE_INCLUDED__)
#define APP_INTERFACE_INCLUDED__

#include "app_test.pb.h"

namespace app {

class ITestApp {
public:
  virtual ~ITestApp() {}

  virtual bool get_BoolValue() const = 0;
  virtual void set_BoolValue(bool) = 0;

  virtual int get_IntValue() const = 0;
  virtual void set_IntValue(int) = 0;

  virtual AppEnum get_EnumValue() const = 0;
  virtual void set_EnumValue(AppEnum) = 0;

  virtual std::string get_StringValue() const = 0;
  virtual void set_StringValue(const std::string &) = 0;

  virtual void get_StructValue(AppStruct *) const = 0;
  virtual void set_StructValue(const AppStruct &) = 0;

  virtual void SyncVoidMethod() = 0;
  virtual void AsyncVoidMethod() = 0;

  virtual int SetStructReturnIntMethod(const AppStruct &) = 0;
  virtual void GetStructMethod(int, AppStruct *return_value) = 0;
};

} // namespace

#endif // APP_INTERFACE_INCLUDED__
