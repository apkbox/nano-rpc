#if !defined( NANO_RPC_RPC_CHANNEL_HPP__ )
#define NANO_RPC_RPC_CHANNEL_HPP__

#include "RpcMessageTypes.pb.h"

namespace NanoRpc {

class RpcController;

class RpcChannel
{
	friend class RpcController;

public:
	explicit RpcChannel( RpcController *controller );
	virtual ~RpcChannel();

	// TODO: Should return bool to indicate fatal error that prevents
	// channel from running.
	virtual bool Start() = 0;
	virtual void Close() = 0;

protected:
	virtual void Send( const RpcMessage &message ) = 0;
	void Receive( const RpcMessage &message );

private:
	RpcController *controller_;
};



} // namespace


#endif // NANO_RPC_RPC_CHANNEL_HPP__

