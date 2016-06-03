// <summary> 
// Implements .NET reflection based stub.
// </summary>

namespace NanoRpc
{
    using System;
    using System.Diagnostics;
    using System.Reflection;
    using Google.Protobuf;

    /// <summary>
    /// Implements .NET reflection based stub.
    /// </summary>
    public class RpcDynamicStub : IRpcService
    {
        private readonly IRpcObjectManager _objectManager;
        private readonly Type _serviceInterfaceType;
        private readonly object _serviceObject;

        /// <summary>
        /// Initializes a new instance of the RpcDynamicStub class.
        /// </summary>
        /// <param name="serviceObject">
        /// The object instance that services calls.
        /// </param>
        /// <param name="serviceInterfaceType">
        /// The inteface type the object implements.
        /// </param>
        /// <param name="objectManager">
        /// The context manager used to register contexts of returned objects.
        /// </param>
        public RpcDynamicStub(object serviceObject, Type serviceInterfaceType, IRpcObjectManager objectManager)
        {
            // ReSharper disable PossibleNullReferenceException
            Debug.Assert(serviceObject != null, "The serviceObject parameter is null.");
            Debug.Assert(serviceInterfaceType != null, "The serviceInterfaceType parameter is null.");
            Debug.Assert(serviceInterfaceType.IsInterface, "The serviceInterfaceType parameter is null.");
            Debug.Assert(
                        serviceInterfaceType.IsAssignableFrom(serviceObject.GetType()),
                         "The specified object does not implement interface.");

            // ReSharper restore PossibleNullReferenceException
            _serviceObject = serviceObject;
            _serviceInterfaceType = serviceInterfaceType;
            _objectManager = objectManager;
        }

