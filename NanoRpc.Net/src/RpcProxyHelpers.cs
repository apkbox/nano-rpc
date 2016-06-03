// <summary> 
// Implements helper methods that are used by RpcProxyGenerator.
// </summary>

namespace NanoRpc
{
    using System;
    using System.Diagnostics;
    using System.Reflection;
    using Google.Protobuf;

    /// <summary>
    /// Implements helper methods that are used by <see cref="RpcProxyGenerator"/>.
    /// </summary>
    public static class RpcProxyHelpers
    {
        public static readonly MethodInfo BuildRpcMessageMethodInfo =
            GetMethod("BuildRpcMessage", typeof( RpcCall ));

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
        public static RpcMessage BuildRpcMessage(RpcCall rpcCallBuilder)
        {
            var rpcMessageBuilder = new RpcMessage();
            rpcMessageBuilder.Call = rpcCallBuilder;
            return rpcMessageBuilder;
        }

        public static RpcCall CreateServiceRpcCallBuilder(string interfaceName, string methodName)
        {
            var callBuilder = new RpcCall();
            callBuilder.Service = interfaceName;
            callBuilder.Method = methodName;
            return callBuilder;
        }

        public static RpcCall CreateObjectRpcCallBuilder(uint objectId, string methodName)
        {
            var callBuilder = new RpcCall();
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
        public static RpcParameter CreateRpcParameterBuilder(IMessage value)
        {
            var par = new RpcParameter();
            par.ProtoValue.Value = value.ToByteString();
            return par;
        }

        /// <summary>
        /// </summary>
        /// <param name="value">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter CreateRpcParameterBuilder(bool value)
        {
            var par = new RpcParameter();
            par.BoolValue.Value = value;
            return par;
        }

        /// <summary>
        /// </summary>
        /// <param name="value">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter CreateRpcParameterBuilder(int value)
        {
            var par = new RpcParameter();
            par.Int32Value.Value = value;
            return par;
        }

        /// <summary>
        /// </summary>
        /// <param name="value">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter CreateRpcParameterBuilder(long value)
        {
            var par = new RpcParameter();
            par.Int64Value.Value = value;
            return par;
        }

        /// <summary>
        /// </summary>
        /// <param name="value">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter CreateRpcParameterBuilder(double value)
        {
            var par = new RpcParameter();
            par.DoubleValue.Value = value;
            return par;
        }

        /// <summary>
        /// </summary>
        /// <param name="value">
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter CreateRpcParameterBuilder(string value)
        {
            var par = new RpcParameter();
            par.StringValue.Value = value;
            return par;
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
                var rpcCall = new RpcCall();
                rpcCall.Service = "NanoRpc.ObjectManagerService";
                rpcCall.Method = "Delete";
                rpcCall.Parameters.Add(new RpcParameter() { Uint32Value = { Value = objectId } });

                var rpcMessage = new RpcMessage();
                rpcMessage.Call = (rpcCall);
                client.Send(rpcMessage);
            }
        }
    }
}

