#if !defined(NANO_RPC_CALLBACK_HPP__)
#define NANO_RPC_CALLBACK_HPP__

namespace NanoRpc {

template <class Parameter_>
class CallbackBase {
public:
  virtual void Invoke(Parameter_ &value) = 0;
};

template <class Object_, class Method_, class Parameter_>
class Callback : public CallbackBase<Parameter_> {
public:
  Callback(Object_ *object, const Method_ method)
      : object_(object), method_(method) {}

  void Invoke(Parameter_ &param) { (object_->*method_)(param); }

private:
  Object_ *object_;
  Method_ method_;
};

template <class Parameter_, class Object_, class Method_>
Callback<Object_, Method_, Parameter_> *NewCallback(Object_ *object,
                                                    const Method_ method) {
  return new Callback<Object_, Method_, Parameter_>(object, method);
}

}  // namespace

#endif  // NANO_RPC_CALLBACK_HPP__