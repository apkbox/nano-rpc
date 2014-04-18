#if !defined( NANO_RPC_RPC_CLIENT_HPP__ )
#define NANO_RPC_RPC_CLIENT_HPP__

namespace NanoRpc {

class IRpcClient : public IRpcMessageSender
{
public:
	// TODO: Rename to SendWithResult to be consistent with C# implementation.
	virtual void SendWithReply( RpcMessage &rpcMessage, RpcResult *result ) = 0;
};

} // namespace

#endif // NANO_RPC_RPC_CLIENT_HPP__