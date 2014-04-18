// <summary>
// Dictionary that holds command line options.
// </summary>

namespace Odgs.Common
{
    using System;
    using System.Collections.Generic;
    using System.Runtime.Serialization;

    /// <summary>
    /// Implements dictionary that holds command line options.
    /// </summary>
    [Serializable]
    public class CommandLineOptionsDictionary : Dictionary<string, List<string>>
    {
        /// <summary>
        /// Initializes a new instance of the CommandLineOptionsDictionary class.
        /// </summary>
        public CommandLineOptionsDictionary()
        {
        }

        /// <summary>
        /// Initializes a new instance of the CommandLineOptionsDictionary class.
        /// </summary>
        /// <param name="info">
        /// Serialization info.
        /// </param>
        /// <param name="context">
        /// Serialization context.
        /// </param>
        protected CommandLineOptionsDictionary(SerializationInfo info, StreamingContext context) :
            base(info, context)
        {
        }
    }
}

