// <summary> 
// </summary>

namespace IdlCompiler
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// </summary>
    public class PlatformInfo
    {
        private readonly List<ComponentInfo> _components = new List<ComponentInfo>();

        public PlatformInfo(string name)
        {
            Name = name;
        }

        /// <summary>
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// </summary>
        public List<ComponentInfo> Components
        {
            get { return _components; }
        }
    }
}