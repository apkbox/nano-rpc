namespace IdlCompiler
{
    /// <summary>
    /// This class describes an abstract output of a component.
    /// </summary>
    public class OutputInfo
    {
        /// <summary>
        /// </summary>
        /// <param name="componentInfo">
        /// </param>
        public OutputInfo(ComponentInfo componentInfo)
        {
            ComponentInfo = componentInfo;
        }

        /// <summary>
        /// Gets the component this output belongs to.
        /// </summary>
        public ComponentInfo ComponentInfo { get; private set; }

        /// <summary>
        /// </summary>
        public string Id { get; set; }

        /// <summary>
        /// </summary>
        public string Type { get; set; }

        /// <summary>
        /// </summary>
        public string TemplateFileName { get; set; }

        /// <summary>
        /// </summary>
        public string FileSuffix { get; set; }

        /// <summary>
        /// Creates new output instance for the specified document name.
        /// </summary>
        /// <param name="documentName">
        /// </param>
        /// <returns>
        /// </returns>
        public OutputInstanceInfo GetInstanceFor(string documentName)
        {
            return new OutputInstanceInfo(this, documentName);
        }
    }
}

