
namespace NanoRpc
{
    using System;

    /// <summary>
    /// </summary>
    public class RpcException : Exception
    {
        private readonly RpcStatus _status;

        /// <summary>
        /// Initializes 
        /// </summary>
        /// <param name="status">
        /// </param>
        public RpcException(RpcStatus status)
        {
            _status = status;
        }

        /// <summary>
        /// </summary>
        /// <param name="status">
        /// </param>
        /// <param name="message">
        /// </param>
        public RpcException(RpcStatus status, string message) :
            base(message)
        {
            _status = status;
        }

        /// <summary>
        /// Gets RPC status associated with exception.
        /// </summary>
        public RpcStatus Status
        {
            get { return _status; }
        }
    }
}