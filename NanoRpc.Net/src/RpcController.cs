namespace NanoRpc
{
    using System;

    /// <summary>
    /// </summary>
    /// TODO: Split this class to RpcClientController and RpcServerController.
    public abstract class RpcController
    {
        /// <summary>
        /// </summary>
        public RpcChannel Channel { get; set; }

        /// <summary>
        /// Gets or sets message recipient.
        /// </summary>
        public IRpcMessageRecipient Recipient { get; set; }

        /// <summary>
        /// </summary>
        /// <param name="rpcMessage">
        /// </param>
        public void Send(RpcMessage rpcMessage)
        {
            CodeContracts.NotNull(Channel, "Channel", new InvalidOperationException("Channel property is null."));

            Channel.Send(rpcMessage);
        }

        /// <summary>
        /// </summary>
        /// <param name="rpcMessage">
        /// </param>
        public abstract void Receive(RpcMessage rpcMessage);
    }
}

