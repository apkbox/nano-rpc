namespace NanoRpc
{
    /// <summary>
    /// </summary>
    /// TODO: Should be renamed to IRpcObject
    public interface IRpcService
    {
        /// <summary>
        /// Parses the RPC call message, executes the method and returns the RPC message with result.
        /// </summary>
        /// <param name="rpcCall">
        /// The RPC call message.
        /// </param>
        /// <returns>
        /// The RPC result message.
        /// </returns>
        RpcResult CallMethod(RpcCall rpcCall);
    }
}

