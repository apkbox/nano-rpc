// <summary> 
// Command line parser.
// </summary>

namespace Odgs.Common
{
    using System;
    using System.Collections.Generic;
    using System.Collections.Specialized;
    using System.Text.RegularExpressions;

    /// <summary>
    /// Parses the command line.
    /// </summary>
    /// <remarks>
    /// The parser recognizes the following command line option forms:
    /// <list type="bullet">
    ///     <item>
    ///         <term>--option</term>
    ///         <description>
    ///             Simple option.
    ///         </description>
    ///     </item>
    ///     <item>
    ///         <term>--option=</term>
    ///         <description>
    ///             Option with an empty value.
    ///         </description>
    ///     </item>
    ///     <item>
    ///         <term>--option=value</term>
    ///         <description>
    ///             Option with an specified value.
    ///         </description>
    ///     </item>
    /// </list>
    /// <para>
    /// Options are case sensitive.
    /// The forward slash <i>'/'</i> character can be used instead of <i>--</i>.
    /// </para>
    /// <para>
    /// The option can be specified multiple times. All values for each instance are preserved.
    /// </para>
    /// </remarks>
    public class CommandLine
    {
        private readonly CommandLineOptionsDictionary _flags = new CommandLineOptionsDictionary();
        private readonly StringCollection _parameters = new StringCollection();

        /// <summary>
        /// Gets the list of arguments in order they appear in the command line.
        /// </summary>
        public StringCollection Arguments
        {
            get { return _parameters; }
        }

        /// <summary>
        /// Gets the collection of command line options along with their values.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Each entry of the collection represents a single specified option.
        /// If multiple option instances are specified, they are collapsed into a single entry.
        /// </para>
        /// <para>
        /// If option specified without value (e.g --option), then value list is empty.
        /// If option specified with equal sign (e.g. --option=), but no value provided, then the list contains
        /// an empty string.
        /// If multiple option instances with values specified, then list contains values in order option
        /// instances appear in the command line.
        /// </para>
        /// <example>This example shows how options are stored:
        /// <code>
        ///       --option1 --option1 --option2= --option3= -option3=value1 -option3=value2
        /// 
        ///       "option1" : {}
        ///       "option2" : { "" }
        ///       "option3" : { "", "value1", "value2" }
        /// 
        /// </code>
        /// </example>
        /// </remarks>
        public CommandLineOptionsDictionary Options
        {
            get { return _flags; }
        }

        /// <summary>
        /// Parses the command line.
        /// </summary>
        /// <param name="args">
        /// Command line.
        /// </param>
        /// <returns>
        /// Object that contains parsed command line.
        /// </returns>
        public static CommandLine Parse(string[] args)
        {
            return Parse(args, 0);
        }

        /// <summary>
        /// Parses the command line starting from index.
        /// </summary>
        /// <param name="args">
        /// Command line.
        /// </param>
        /// <param name="start">
        /// Index in <c>args</c> to start parsing from.
        /// </param>
        /// <returns>
        /// Object that contains parsed command line.
        /// </returns>
        public static CommandLine Parse(string[] args, int start)
        {
            CommandLine commandLine = new CommandLine();

            for( int i = start; i < args.Length; i++ )
            {
                string arg = args[i];

                Match match = Regex.Match(
                        arg, 
                        "^(--|/)(?<name>[^=]+)(=(?<value>.*)?)?",
                        RegexOptions.Singleline | RegexOptions.CultureInvariant | RegexOptions.ExplicitCapture);
                if( match.Success )
                {
                    string name = match.Groups["name"].Value;
                    if(!commandLine._flags.ContainsKey(name))
                    {
                        commandLine._flags[name] = new List<string>();
                    }

                    Group valueGroup = match.Groups["value"];
                    if(valueGroup.Success)
                    {
                        commandLine._flags[name].Add(valueGroup.Value);
                    }
                }
                else
                {
                    commandLine._parameters.Add(arg);
                }
            }

            return commandLine;
        }

        /// <summary>
        /// Splits each option value and stores each part as a separate value.
        /// </summary>
        /// <param name="optionName">
        /// Option name.
        /// </param>
        /// <remarks>
        /// Semicolumn ';' and comma ',' used as separators.
        /// </remarks>
        public void ConsolidateOptionValues(string optionName)
        {
            ConsolidateOptionValues(optionName, new[] { ';', ',' });
        }

        /// <summary>
        /// Splits each option value and stores each part as a separate value.
        /// </summary>
        /// <param name="optionName">
        /// Option name.
        /// </param>
        /// <param name="separators">
        /// List of separator characters.
        /// </param>
        public void ConsolidateOptionValues(string optionName, char[] separators)
        {
            List<string> separateOptions;

            if(Options.TryGetValue(optionName, out separateOptions))
            {
                var consolidation = new List<string>();

                foreach( string option in separateOptions )
                {
                    string[] items = option.Split(separators, StringSplitOptions.RemoveEmptyEntries);
                    foreach( string item in items )
                    {
                        consolidation.Add(item);
                    }
                }

                _flags[optionName] = consolidation;
            }
        }
    }
}

