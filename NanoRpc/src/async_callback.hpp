#if !defined( NANO_RPC_ASYNC_CALLBACK_HPP__ )
#define NANO_RPC_ASYNC_CALLBACK_HPP__

#include <windows.h>

namespace NanoRpc {

class ThreadPool;

class AsyncCallbackBase
{
	friend class ThreadPool;

public:
	virtual ~AsyncCallbackBase() {}

	operator WAITORTIMERCALLBACK()
	{
		return CallbackThunk;
	}

	WAITORTIMERCALLBACK get_callback_ptr()
	{
		return *this;
	}

	virtual void CallbackHandler( void *ptr, BOOLEAN timer_or_wait_fired ) = 0;

private:
	static void CALLBACK CallbackThunk( void *ptr, BOOLEAN timer_or_wait_fired );

	HANDLE wait_handle_;
};

// Note that state must be copyable.
template<class Object_,typename Method_,typename State_>
class AsyncCallback : public AsyncCallbackBase
{
	typedef AsyncCallback<Object_,Method_,typename State_> Self_;

public:
	AsyncCallback( Object_ *object, Method_ method, const State_ &state ) :
		object_( object ),
		method_( method ),
		state_( state )
	{
	}

	State_ &State() { return state_; }

	// TODO: This is temporary to troubleshoot some weirdness in overlapped I/O.
	void Invoke()
	{
		CallbackHandler( this, true );
	}

private:
	virtual void CallbackHandler( void *ptr, BOOLEAN timer_or_wait_fired )
	{
		Self_ *async = reinterpret_cast<Self_ *>( ptr );

		// We make local copy here, because the callback object can be deleted in the callback method,
		// thus invalidating state parameter. It is not apparent for the callback method implementer
		// that state is held in the AsyncCallback object.
		State_ state_copy = state_;
		((async->object_)->*(async->method_))( static_cast<AsyncCallbackBase *>( async ), state_copy );
	}

	Object_ *object_;
	Method_ method_;
	State_ state_;
};


template<class Object_,typename Method_,typename State_>
AsyncCallback<Object_,Method_,State_> *CreateAsyncCallback( Object_ *object, Method_ method, const State_ &state )
{
	return new AsyncCallback<Object_,Method_,State_>( object, method, state );
}


class ThreadPool
{
public:
	static bool RegisterAsyncIOOperation( HANDLE wait_handle, AsyncCallbackBase *callback );
};


} // namespace

#endif // NANO_RPC_ASYNC_CALLBACK_HPP__

