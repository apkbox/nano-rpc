// <summary> 
// </summary>

namespace IdlCompiler
{
    using System;
    using System.Xml;

    /// <summary>
    /// </summary>
    public static class XmlAttributeExtension
    {
        /// <summary>
        /// Splits list of values in attribute value.
        /// </summary>
        /// <param name="attribute">
        /// </param>
        /// <returns>
        /// </returns>
        public static string[] GetValueList(this XmlAttribute attribute)
        {
            return attribute.Value.Split(
                new[] { '\x20', '\x09', '\x0d', '\x0a' }, 
                StringSplitOptions.RemoveEmptyEntries);
        }
    }
}

