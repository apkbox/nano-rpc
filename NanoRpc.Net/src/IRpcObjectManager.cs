// <summary> 
// Provides interface for managing object instances.
// </summary>

namespace NanoRpc
{
    using System;

    /// <summary>
    /// Provides interface for managing object instances.
    /// </summary>
    public interface IRpcObjectManager
    {
        /// <summary>
        /// Registers an object that implements the specified interface.
        /// </summary>
        /// <param name="objectInstance">
        /// The object instance.
        /// </param>
        /// <param name="interfaceType">
        /// The interface type the object implements.
        /// </param>
        /// <returns>
        /// Object ID.
        /// </returns>
        uint RegisterInstance(object objectInstance, Type interfaceType);
    }
}

