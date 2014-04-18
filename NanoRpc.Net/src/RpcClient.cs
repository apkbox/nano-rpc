// <summary> 
// Implementation of an RPC client.
// </summary>

namespace NanoRpc
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// Implementation of an RPC client.
    /// </summary>
    public class RpcClient : IRpcClient
    {
        private readonly RpcController _rpcController;

        private readonly PendingCallManager _pendingCalls = new PendingCallManager();
        private readonly Dictionary<string, IRpcService> _eventListeners = new Dictionary<string, IRpcService>();

        /// <summary>
        /// Initializes a new instance of the <see cref="RpcClient"/> class.
        /// </summary>
        /// <param name="rpcController">
        /// <see cref="RpcController"/> instance.
        /// </param>
        public RpcClient(RpcController rpcController)
        {
            _rpcController = rpcController;
            _rpcController.Recipient = this;
        }

        /// <summary>
        /// Creates proxy for the specified interface.
        /// </summary>
        /// <typeparam name="T">
        /// The interface type to build proxy for.
        /// </typeparam>
        /// <returns>
        /// A proxy object implementing <typeparamref name="T"/> interface.
        /// </returns>
        public T GetProxy<T>() where T : class
        {
            return (T) RpcProxyBuilder.BuildSingletonObjectProxy(typeof( T ), this);
        }

        /// <summary>
        /// Registers event listener interface.
        /// </summary>
        /// <param name="handler">
        /// The object that will handle incoming events.
        /// </param>
        /// <typeparam name="T">
        /// The event source interface type.
        /// </typeparam>
        /// <exception cref="ArgumentNullException">
        /// Thrown when <paramref name="handler"/> parameter is <c>null</c>.
        /// </exception>
        /// <exception cref="ArgumentException">
        /// The <paramref name="handler"/> type is a generic type parameter, an array type, 
        /// pointer type, or byref type based on a type parameter, or a generic type that is not a 
        /// generic type definition but contains unresolved type parameters.
        /// </exception>
        /// <remarks>
        /// <para>
        /// The registration creates a stub for the interface and stores it
        /// in a dictionary that is used to route incoming messages to the
        /// appropriate event interface.
        /// </para>
        /// <para>
        /// The registration does not cause server to send events to the registered interface.
        /// Use <see cref="StartListening"/> method to tell server that the client is interested
        /// in receiving events.
        /// </para>
        /// </remarks>
        public void RegisterEventListener<T>(T handler) where T : class
        {
            CodeContracts.NotNull(handler, "handler");
            CodeContracts.ResolvedType(typeof(T), "handler");

            // TODO: Event listeners should provide the way to register with a weak reference.
            // TODO: Currently stubs use reflection, which limits the performance. Need to implement dynamicly generated stubs.
            _eventListeners.Add(typeof( T ).FullName, new RpcReflectedStub<T>(handler, null));
        }

        /// <summary>
        /// Tells the server that the client is interested in receiving events.
        /// </summary>
        /// <param name="eventInterface">
        /// Interface to start listening to.
        /// </param>
        public void StartListening(Type eventInterface)
        {
            var rpcCall = new RpcCall.Builder();
            rpcCall.Service = "NanoRpc.RpcEventService";
            rpcCall.Method = "Add";
            rpcCall.ParametersList.Add(new RpcParameter.Builder().SetStringValue(eventInterface.FullName).Build());

            var rpcMessage = new RpcMessage.Builder();
            rpcMessage.Call = rpcCall.Build();

            // TODO: This probably should be a synchronous message
            _rpcController.Send(rpcMessage.Build());
        }

        /// <summary>
        /// Tells the server that the client does not want to receive events anymore.
        /// </summary>
        /// <param name="eventInterface">
        /// Interface to cancel listening to.
        /// </param>
        public void StopListening(Type eventInterface)
        {
            var rpcCall = new RpcCall.Builder();
            rpcCall.Service = "NanoRpc.RpcEventService";
            rpcCall.Method = "Remove";
            rpcCall.ParametersList.Add(new RpcParameter.Builder().SetStringValue(eventInterface.FullName).Build());

            var rpcMessage = new RpcMessage.Builder();
            rpcMessage.Call = rpcCall.Build();

            // TODO: This probably should be synchronous message
            _rpcController.Send(rpcMessage.Build());
        }

        #region IRpcClient Members

        /// <summary>
        /// Sends and asynchronous message.
        /// </summary>
        /// <param name="rpcMessage">
        /// The message to send.
        /// </param>
        public void Send(RpcMessage rpcMessage)
        {
            _rpcController.Send(rpcMessage);
        }

        /// <summary>
        /// Sends a message and waits for reply.
        /// </summary>
        /// <param name="rpcMessage">
        /// The message to send.
        /// </param>
        /// <returns>
        /// Replied result.
        /// </returns>
        public RpcResult SendWithResult(RpcMessage rpcMessage)
        {
            CodeContracts.NotNull(rpcMessage, "rpcMessage");
            CodeContracts.True(rpcMessage.HasCall, "rpcMessage.HasCall");

            // TODO: This code causes grief. However, wait until performance counters and stress tests are implemented, so we can measure improvement.
            // TODO: This code is inefficient, because it causes unnecessary copying when converting to builder.
            // TODO: One way may be to encapsulate RpcMessage (and other protobuf generated classes) as a builder and pass these around.
            // TODO: The encapsulation would also help to retarget implementation to other non-protobuf based implementation.
            // TODO: Simply replacing RpcMessage with RpcCall would not gain much, because we still have to set expects_result.
            // TODO: The bigger question, whether expects result should be in RpcMessage or stay in RpcCall, and who is responsible setting it - call issuer or client?
            // Set call id and expects_result.
            var pendingCall = new PendingCall();
            rpcMessage =
                rpcMessage.ToBuilder()
                    .SetId(pendingCall.Id)
                    .SetCall(
                        rpcMessage.Call.ToBuilder()
                            .SetExpectsResult(true).Build()).Build();

            _pendingCalls.Add(pendingCall);

            Send(rpcMessage);
            pendingCall.AsyncWaitHandle.WaitOne();

            return pendingCall.Result;
        }

        #endregion

        #region IRpcMessageRecipient Members

        /// <summary>
        /// Handles the received message by resuming a pending call or invoking the event listener.
        /// </summary>
        /// <param name="rpcMessage">
        /// The <see cref="RpcMessage"/> to handle.
        /// </param>
        public void Receive(RpcMessage rpcMessage)
        {
            CodeContracts.NotNull(rpcMessage, "rpcMessage");

            if( rpcMessage.Result.Status == RpcStatus.RpcChannelFailure )
            {
                _pendingCalls.FailAllPendingCalls(rpcMessage);
            }
            else
            {
                if( rpcMessage.HasCall )
                {
                    IRpcService eventListener;
                    if( _eventListeners.TryGetValue(rpcMessage.Call.Service, out eventListener) )
                    {
                        eventListener.CallMethod(rpcMessage.Call);
                    }
                }
                else
                {
                    _pendingCalls.CompletePendingCall(rpcMessage);
                }
            }
        }

        #endregion
    }
}

