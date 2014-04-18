// <summary> 
// </summary>

namespace NanoRpc
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;

    internal class RpcObjectManager : IRpcService, IRpcObjectManager
    {
        private readonly Dictionary<uint, IRpcService> _objects = new Dictionary<uint, IRpcService>();
        private readonly Dictionary<string, uint> _services = new Dictionary<string, uint>();
        private uint _lastContextKey;

        #region IRpcObjectManager Members

        /// <summary>
        /// Retgisters an object instance in the catalog, creating the stub if necessary.
        /// </summary>
        /// <param name="objectInstance">
        /// Instance of the object to register.
        /// </param>
        /// <param name="interfaceType">
        /// Interface the object is suppose to implement. If <paramref name="objectInstance"/>
        /// implements <see cref="IRpcService"/>, then this parameter is ignored.
        /// </param>
        /// <returns>
        /// Unique object identifier.
        /// </returns>
        public uint RegisterInstance(object objectInstance, Type interfaceType)
        {
            Debug.Assert(interfaceType != null, "interfaceType is null.");
            Debug.Assert(objectInstance != null, "objectInstance is null.");

// ReSharper disable PossibleNullReferenceException
            IRpcService objectStub;

            // Check if object passed is stub, and if not, then create one.
            if( typeof( IRpcService ).IsAssignableFrom(objectInstance.GetType()) )
            {
                objectStub = objectInstance as IRpcService;
            }
            else
            {
                Debug.Assert(interfaceType.IsInstanceOfType(objectInstance), "objectInstance does not implement the type specified in interfaceType.");
                objectStub = new RpcDynamicStub(objectInstance, interfaceType, this);
            }

// ReSharper restore PossibleNullReferenceException
            _objects[++_lastContextKey] = objectStub;
            Debug.Assert(_lastContextKey != 0, "Object identifier space seems to run out.");

            return _lastContextKey;
        }

        #endregion

        #region IRpcService Members

        /// <summary>
        /// Parses the RPC call message, executes the method and returns the RPC message with result.
        /// </summary>
        /// <param name="rpcCall">
        /// The RPC call message.
        /// </param>
        /// <returns>
        /// The RPC result message.
        /// </returns>
        public RpcResult CallMethod(RpcCall rpcCall)
        {
            var rpcResult = new RpcResult.Builder();
            rpcResult.SetStatus(RpcStatus.RpcSucceeded);

            if( rpcCall.Method == "Delete" )
            {
                Debug.Assert(rpcCall.ParametersCount == 1, "Delete message does not match expected number or parameters.");
                Debug.Assert(rpcCall.ParametersList[0].HasUint32Value, "Delete message does not match required argument type.");

                if( rpcCall.ParametersCount != 1 || !rpcCall.ParametersList[0].HasUint32Value )
                {
                    rpcResult.SetStatus(RpcStatus.RpcInvalidCallParameter);
                    rpcResult.SetErrorMessage("IRpcObjectManager.Delete: Invalid call parameter or parameter count mismatch.");
                }
                else
                {
                    var objectId = rpcCall.ParametersList[0].Uint32Value;
                    DeleteObject(objectId);
                }
            }
            else
            {
                rpcResult.SetStatus(RpcStatus.RpcUnknownMethod);
                rpcResult.SetErrorMessage("Unknown method.");
            }

            return rpcResult.Build();
        }

        #endregion

        /// <summary>
        /// Registers an object that implements the specified interface as a singleton.
        /// </summary>
        /// <param name="interfaceType">
        /// The interface type.
        /// </param>
        /// <param name="service">
        /// The singleton object that implements the interface specified in <paramref name="interfaceType"/>.
        /// </param>
        /// <exception cref="ArgumentNullException">
        /// The <paramref name="interfaceType"/> or <paramref name="service"/> is null.
        /// </exception>
        /// <exception cref="ArgumentException">
        /// The <paramref name="service"/> object does not implement interface specified in <paramref name="interfaceType"/>.
        /// The <paramref name="interfaceType"/> type is not an interface, an generic interface or an unresolved type.
        /// </exception>
        public void RegisterService(Type interfaceType, object service)
        {
            CodeContracts.NotNull(interfaceType, "interfaceType");
            CodeContracts.NotNull(service, "service");
            CodeContracts.Implements(service, "service", interfaceType, "interfaceType");
            CodeContracts.ResolvedType(interfaceType, "interfaceType");

            string serviceName = interfaceType.FullName;
            uint objectId;
            if( _services.TryGetValue(serviceName, out objectId) )
            {
                _services.Remove(serviceName);
                DeleteObject(objectId);
            }

            _services.Add(serviceName, RegisterInstance(service, interfaceType));
        }

        public void RegisterService<T>(T service) where T : class
        {
            CodeContracts.NotNull(service, "service");

            RegisterService(typeof( T ), service);
        }

        /// <summary>
        /// Gets the service by name.
        /// </summary>
        /// <param name="name">
        /// The service name.
        /// </param>
        /// <returns>
        /// Service instance or null.
        /// </returns>
        public IRpcService GetService(string name)
        {
            uint objectId;
            if( _services.TryGetValue(name, out objectId) )
            {
                return GetInstance(objectId);
            }

            return null;
        }

        /// <summary>
        /// Gets an instance of the specified object by its object ID.
        /// </summary>
        /// <param name="objectId">
        /// The object id.
        /// </param>
        /// <returns>
        /// Object instance or null.
        /// </returns>
        public IRpcService GetInstance(uint objectId)
        {
            if( objectId == 0 )
            {
                return null;
            }

            IRpcService service;
            if( _objects.TryGetValue(objectId, out service) )
            {
                return service;
            }

            return null;
        }

        private void DeleteObject(uint objectId)
        {
            _objects.Remove(objectId);
        }
    }
}

