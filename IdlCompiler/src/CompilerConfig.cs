// <summary> 
// Manages compiler configuration settings.
// </summary>

namespace IdlCompiler
{
    using System.Collections.Generic;

    /// <summary>
    /// </summary>
    public class CompilerConfig
    {
        private readonly List<ComponentInfo> _componentInfos = new List<ComponentInfo>();
        private readonly List<OutputInfo> _outputs = new List<OutputInfo>();
        private readonly List<PlatformInfo> _platforms = new List<PlatformInfo>();

        /// <summary>
        /// </summary>
        public List<PlatformInfo> Platforms
        {
            get { return _platforms; }
        }

        /// <summary>
        /// </summary>
        public List<ComponentInfo> ComponentInfos
        {
            get { return _componentInfos; }
        }

        /// <summary>
        /// </summary>
        public List<OutputInfo> Outputs
        {
            get { return _outputs; }
        }
    }
}

