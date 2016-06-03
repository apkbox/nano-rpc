#include "app_interface.h"

namespace app {

class ITestApp_Impl : public ITestApp {
public:
  bool get_BoolValue() const override {
    return true;
  }

  void set_BoolValue(bool) override {

  }

  int get_IntValue() const override {
    return 42;
  }

  void set_IntValue(int) override {

  }

  AppEnum get_EnumValue() const override {
    return EnumValue3;
  }

  void set_EnumValue(AppEnum) override {

  }

  std::string get_StringValue() const override {
    return std::string("What is the meaning?");
  }
  void set_StringValue(const std::string &) override {

  }

  void get_StructValue(AppStruct *s) const override {
    s->set_string_value("What is another meaning?");
    s->set_bool_value(true);
    s->set_int_value(true);
    s->set_enum_value(EnumValue2);
  }

  void set_StructValue(const AppStruct &) override {

  }

  void SyncVoidMethod() override {}
  void AsyncVoidMethod() override {}

  int SetStructReturnIntMethod(const AppStruct &) override {
    return 10;
  }
  void GetStructMethod(int, AppStruct *return_value) override {
    return_value->set_bool_value(false);
    return_value->set_int_value(30);
    return_value->set_enum_value(EnumValue1);
    return_value->set_string_value("HelloFromGetStructMethod");
  }
};

} // namespace
