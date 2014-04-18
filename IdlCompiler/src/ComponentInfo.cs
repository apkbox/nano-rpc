
namespace IdlCompiler
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// </summary>
    public class ComponentInfo
    {
        private readonly List<string> _dependsIds = new List<string>();
        private readonly List<OutputInfo> _outputInfos = new List<OutputInfo>();

        public ComponentInfo(PlatformInfo platformInfo, string name)
        {
            PlatformInfo = platformInfo;
            Name = name;
        }

        /// <summary>
        /// Gets the platform this component belongs to.
        /// </summary>
        public PlatformInfo PlatformInfo { get; private set; }

        /// <summary>
        /// Gets the component name.
        /// </summary>
        public string Name { get; private set; }

        /// <summary>
        /// </summary>
        public string Run { get; set; }

        /// <summary>
        /// </summary>
        public string Arguments { get; set; }

        /// <summary>
        /// </summary>
        public List<OutputInfo> OutputInfos
        {
            get { return _outputInfos; }
        }

        /// <summary>
        /// </summary>
        public List<string> DependsIds
        {
            get { return _dependsIds; }
        }
    }
}

