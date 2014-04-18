// <summary> 
// Represents an interface for sending a message with reply.
// </summary>

namespace NanoRpc
{
    /// <summary>
    /// Represents an interface for sending a message with reply.
    /// </summary>
    public interface IRpcClient : IRpcMessageSender, IRpcMessageRecipient
    {
        /// <summary>
        /// Sends a message and waits for reply.
        /// </summary>
        /// <param name="rpcMessage">
        /// The message to send.
        /// TODO: The argument type probably have to be changed to RpcCall. This may solve a lot of problems.
        /// </param>
        /// <returns>
        /// Replied result.
        /// </returns>
        RpcResult SendWithResult(RpcMessage rpcMessage);
    }
}

