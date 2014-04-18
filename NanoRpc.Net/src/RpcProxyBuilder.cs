// <summary> 
// Implements runtime proxy generation.
// </summary>

namespace NanoRpc
{
    using System;
    using System.Diagnostics;

    /// <summary>
    /// Creates proxies of various types.
    /// </summary>
    internal static class RpcProxyBuilder
    {
        /// <summary>
        /// Creates an instance of a singleton object proxy.
        /// </summary>
        /// <param name="interfaceType">
        /// The interface Type.
        /// </param>
        /// <param name="rpcClient">
        /// The client instance used to send and receive messages.
        /// </param>
        /// <returns>
        /// The build singleton proxy.
        /// </returns>
        public static object BuildSingletonObjectProxy(Type interfaceType, IRpcClient rpcClient)
        {
            CodeContracts.NotNull(interfaceType, "interfaceType");

            var proxyType = RpcProxyGenerator.GetProxyType(interfaceType, false);
            Debug.Assert(proxyType != null, "RpcProxyGenerator.GetProxyType returned null.");
            return Activator.CreateInstance(proxyType, rpcClient);
        }

        /// <summary>
        /// Creates an instance of an event source interface proxy.
        /// </summary>
        /// <param name="interfaceType">
        /// The interface Type.
        /// </param>
        /// <param name="rpcMessageSender">
        /// The object that is used to send asynchronous messages.
        /// </param>
        /// <returns>
        /// The build event source proxy.
        /// </returns>
        public static object BuildEventSourceProxy(Type interfaceType, IRpcMessageSender rpcMessageSender)
        {
            CodeContracts.NotNull(interfaceType, "interfaceType");

            var proxyType = RpcProxyGenerator.GetProxyType(interfaceType, true);
            Debug.Assert(proxyType != null, "RpcProxyGenerator.GetProxyType returned null.");
            return Activator.CreateInstance(proxyType, rpcMessageSender);
        }

        /// <summary>
        /// Creates an instance of an transient object proxy.
        /// </summary>
        /// <param name="interfaceType">
        /// The interface Type.
        /// </param>
        /// <param name="rpcClient">
        /// The client instance used to send and receive messages.
        /// </param>
        /// <param name="objectId">
        /// The object ID of the transient object.
        /// </param>
        /// <returns>
        /// The build transient object proxy.
        /// </returns>
        public static object BuildTransientObjectProxy(Type interfaceType, IRpcClient rpcClient, uint objectId)
        {
            CodeContracts.NotNull(interfaceType, "interfaceType");
            CodeContracts.NotEqualTo(objectId, (uint)0, "objectId");

            var proxyType = RpcProxyGenerator.GetProxyType(interfaceType, false);
            Debug.Assert(proxyType != null, "RpcProxyGenerator.GetProxyType returned null.");
            return Activator.CreateInstance(proxyType, rpcClient, objectId);
        }
    }
}