        #region IRpcService Methods
        /// <summary>
        /// Deserializes an incoming RPC call and makes call to the implementation.
        /// </summary>
        /// <param name="rpcCall">
        /// RPC call to deserialize.
        /// </param>
        /// <returns>
        /// RPC result of the method call.
        /// </returns>
        /// <remarks>
        /// If RPC call was unsuccessful, the return value contains status code describing the failure.
        /// </remarks>
        public RpcResult CallMethod(RpcCall rpcCall)
        {
            var result = new RpcResult.Builder();
            var callResult = new RpcParameter.Builder();

            if(_serviceObject == null || _serviceInterfaceType == null)
            {
                result.Status = RpcStatus.RpcProtocolError;
                result.ErrorMessage = "Null interface pointer.";
            }
            else
            {
                // Try to find the method.
                MethodInfo methodInfo = _serviceInterfaceType.GetMethod(rpcCall.Method);
                if(methodInfo == null)
                {
                    result.Status = RpcStatus.RpcUnknownMethod;
                    result.ErrorMessage = "Unknown method";
                }
                else
                {
                    // Get number of method parameters and check if number of arguments, passed in
                    // the message, matches the method signature.
                    ParameterInfo[] methodParameters = methodInfo.GetParameters();
                    if(methodParameters.Length != rpcCall.ParametersCount)
                    {
                        result.Status = RpcStatus.RpcInvalidCallParameter;
                        result.ErrorMessage = "Parameter mismatch";
                    }
                    else
                    {
                        // Deserialize method arguments from the message.
                        var callParameters = new object[rpcCall.ParametersCount];

                        for(int i = 0; i < methodParameters.Length; i++)
                        {
                            var rpcParameter = rpcCall.ParametersList[i];

                            if(typeof(IMessage).IsAssignableFrom(methodParameters[i].ParameterType))
                            {
                                if(rpcParameter.HasProtoValue)
                                {
                                    MethodInfo parseFromMethodInfo = methodParameters[i].ParameterType.GetMethod(
                                        "ParseFrom",
                                        BindingFlags.Public | BindingFlags.Static,
                                        null,
                                        new[] { rpcParameter.ProtoValue.GetType() },
                                        null);
                                    Debug.Assert(parseFromMethodInfo != null, "Undefined method ParseFrom");
                                    if(parseFromMethodInfo == null)
                                    {
                                        result.Status = RpcStatus.RpcProtocolError;

                                        // TODO: Should it be InternalError or InternalFailure?
                                        result.ErrorMessage = "Protobuf failure.";
                                    }
                                    else
                                    {
                                        callParameters[i] = parseFromMethodInfo.Invoke(
                                            null,
                                            new[] { rpcParameter.ProtoValue });
                                    }
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Parameter type mismatch";
                                    break;
                                }
                            }
                            else if(methodParameters[i].ParameterType == typeof(bool))
                            {
                                if(rpcParameter.HasBoolValue)
                                {
                                    callParameters[i] = rpcParameter.BoolValue;
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Parameter type mismatch";
                                    break;
                                }
                            }
                            else if(methodParameters[i].ParameterType == typeof(int))
                            {
                                if(rpcParameter.HasInt32Value)
                                {
                                    callParameters[i] = rpcParameter.Int32Value;
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Parameter type mismatch";
                                    break;
                                }
                            }
                            else if(methodParameters[i].ParameterType == typeof(long))
                            {
                                if(rpcParameter.HasInt64Value)
                                {
                                    callParameters[i] = rpcParameter.Int64Value;
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Parameter type mismatch";
                                    break;
                                }
                            }
                            else if(methodParameters[i].ParameterType == typeof(double))
                            {
                                if(rpcParameter.HasDoubleValue)
                                {
                                    callParameters[i] = rpcParameter.DoubleValue;
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Parameter type mismatch";
                                    break;
                                }
                            }
                            else if(methodParameters[i].ParameterType == typeof(string))
                            {
                                if(rpcParameter.HasStringValue)
                                {
                                    callParameters[i] = rpcParameter.StringValue;
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Parameter type mismatch";
                                    break;
                                }
                            }
                            else if(methodParameters[i].ParameterType.IsEnum)
                            {
                                if(rpcParameter.HasInt32Value)
                                {
                                    callParameters[i] = rpcParameter.Int32Value;
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Parameter type mismatch";
                                    break;
                                }
                            }
                            else if(methodParameters[i].ParameterType.IsInterface)
                            {
                                result.Status = RpcStatus.RpcInvalidCallParameter;
                                result.ErrorMessage = "Interface pointer is not supported as an input parameter";
                                break;
                            }
                            else
                            {
                                result.Status = RpcStatus.RpcInvalidCallParameter;
                                result.ErrorMessage = "Unsupported parameter type";
                                break;
                            }
                        }

                        // Check whether all parameters have been successfully deserialized.
                        if(result.Status == RpcStatus.RpcSucceeded)
                        {
                            object objResult = methodInfo.Invoke(_serviceObject, callParameters);

                            if(typeof(IMessage).IsAssignableFrom(methodInfo.ReturnType))
                            {
                                if(objResult is IMessage)
                                {
                                    callResult.ProtoValue = ((IMessage)objResult).ToByteString();
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Return type mismatch";
                                }
                            }
                            else if(methodInfo.ReturnType == typeof(bool))
                            {
                                if(objResult is bool)
                                {
                                    callResult.BoolValue = (bool)objResult;
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Return type mismatch";
                                }
                            }
                            else if(methodInfo.ReturnType == typeof(int))
                            {
                                if(objResult is int)
                                {
                                    callResult.Int32Value = (int)objResult;
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Return type mismatch";
                                }
                            }
                            else if(methodInfo.ReturnType == typeof(long))
                            {
                                if(objResult is long)
                                {
                                    callResult.Int64Value = (long)objResult;
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Return type mismatch";
                                }
                            }
                            else if(methodInfo.ReturnType == typeof(double))
                            {
                                if(objResult is double)
                                {
                                    callResult.DoubleValue = (double)objResult;
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Return type mismatch";
                                }
                            }
                            else if(methodInfo.ReturnType == typeof(string))
                            {
                                if(objResult == null)
                                {
                                    callResult.IsNull = true;
                                }
                                else if(objResult is string)
                                {
                                    callResult.StringValue = (string)objResult;
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Return type mismatch";
                                }
                            }
                            else if(methodInfo.ReturnType.IsEnum)
                            {
                                if(objResult is Enum)
                                {
                                    // The only case the InvalidCastException could be thrown here
                                    // is if protobuf extended to support other types for enum besides int32
                                    // or another on-wire format is used.
                                    Debug.Assert(
                                        Enum.GetUnderlyingType(objResult.GetType()) != typeof(int),
                                        "Unexpected underlying type for enum.");

                                    // ReSharper disable PossibleInvalidCastException
                                    callResult.Int32Value = (int)objResult;

                                    // ReSharper restore PossibleInvalidCastException
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Return type mismatch";
                                }
                            }
                            else if(methodInfo.ReturnType.IsInterface)
                            {
                                if(objResult == null)
                                {
                                    callResult.IsNull = true;
                                }
                                else if(methodInfo.ReturnType.IsAssignableFrom(objResult.GetType()))
                                {
                                    if(_objectManager == null)
                                    {
                                        result.Status = RpcStatus.RpcProtocolError;
                                        result.ErrorMessage =
                                            "Object manager not set. Possibly stub is client a side event listener.";
                                    }
                                    else
                                    {
                                        callResult.ObjectIdValue = _objectManager.RegisterInstance(objResult, methodInfo.ReturnType);
                                    }
                                }
                                else
                                {
                                    result.Status = RpcStatus.RpcInvalidCallParameter;
                                    result.ErrorMessage = "Return type mismatch";
                                }
                            }
                            else
                            {
                                result.SetStatus(RpcStatus.RpcProtocolError); // TODO: Should be unsupported return type
                                result.SetErrorMessage("Unsupported return type");
                            }
                        }
                    }
                }
            }

            result.CallResult = callResult.Build();

            return result.Build();
        }

        #endregion
    }
}

