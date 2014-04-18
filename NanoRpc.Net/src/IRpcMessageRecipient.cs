namespace NanoRpc
{
    public interface IRpcMessageRecipient
    {
        /// <summary>
        /// </summary>
        /// <param name="rpcMessage">
        /// </param>
        void Receive(RpcMessage rpcMessage);
    }
}
