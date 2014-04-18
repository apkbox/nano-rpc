#if !defined( NANO_RPC_RPC_STUB_HPP__ )
#define NANO_RPC_RPC_STUB_HPP__

#include "RpcMessageTypes.pb.h"
#include "rpc_service.hpp"

namespace NanoRpc {

class IRpcStub : public IRpcService
{
public:
	virtual ~IRpcStub() {}
	virtual const char *GetInterfaceName() const = 0;
};

} // namespace

#endif // NANO_RPC_RPC_STUB_HPP__

