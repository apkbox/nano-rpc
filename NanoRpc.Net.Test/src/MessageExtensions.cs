namespace NanoRpc.Net.Test
{
    /// <summary>
    /// The message extensions.
    /// </summary>
    internal static class MessageExtensions
    {
        /// <summary>
        /// Creates a call message.
        /// </summary>
        /// <param name="interfaceName">
        /// The interface name.
        /// </param>
        /// <param name="methodName">
        /// The method name.
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcMessage.Builder Message(string interfaceName, string methodName)
        {
            return Message(interfaceName, methodName, false);
        }

        /// <summary>
        /// Creates a call message that expects result.
        /// </summary>
        /// <param name="interfaceName">
        /// The interface name.
        /// </param>
        /// <param name="methodName">
        /// The method name.
        /// </param>
        /// <param name="expectsResult">
        /// The expects result.
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcMessage.Builder Message(string interfaceName, string methodName, bool expectsResult)
        {
            var call = new RpcCall.Builder()
                .SetService(interfaceName)
                .SetMethod(methodName);

            if( expectsResult )
            {
                call.SetExpectsResult(true);
            }

            return new RpcMessage.Builder().SetCall(call);
        }

        /// <summary>
        /// Creates empty result object with the specified status.
        /// </summary>
        /// <param name="status">
        /// The status.
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcResult.Builder Result(RpcStatus status)
        {
            return new RpcResult.Builder().SetStatus(status);
        }

        /// <summary>
        /// Creates result message with the specified value.
        /// </summary>
        /// <param name="status">
        /// The status.
        /// </param>
        /// <param name="valueBuilder">
        /// The value builder.
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcResult.Builder Result(RpcStatus status, RpcParameter.Builder valueBuilder)
        {
            return new RpcResult.Builder().SetStatus(status).SetCallResult(valueBuilder);
        }

        public static RpcResult.Builder Result(RpcParameter.Builder valueBuilder)
        {
            return new RpcResult.Builder().SetCallResult(valueBuilder);
        }

        /// <summary>
        /// Adds a bool parameter to the message.
        /// </summary>
        /// <param name="builder">
        /// The builder.
        /// </param>
        /// <param name="value">
        /// The value.
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcMessage.Builder WithParameter(this RpcMessage.Builder builder, bool value)
        {
            return builder.MergeCall(
                builder.Call.ToBuilder()
                    .AddParameters(Value(value))
                    .Build());
        }

        /// <summary>
        /// The with parameter.
        /// </summary>
        /// <param name="builder">
        /// The builder.
        /// </param>
        /// <param name="value">
        /// The value.
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcMessage.Builder WithParameter(this RpcMessage.Builder builder, int value)
        {
            return builder.MergeCall(
                builder.Call.ToBuilder()
                    .AddParameters(Value(value))
                    .Build());
        }

        public static RpcMessage.Builder WithParameter(this RpcMessage.Builder builder, uint value)
        {
            return builder.MergeCall(
                builder.Call.ToBuilder()
                    .AddParameters(Value(value))
                    .Build());
        }

        public static RpcMessage.Builder WithParameter(this RpcMessage.Builder builder, long value)
        {
            return builder.MergeCall(
                builder.Call.ToBuilder()
                    .AddParameters(Value(value))
                    .Build());
        }

        public static RpcMessage.Builder WithParameter(this RpcMessage.Builder builder, double value)
        {
            return builder.MergeCall(
                builder.Call.ToBuilder()
                    .AddParameters(Value(value))
                    .Build());
        }

        public static RpcMessage.Builder WithResult(this RpcMessage.Builder builder, RpcResult.Builder result)
        {
            return builder.SetResult(result);
        }

        public static RpcMessage.Builder WithResult(this RpcMessage.Builder builder, RpcStatus status)
        {
            return builder.SetResult(Result(status));
        }

        /// <summary>
        /// The value.
        /// </summary>
        /// <param name="value">
        /// The value.
        /// </param>
        /// <returns>
        /// </returns>
        public static RpcParameter.Builder Value(bool value)
        {
            return new RpcParameter.Builder().SetBoolValue(value);
        }

        public static RpcParameter.Builder Value(int value)
        {
            return new RpcParameter.Builder().SetInt32Value(value);
        }

        public static RpcParameter.Builder Value(uint value)
        {
            return new RpcParameter.Builder().SetUint32Value(value);
        }

        public static RpcParameter.Builder Value(long value)
        {
            return new RpcParameter.Builder().SetInt64Value(value);
        }

        public static RpcParameter.Builder Value(double value)
        {
            return new RpcParameter.Builder().SetDoubleValue(value);
        }
    }
}

