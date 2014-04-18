// <summary>
// Defines the interface for sending an asynchronous message.
// </summary>

namespace NanoRpc
{
    /// <summary>
    /// The interface for sending an asynchronous message.
    /// </summary>
    public interface IRpcMessageSender
    {
        /// <summary>
        /// Sends an asynchronous message.
        /// </summary>
        /// <param name="rpcMessage">
        /// The message to send.
        /// </param>
        void Send(RpcMessage rpcMessage);
    }
}

