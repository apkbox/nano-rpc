namespace NanoRpc
{
    using System.Collections.Generic;

    /// <summary>
    /// The rpc event service.
    /// </summary>
    public class RpcEventService : IRpcService
    {
        private readonly HashSet<string> _eventInterfaces = new HashSet<string>();

        #region IRpcService Members

        /// <summary>
        /// The call method.
        /// </summary>
        /// <param name="rpcCall">
        /// The rpc call.
        /// </param>
        /// <returns>
        /// </returns>
        public RpcResult CallMethod(RpcCall rpcCall)
        {
            if( rpcCall.Method == "Add" )
            {
                _eventInterfaces.Add(rpcCall.GetParameters(0).StringValue);
            }
            else if( rpcCall.Method == "Remove" )
            {
                _eventInterfaces.Remove(rpcCall.GetParameters(0).StringValue);
            }

            return null;
        }

        #endregion

        /// <summary>
        /// The has interface.
        /// </summary>
        /// <param name="interfaceName">
        /// The interface name.
        /// </param>
        /// <returns>
        /// The has interface.
        /// </returns>
        public bool HasInterface(string interfaceName)
        {
            return _eventInterfaces.Contains(interfaceName);
        }
    }
}

