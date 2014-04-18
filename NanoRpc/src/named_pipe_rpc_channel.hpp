#if !defined( NANO_RPC_NAMED_PIPE_RPC_CHANNEL_HPP__ )
#define NANO_RPC_NAMED_PIPE_RPC_CHANNEL_HPP__

#include <windows.h>

#include "RpcMessageTypes.pb.h"

#include "rpc_channel.hpp"
#include "buffer_pool.hpp"
#include "object_pool.hpp"
#include "callback.hpp"

namespace NanoRpc {


class OverlappedOperation
{
public:
	enum Type {
		Undefined,
		ReadPrefix,
		ReadMessage,
		Write
	};
};


class Overlapped : public OVERLAPPED
{
public:
	class Initializer
	{
	public:
		void operator()( Overlapped *overlapped )
		{
			overlapped->Initialize();
		}
	};

	Overlapped()
	{
		Initialize();
	}

	~Overlapped()
	{
	}

	void Initialize()
	{
		memset( static_cast<OVERLAPPED *>( this ), 0, sizeof( OVERLAPPED ) );
		buffer = 0;
		operation_ = OverlappedOperation::Undefined;
	}

	char *buffer;
	OverlappedOperation::Type operation_;

private:
	Overlapped( const Overlapped & );
	Overlapped &operator=( const Overlapped & );
};


class NamedPipeRpcChannel : public RpcChannel
{
public:
	NamedPipeRpcChannel( RpcController *controller, HANDLE pipe_handle );
	~NamedPipeRpcChannel();

	virtual bool Start();
	virtual void Close();

	// Important: The handle passed in the disconnected callback is a closed pipe handle.
	void set_disconnected_callback( CallbackBase<HANDLE> *callback ) { disconnected_callback_ = callback; }
	CallbackBase<HANDLE> *get_disconnected_callback() { return disconnected_callback_; }

protected:
	virtual void Send( const RpcMessage &message );

private:
	enum ChannelState
	{
		NotConnected = 0,
		Connected = 1,
		Disconnected = 2
	};

	int IoCompletionThreadProc();

	static DWORD WINAPI IoCompletionThreadProcThunk( void *parameter )
	{
		NamedPipeRpcChannel *channel = reinterpret_cast<NamedPipeRpcChannel *>( parameter );
		return channel->IoCompletionThreadProc();
	}

	void StartRead( int size, OverlappedOperation::Type requested_operation );

	void ReadOperationCompleted( Overlapped *overlapped, DWORD bytes_read );
	void WriteOperationCompleted( Overlapped *overlapped, DWORD bytes_written );

	HANDLE CloseConnection();

	void HandleSurpriseDisconnect();

	void InvokeDisconnectedCallback( HANDLE pipe );

	Overlapped *AllocateOverlappedState( int size );
	void FreeOverlappedState( Overlapped *overlapped );

	HANDLE pipe_;
	HANDLE completion_port_;
	DWORD completion_thread_id_;
	HANDLE completion_thread_;

	BufferPool buffer_pool_;
	ObjectPool<Overlapped,Overlapped::Initializer> overlapped_pool_;

	CallbackBase<HANDLE> *disconnected_callback_;

	volatile __declspec(align(32)) LONG is_connected_;  // TODO: Disconnected should be 0 = not connected, 1 = connected, 2 = disconnected/closed
};


} // namespace

#endif // NANO_RPC_NAMED_PIPE_RPC_CHANNEL_HPP__



