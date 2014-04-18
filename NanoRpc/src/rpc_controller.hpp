
#if !defined( NANO_RPC_RPC_CONTROLLER_HPP__ )
#define NANO_RPC_RPC_CONTROLLER_HPP__

#include "RpcMessageTypes.pb.h"

namespace NanoRpc {

class RpcChannel;
class RpcServer;
class RpcClient;
class RpcController;


class RpcController
{
	friend class RpcChannel;
	friend class RpcServer;
	friend class RpcClient;

public:
	RpcServer *get_server() { return server_; }
	RpcClient *get_client() { return client_; }
	RpcChannel *get_channel() { return channel_; }

protected:
	// Accessible by client and server
	void Send( const RpcMessage &message );

	// Accessible by channel
	void Receive( const RpcMessage &message );

	void set_server( RpcServer *server ) { server_ = server; }
	void set_client( RpcClient *client ) { client_ = client; }
	void set_channel( RpcChannel *channel ) { channel_ = channel; }

private:
	RpcChannel *channel_;
	RpcServer *server_;
	RpcClient *client_;
};


} // namespace


#endif // NANO_RPC_RPC_CONTROLLER_HPP__

