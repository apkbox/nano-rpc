// <summary>
// </summary>

namespace NanoRpc
{
    using System;

    public class RpcServerController : RpcController
    {
        public override void Receive(RpcMessage rpcMessage)
        {
            CodeContracts.NotNull(Recipient, "Recipient", new InvalidOperationException("Recipient property is null."));

            // Incoming call
            if( rpcMessage.HasCall )
            {
                Recipient.Receive(rpcMessage);
            }
            
            // Channel error
            if(rpcMessage.HasResult && rpcMessage.Result.Status != RpcStatus.RpcSucceeded)
            {
                Recipient.Receive(rpcMessage);
            }

            throw new RpcException(RpcStatus.RpcProtocolError, "Unexpected message format.");
        }
    }
}
