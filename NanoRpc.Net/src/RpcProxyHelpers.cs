// <summary> 
// Implements helper methods that are used by RpcProxyGenerator.
// </summary>

namespace NanoRpc
{
    using System;
    using System.Diagnostics;
    using System.Reflection;
    using Google.ProtocolBuffers;

    /// <summary>
    /// Implements helper methods that are used by <see cref="RpcProxyGenerator"/>.
    /// </summary>
    public static class RpcProxyHelpers
    {
        public static readonly MethodInfo BuildRpcMessageMethodInfo =
            GetMethod("BuildRpcMessage", typeof( RpcCall.Builder ));

        public static readonly MethodInfo CreateServiceRpcCallBuilderMethodInfo =
            GetMethod("CreateServiceRpcCallBuilder", typeof( string ), typeof( string ));

        public static readonly MethodInfo CreateObjectRpcCallBuilderMethodInfo =
            GetMethod("CreateObjectRpcCallBuilder", typeof( uint ), typeof( string ));

        public static readonly MethodInfo BuildProxyMethodInfo =
            GetMethod("BuildProxy", typeof( IRpcClient ), typeof( Type ), typeof( uint ));

        public static readonly MethodInfo SendDeleteMessageMethodInfo =
            GetMethod("SendDeleteMessage", typeof( uint ), typeof( IRpcClient ));

        /// <summary>
        /// Gets <see cref="MethodInfo"/> for <c>public</c>, <c>static</c> method with the specified name and argument types.
        /// </summary>
        /// <param name="name">
        /// Method name.
        /// </param>
        /// <param name="argumentTypes">
        /// Argument types.
        /// </param>
        /// <returns>
        /// <see cref="MethodInfo"/> of the method or null.
        /// </returns>
        public static MethodInfo GetMethod(string name, params Type[] argumentTypes)
        {
            MethodInfo methodInfo = typeof( RpcProxyHelpers ).GetMethod(
                name,
                BindingFlags.Public | BindingFlags.Static | BindingFlags.ExactBinding,
                null,
                argumentTypes,
                null);
            Debug.Assert(methodInfo != null, String.Format("Undefined method {0}", name));
            return methodInfo;
        }

        /// <summary>
        /// Builds <see cref="RpcMessage"/> object from provided <see cref="RpcCall"/>
        /// </summary>
        /// <param name="rpcCallBuilder">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcMessage BuildRpcMessage(RpcCall.Builder rpcCallBuilder)
        {
            var rpcMessageBuilder = new RpcMessage.Builder();
            rpcMessageBuilder.Call = rpcCallBuilder.Build();
            return rpcMessageBuilder.Build();
        }

        public static RpcCall.Builder CreateServiceRpcCallBuilder(string interfaceName, string methodName)
        {
            var callBuilder = new RpcCall.Builder();
            callBuilder.Service = interfaceName;
            callBuilder.Method = methodName;
            return callBuilder;
        }

        public static RpcCall.Builder CreateObjectRpcCallBuilder(uint objectId, string methodName)
        {
            var callBuilder = new RpcCall.Builder();
            callBuilder.ObjectId = objectId;
            callBuilder.Method = methodName;
            return callBuilder;
        }

        /// <summary>
        /// </summary>
        /// <param name="value">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter.Builder CreateRpcParameterBuilder(IMessage value)
        {
            return new RpcParameter.Builder().SetProtoValue(value.ToByteString());
        }

        /// <summary>
        /// </summary>
        /// <param name="value">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter.Builder CreateRpcParameterBuilder(bool value)
        {
            return new RpcParameter.Builder().SetBoolValue(value);
        }

        /// <summary>
        /// </summary>
        /// <param name="value">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter.Builder CreateRpcParameterBuilder(int value)
        {
            return new RpcParameter.Builder().SetInt32Value(value);
        }

        /// <summary>
        /// </summary>
        /// <param name="value">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter.Builder CreateRpcParameterBuilder(long value)
        {
            return new RpcParameter.Builder().SetInt64Value(value);
        }

        /// <summary>
        /// </summary>
        /// <param name="value">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter.Builder CreateRpcParameterBuilder(double value)
        {
            return new RpcParameter.Builder().SetDoubleValue(value);
        }

        /// <summary>
        /// </summary>
        /// <param name="value">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter.Builder CreateRpcParameterBuilder(string value)
        {
            return new RpcParameter.Builder().SetStringValue(value);
        }

        /// <summary>
        /// </summary>
        /// <param name="client"></param>
        /// <param name="interfaceType">
        /// </param>
        /// <param name="objectId">
        /// </param>
        /// <returns>
        /// </returns>
        public static object BuildProxy(IRpcClient client, Type interfaceType, uint objectId)
        {
            return RpcProxyBuilder.BuildTransientObjectProxy(interfaceType, client, objectId);
        }

        /// <summary>
        /// Sends delete message for transient objects.
        /// </summary>
        /// <param name="objectId">
        /// Object ID.
        /// </param>
        /// <param name="client">
        /// RPC client.
        /// </param>
        public static void SendDeleteMessage(uint objectId, IRpcClient client)
        {
            if (objectId != 0)
            {
                var rpcCall = new RpcCall.Builder();
                rpcCall.Service = "NanoRpc.ObjectManagerService";
                rpcCall.Method = "Delete";
                rpcCall.ParametersList.Add(new RpcParameter.Builder().SetUint32Value(objectId).Build());

                var rpcMessage = new RpcMessage.Builder();
                rpcMessage.SetCall(rpcCall);
                client.Send(rpcMessage.Build());
            }
        }
    }
}

