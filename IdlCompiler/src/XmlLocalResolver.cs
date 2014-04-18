// <summary> 
// Implements XML resource resolver that resolves references only 
// to the local file system.
// </summary>

namespace IdlCompiler
{
    using System;
    using System.IO;
    using System.Net;
    using System.Xml;

    /// <summary>
    /// </summary>
    internal class XmlLocalResolver : XmlResolver
    {
        /// <summary>
        /// </summary>
        public override ICredentials Credentials
        {
            set { }
        }

        /// <summary>
        /// </summary>
        /// <param name="absoluteUri">
        /// </param>
        /// <param name="role">
        /// </param>
        /// <param name="ofObjectToReturn">
        /// </param>
        /// <returns>
        /// </returns>
        public override object GetEntity(Uri absoluteUri, string role, Type ofObjectToReturn)
        {
            return new FileStream(absoluteUri.AbsolutePath, FileMode.Open, FileAccess.Read);
        }
    }
}

