namespace NanoRpc
{
    using System;
    using System.Diagnostics;

    internal static class CodeContracts
    {
        public static void True(bool argument, string argumentName)
        {
            Debug.Assert(argument, string.Format("{0} expected to be true.", argumentName));
            if( !argument )
            {
                throw new ArgumentException(string.Format("{0} expected to be true.", argumentName), argumentName);
            }
        }

        public static void NotNull(object argument, string argumentName)
        {
            Debug.Assert(argument != null, string.Format("{0} must not be null.", argumentName));
            if( argument == null )
            {
                throw new ArgumentNullException(argumentName);
            }
        }

        public static void NotNull(object argument, string argumentName, Exception exception)
        {
            Debug.Assert(argument != null, string.Format("{0} must not be null.", argumentName));
            if( argument == null )
            {
                throw exception;
            }
        }

        public static void IsInterface(Type argument, string argumentName)
        {
            string message = string.Format("{0} is not an interface.", argumentName);

            Debug.Assert(argument.IsInterface, message);
            if( !argument.IsInterface )
            {
                throw new ArgumentException(message, argumentName);
            }
        }

        public static void Implements<T>( object argument, string argumentName ) where T : class
        {
            string message = string.Format("{0} object must implement {1} interface.", argumentName, typeof(T).FullName);

            Debug.Assert(
                argument != null &&
                typeof(T).IsAssignableFrom(argument.GetType()), 
                message);
            if( argument == null || !typeof(T).IsAssignableFrom(argument.GetType()) )
            {
                throw new ArgumentException(message, argumentName);
            }
        }

        public static void Implements(
            object objectArgument, 
            string objectArgumentName,
            Type interfaceArgument,
            string interfaceArgumentName)
        {
            NotNull(interfaceArgument, interfaceArgumentName);

            var message = string.Format(
                "{0} object does not implement interface {1} passed in {2} interface.",
                objectArgumentName,
                interfaceArgument.FullName,
                interfaceArgumentName);

            Debug.Assert(
                objectArgument != null &&
                interfaceArgument.IsAssignableFrom(objectArgument.GetType()),
                message);

            if( objectArgument == null || !interfaceArgument.IsAssignableFrom(objectArgument.GetType()) )
            {
                throw new ArgumentException(message, objectArgumentName);
            }
        }

        public static void ResolvedType(Type argument, string argumentName)
        {
            var message = string.Format("The passed {0} is generic, unresolved generic or not an object.", argumentName);

            Debug.Assert(argument.FullName != null, message);
            if( argument.FullName == null )
            {
                throw new ArgumentException(message, argumentName);
            }
        }

        public static void NotEqualTo<T>( T argument, T value, string argumentName) where T : IEquatable<T> 
        {
            string message = string.Format("{0} argument is not equal to {1}.", argumentName, value);

            Debug.Assert(!argument.Equals(value), message);
            if( argument.Equals(value) )
            {
                throw new ArgumentException(message, argumentName);
            }
        }

        public static void NotNullOrEmpty(string argument, string argumentName)
        {
            string message = string.Format("{0} must not be null or an empty string.", argumentName);

            Debug.Assert(!string.IsNullOrEmpty(argument), message);
            if( string.IsNullOrEmpty(argument) )
            {
                throw new ArgumentException(message, argumentName);
            }
        }
    }
}

