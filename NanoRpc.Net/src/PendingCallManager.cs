namespace NanoRpc
{
    using System.Collections.Generic;
    using System.Linq;

    /// <summary>
    /// The pending call manager.
    /// </summary>
    internal class PendingCallManager
    {
        private readonly Dictionary<int, PendingCall> _pendingCalls = new Dictionary<int, PendingCall>();

        /// <summary>
        /// The add.
        /// </summary>
        /// <param name="pendingCall">
        /// The pending call.
        /// </param>
        public void Add(PendingCall pendingCall)
        {
            lock( _pendingCalls )
            {
                _pendingCalls.Add(pendingCall.Id, pendingCall);
            }
        }

        /// <summary>
        /// The fail all pending calls.
        /// </summary>
        /// <param name="rpcMessage">
        /// The rpc message.
        /// </param>
        public void FailAllPendingCalls(RpcMessage rpcMessage)
        {
            lock( _pendingCalls )
            {
                while( _pendingCalls.Count > 0 )
                {
                    var item = _pendingCalls.First();
                    _pendingCalls.Remove(item.Key);
                    item.Value.ReceiveResult(rpcMessage.Result);
                }
            }
        }

        /// <summary>
        /// The complete pending call.
        /// </summary>
        /// <param name="rpcMessage">
        /// The rpc message.
        /// </param>
        public void CompletePendingCall(RpcMessage rpcMessage)
        {
            PendingCall pendingCall;

            lock( _pendingCalls )
            {
                pendingCall = _pendingCalls[rpcMessage.Id];
                _pendingCalls.Remove(rpcMessage.Id);
            }

            pendingCall.ReceiveResult(rpcMessage.Result);
        }
    }
}

