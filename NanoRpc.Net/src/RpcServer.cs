// <summary> 
// Implements RPC server.
// </summary>

namespace NanoRpc
{
    using System;
    using System.Diagnostics;

    /// <summary>
    /// Implements RPC server.
    /// </summary>
    public class RpcServer : IRpcServer
    {
        private readonly RpcController _rpcController;
        private readonly RpcEventService _rpcEventService = new RpcEventService();
        private readonly RpcObjectManager _rpcObjectManager = new RpcObjectManager();

        /// <summary>
        /// Initializes a new instance of the RpcServer class.
        /// </summary>
        /// <param name="rpcController">
        /// The controller.
        /// </param>
        public RpcServer(RpcController rpcController)
        {
            _rpcController = rpcController;
            _rpcController.Recipient = this;
            RegisterService(_rpcObjectManager);
            RegisterService(_rpcEventService);
        }

        #region IRpcMessageSender Members

        /// <summary>
        /// </summary>
        /// <param name="rpcMessage">
        /// </param>
        void IRpcMessageSender.Send(RpcMessage rpcMessage)
        {
            // NOTE: This method is used exclusively for sending events.
            // Send only if client interested.
            if(_rpcEventService.HasInterface(rpcMessage.Call.Service))
            {
                _rpcController.Send(rpcMessage);
            }
        }

        #endregion

        /// <summary>
        /// </summary>
        /// <param name="interfaceType">
        /// The interface type.
        /// </param>
        /// <param name="service">
        /// The service.
        /// </param>
        public void RegisterService( Type interfaceType, object service )
        {
            _rpcObjectManager.RegisterService(interfaceType, service);
        }

        /// <summary>
        /// </summary>
        /// <param name="service">
        /// </param>
        /// <typeparam name="T">
        /// </typeparam>
        public void RegisterService<T>(T service) where T : class
        {
            _rpcObjectManager.RegisterService(service);
        }

        /// <summary>
        /// </summary>
        /// <typeparam name="T">
        /// </typeparam>
        /// <returns>
        /// </returns>
        public T RegisterEventSource<T>() where T : class
        {
            // TODO: The underlying code should be smart enough to not send events to clients that are not interested.
            // TODO: It would be much better if proxy is smart enough to not even create a message for the event that the client is not interested in.
            return (T) RpcProxyBuilder.BuildEventSourceProxy(typeof( T ), this);
        }

        /// <summary>
        /// </summary>
        /// <param name="rpcMessage">
        /// </param>
        public void Receive(RpcMessage rpcMessage)
        {
            // TODO: Could that be we actually need rpcMessage.HasResult ?
            if( rpcMessage.Result != null && rpcMessage.Result.Status != RpcStatus.RpcSucceeded )
            {
                // TODO: Dont know what to do in this case yet.
                // We were waiting for the message and seen the channel break and received notification about it.
                // The best we can do is to notify the context (not there yet) about broken connection.
                // Currently there are no context and the server does not differentiate between clients and serve them
                // all as one. On the other side the RpcController can act as context.
                // TODO: Probably we should delist the client from event manager.
                return;
            }

            // TODO: Asynchronous calls, see below.
            // Here we have the option of handling incoming calls sequentually 
            // (basically allowing single thread service) or making them asynchronously
            // thus making the service implementation responsible for handling multithreaded calls.
            // This theoretically can be controlled by the attribute AllowAsynchronousCalls that applies
            // to class and method (in override fashion to the default server behavior).
            // The default server behavior can also be specified as Syncrhonous or Asynchronous.
            // For now make all calls synchronous.
            IRpcService service;

            var resultMessage = new RpcMessage.Builder();
            resultMessage.Id = rpcMessage.Id;

            if( rpcMessage.Call.ObjectId != 0 )
            {
                service = _rpcObjectManager.GetInstance(rpcMessage.Call.ObjectId);
                if(service == null)
                {
                    var callResult = new RpcResult.Builder();
                    callResult.Status = RpcStatus.RpcUnknownInterface;
                    callResult.ErrorMessage = "Marshalled object does not exist (was object disposed?).";
                    resultMessage.Result = callResult.Build();
                }
            }
            else
            {
                service = _rpcObjectManager.GetService(rpcMessage.Call.Service);
                if (service == null)
                {
                    var callResult = new RpcResult.Builder();
                    callResult.Status = RpcStatus.RpcUnknownInterface;
                    callResult.ErrorMessage = "Unknown interface";
                    resultMessage.Result = callResult.Build();
                }
            }

            if( service != null )
            {
                RpcResult rpcResult = service.CallMethod(rpcMessage.Call);
                // TODO: See server.cpp for details on ExpectResult.
                if(rpcMessage.Call.ExpectsResult)
                {
                    Debug.Assert(resultMessage.HasId, "Reply message must have call ID.");
                    resultMessage.Result = rpcResult;
                    _rpcController.Send(resultMessage.Build());
                }
            }
            else
            {
                // TODO: See server.cpp for details on ExpectResult.
                if(rpcMessage.Call.ExpectsResult)
                {
                    Debug.Assert(resultMessage.HasId, "Reply message must have call ID.");
                    Debug.Assert(resultMessage.HasResult, "Reply message must have result.");
                    Debug.Assert(resultMessage.Result.HasStatus, "Reply error message must have status.");
                    _rpcController.Send(resultMessage.Build());
                }
            }
        }
    }
}

