namespace NanoRpc
{
    using System;

    public class RpcClientController : RpcController
    {
        /// <summary>
        /// </summary>
        /// <param name="rpcMessage">
        /// </param>
        public override void Receive(RpcMessage rpcMessage)
        {
            CodeContracts.NotNull(Recipient, "Recipient", new InvalidOperationException("Recipient property is null."));

            if( rpcMessage.HasResult )
            {
                // The only time we allow call to return is when underlying channel returns error.
                if( rpcMessage.HasCall && rpcMessage.Result.Status == RpcStatus.RpcSucceeded )
                {
                    throw new RpcException(RpcStatus.RpcProtocolError, "The message containing a result is not expected to have call.");
                }

                Recipient.Receive(rpcMessage);
            }
            else if( rpcMessage.HasCall )
            {
                // Incoming call are expected to be events, and events are not expected to return result.
                if( rpcMessage.Call.ExpectsResult )
                {
                    throw new RpcException(RpcStatus.RpcProtocolError, "Event message not supposed expect result.");
                }

                Recipient.Receive(rpcMessage);
            }
        }
    }
}

