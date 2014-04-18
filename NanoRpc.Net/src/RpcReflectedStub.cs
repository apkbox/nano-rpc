// <summary>
// Implements typesafe .NET reflection based server stub.
// </summary>

namespace NanoRpc
{
    /// <summary>
    /// Implements typesafe .NET reflection based server stub.
    /// </summary>
    /// <typeparam name="T">
    /// Type of the interface to make calls to.
    /// </typeparam>
    public class RpcReflectedStub<T> : RpcDynamicStub where T : class
    {
        /// <summary>
        /// Initializes a new instance of the RpcReflectedStub class.
        /// </summary>
        /// <param name="serviceInterface">
        /// Object instance implementing interface to make calls to.
        /// </param>
        /// <param name="objectManager">
        /// The context manager used to register contexts of returned objects.
        /// </param>
        public RpcReflectedStub(T serviceInterface, IRpcObjectManager objectManager) :
            base( serviceInterface, typeof(T), objectManager )
        {
        }
    }
}

