namespace IdlCompiler
{
    using System;

    /// <summary>
    /// </summary>
    public class ApplicationErrorException : Exception
    {
        /// <summary>
        /// </summary>
        public ApplicationErrorException()
        {
        }

        /// <summary>
        /// </summary>
        /// <param name="inner">
        /// </param>
        public ApplicationErrorException(Exception inner) :
            base("Application error occurred", inner)
        {
        }

        /// <summary>
        /// </summary>
        /// <param name="message">
        /// </param>
        public ApplicationErrorException(string message) :
            base(message)
        {
        }
    }
}