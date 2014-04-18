namespace NanoRpc
{
    /// <summary>
    /// </summary>
    public abstract class RpcChannel
    {
        private readonly RpcController _rpcController;

        /// <summary>
        /// </summary>
        /// <param name="rpcController">
        /// </param>
        public RpcChannel(RpcController rpcController)
        {
            _rpcController = rpcController;
            _rpcController.Channel = this;
        }

        /// <summary>
        /// </summary>
        public abstract void Start();

        /// <summary>
        /// </summary>
        /// <param name="rpcMessage">
        /// </param>
        internal abstract void Send(RpcMessage rpcMessage);

        /// <summary>
        /// </summary>
        /// <param name="message">
        /// </param>
        protected void Receive(RpcMessage message)
        {
            _rpcController.Receive(message);
        }
    }
}

