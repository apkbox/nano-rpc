// <summary> 
//
// Compiles IDL file in XML representation.
//
// The full compilation workflow that produces interfaces, proxies 
// and stubs for C# and C++ shown below
//
// rpc_interface.xml -> 
//    templates.xslt -> IdlCompiler -> messages.proto 
//                                  -> interfaces.cs
//                                  -> interfaces.rpc.hpp
//                                  -> interfaces.rpc.cpp
//                                  -> proxystub.rpc.hpp
//                                  -> client_proxy.rpc.cpp
//                                  -> server_stub.rpc.cpp
//
// messages.proto -> protoc -> messages.protobin
//                          -> messages.pb.cc
//                          -> messages.pb.h
//
// messages.protobin -> ProtoGen -> messages.pb.cs
//
// </summary>

namespace IdlCompiler
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Text;
    using System.Xml;
    using System.Xml.Schema;
    using System.Xml.Xsl;
    using Odgs.Common;

    /// <summary>
    /// Implements command line interface.
    /// </summary>
    internal class Program
    {
        private const string ConfigXml = "IdlCompiler.xml";
        private const string ConfigXsd = "nano-rpc-idl-compiler-config.xsd";
        private const string NanoRpcIdlXsd = "nano-rpc-idl.xsd";
        private const string NanoRpcIdlXsdPb = "nano-rpc-idl-pb.xsd";

        private readonly CachingDocumentLoader _documentLoader = new CachingDocumentLoader();

        private readonly List<string> _idlFiles = new List<string>();

        private List<string> _selectedPlatforms;

        private string _startupDirectory;
        private string _outputDirectory;
        private CompilerConfig _config;

        private static void Main(string[] args)
        {
            Program program = new Program();

            DateTime start = DateTime.Now;
            try
            {
                program.Run(args);
            }
            catch( ApplicationErrorException )
            {
                // All handled exceptions should end up here.
                // Error message was already printed at the point of error or
                // when original exception handling
                Environment.Exit(100);
            }
#if !DEBUG
			catch( Exception ex ) {
				// All exceptions that were not handled fall here.
				Logger.LogError( "Failed with following error: {0}", ex.Message );
				Logger.LogText( Verbosity.Diagnostic, "{0}", ex );
				Environment.Exit( 100 );
			}
#endif
            DateTime end = DateTime.Now;

            Logger.LogInfo("Compilation done in: {0}", end - start);

            Environment.Exit(0);
        }

        private static List<KeyValuePair<string, string>> CreateOutputReferenceVariables(
            string idlFileName,
            PlatformInfo platformInfo)
        {
            var outputNameVariables = new List<KeyValuePair<string, string>>();

            foreach( var componentInfo in platformInfo.Components )
            {
                foreach( var outputInfo in componentInfo.OutputInfos )
                {
                    var inst = outputInfo.GetInstanceFor(idlFileName);
                    outputNameVariables.Add(new KeyValuePair<string, string>(
                                                "_output_" + outputInfo.ComponentInfo.Name + "_" + outputInfo.Type,
                                                inst.OutputName));
                }
            }

            return outputNameVariables;
        }

        private static void Usage()
        {
            Console.WriteLine("usage: IdlCompiler [options] idl_files");
            Console.WriteLine("\nSwitches:\n" +
                              "--platform=<platform>[,<platform>...] - Comma or semicolumn separated list of platforms to process\n" +
                              "--output-dir=<directory>              - Output directory\n" +
                              "--verbosity=<level>                   - Logging verbosity level\n");
            Console.WriteLine("\tVerbosity levels:\n" +
                              "\t\t0 - errors\n" +
                              "\t\t1 - warnings\n" +
                              "\t\t2 - important information\n" +
                              "\t\t3 - information\n" +
                              "\t\t4 - diagnostic\n" +
                              "\t\t5> - verbose\n");
        }

        private void Run(string[] args)
        {
            if( args.Length == 0 )
            {
                Usage();
                Environment.Exit(0);
            }

            ProcessCommandLine(args);

            _startupDirectory = Path.GetDirectoryName(Environment.GetCommandLineArgs()[0]);

            if( _outputDirectory == null )
            {
                _outputDirectory = ".";
            }

            _config = LoadConfig();

            // Check that all specified platforms are supported
            if( _selectedPlatforms != null )
            {
                foreach( var platformName in _selectedPlatforms )
                {
                    string name = platformName;
                    if( _config.Platforms.Find(platformInfo => platformInfo.Name == name) == null )
                    {
                        Logger.LogError("Unknown or unsupported platform '{0}'.", platformName);
                        throw new ApplicationErrorException();
                    }
                }
            }

            foreach( var argument in _idlFiles )
            {
                GenerateCode(argument);
            }
        }

        private void ProcessCommandLine(string[] args)
        {
            CommandLine cmdline = CommandLine.Parse(args);

            // TODO: I feel there is a better way to make validation on the CommandLine's side
            // but just can't yet figure out the best of various available design options.
            // So, for now to it here.
            foreach( var option in cmdline.Options )
            {
                switch( option.Key )
                {
                    case "verbosity":
                    case "platform":
                    case "output-dir":
                        if( option.Value.Count == 0 )
                        {
                            Logger.LogWarning("Command line option '{0}' requires an argument.", option.Key);
                        }

                        break;

                    default:
                        Logger.LogError("Unknown command line option '{0}'.", option.Key);
                        throw new ApplicationErrorException();
                }
            }

            if( cmdline.Options.ContainsKey("verbosity") )
            {
                if( cmdline.Options["verbosity"].Count > 0 )
                {
                    string slevel = cmdline.Options["verbosity"][0];
                    uint level;
                    if( UInt32.TryParse(slevel, out level) )
                    {
                        Logger.Level = (Verbosity) level;
                        Logger.LogInfo("Logging level is {0}", Logger.Level);
                    }
                    else
                    {
                        Logger.LogError("Invalid argument for command line option 'verbosity'");
                        throw new ApplicationErrorException();
                    }
                }
            }

            if( cmdline.Arguments.Count < 1 )
            {
                Logger.LogError("Missing IDL file name in the command line");
                throw new ApplicationErrorException();
            }

            foreach( var argument in cmdline.Arguments )
            {
                _idlFiles.Add(argument);
            }

            // Collect and setup constants
            cmdline.ConsolidateOptionValues("platform");
            if( cmdline.Options.ContainsKey("platform") )
            {
                _selectedPlatforms = cmdline.Options["platform"];
            }

            if( cmdline.Options.ContainsKey("output-dir") )
            {
                if( cmdline.Options["output-dir"].Count > 1 )
                {
                    Logger.LogError("The 'output-dir' option specified more than once.");
                    throw new ApplicationErrorException();
                }

                _outputDirectory = cmdline.Options["output-dir"][0].Trim();
                if( _outputDirectory.Length == 0 )
                {
                    Logger.LogWarning("The 'output-dir' option value is empty. Assuming current directory.");
                }
            }

            return;
        }

        private CompilerConfig LoadConfig()
        {
            Debug.Assert(_startupDirectory != null, "Startup directory is not set.");

            var schemaReader = new XmlTextReader(Path.Combine(_startupDirectory, ConfigXsd));
            var schema = XmlSchema.Read(schemaReader, OnDocumentValidation);

            var config = new XmlDocument();
            config.Schemas.Add(schema);
            config.Load(Path.Combine(_startupDirectory, ConfigXml));
            config.Validate(OnDocumentValidation);

            var ns = new XmlNamespaceManager(config.NameTable);
            ns.AddNamespace("p", "urn:odgs-oce-net:schemas-nano-rpc-idlcompiler-config");

            var workList = new CompilerConfig();

            foreach( XmlElement platformNode in config.SelectNodes("/p:platforms/p:platform", ns) )
            {
                var platformInfo = new PlatformInfo(platformNode.Attributes["name"].Value);

                foreach( XmlElement componentNode in platformNode.SelectNodes("p:component", ns) )
                {
                    var componentInfo = new ComponentInfo(platformInfo, componentNode.Attributes["name"].Value);
                    componentInfo.Run = componentNode.HasAttribute("run") ? componentNode.Attributes["run"].Value : null;
                    componentInfo.Arguments = componentNode.HasAttribute("with")
                                                  ? componentNode.Attributes["with"].Value
                                                  : null;

                    if( componentNode.HasAttribute("depends") )
                    {
                        componentInfo.DependsIds.AddRange(componentNode.Attributes["depends"].GetValueList());
                    }

                    foreach( XmlElement outputNode in componentNode.SelectNodes("p:output", ns) )
                    {
                        var outputInfo = new OutputInfo(componentInfo);
                        outputInfo.Id = outputNode.HasAttribute("id")
                                            ? outputNode.Attributes["id"].Value
                                            : Guid.NewGuid().ToString();
                        outputInfo.FileSuffix = outputNode.Attributes["extension"].Value;
                        outputInfo.TemplateFileName = outputNode.HasAttribute("template")
                                                          ? outputNode.Attributes["template"].Value
                                                          : null;
                        outputInfo.Type = outputNode.Attributes["type"].Value;

                        componentInfo.OutputInfos.Add(outputInfo);
                        workList.Outputs.Add(outputInfo);
                    }

                    platformInfo.Components.Add(componentInfo);
                    workList.ComponentInfos.Add(componentInfo);
                }

                workList.Platforms.Add(platformInfo);
            }

            // TODO: Do dependency resolution.
            return workList;
        }

        private string ExpandVariables(string xmlidlFileName, string input)
        {
            string result = input;

            result = result.Replace(@"${document_full_path}", xmlidlFileName);
            result = result.Replace(@"${document_basename}", Path.GetFileNameWithoutExtension(xmlidlFileName));
            result = result.Replace(@"${document_dir}", Path.GetDirectoryName(xmlidlFileName));
            result = result.Replace(@"${output_dir}", _outputDirectory);

            return result;
        }

        private void GenerateCode(string idlFileName)
        {
            XmlDocument idlDocument;

            try
            {
                _documentLoader.AddSchema(Path.Combine(_startupDirectory, NanoRpcIdlXsd));
                _documentLoader.AddSchema(Path.Combine(_startupDirectory, NanoRpcIdlXsdPb));

                idlDocument = _documentLoader.LoadAndValidate(idlFileName);
            }
            catch( IOException ex )
            {
                Logger.LogError("Failed to load IDL: '{0}': {1}", idlFileName, ex.Message);
                throw new ApplicationErrorException(ex);
            }

            foreach( var platformInfo in _config.Platforms )
            {
                if( _selectedPlatforms != null )
                {
                    Logger.LogText(Verbosity.Diagnostic, "Platforms filter specified.");

                    if( !_selectedPlatforms.Contains(platformInfo.Name) )
                    {
                        Logger.LogText(Verbosity.Diagnostic, "Skipping platform '{0}'.", platformInfo.Name);
                        continue;
                    }
                }

                var outputNameVariables = CreateOutputReferenceVariables(idlFileName, platformInfo);
                Logger.LogInfo("Generating for platform {0}", platformInfo.Name);

                foreach( var componentInfo in platformInfo.Components )
                {
                    Logger.LogInfo("Generating component: {0}", componentInfo.Name);

                    if( componentInfo.Run != null )
                    {
                        var processStartInfo = new ProcessStartInfo
                                                   {
                                                       UseShellExecute = false,
                                                       WorkingDirectory = Environment.CurrentDirectory,
                                                       FileName = Path.Combine(_startupDirectory, componentInfo.Run),
                                                       Arguments = ExpandVariables(idlFileName, componentInfo.Arguments)
                                                   };
                        try
                        {
                            Logger.LogInfo("Invoking external process: {0}", processStartInfo.FileName);
                            Logger.LogInfo("    with command line: {0}", processStartInfo.Arguments);
                            var process = Process.Start(processStartInfo);
                            process.WaitForExit();
                            if( process.ExitCode != 0 )
                            {
                                Logger.LogError(
                                    "Process {0} exited with code {1}",
                                    process.StartInfo.FileName,
                                    process.ExitCode);
                                return;
                            }
                        }
                        catch( Win32Exception ex )
                        {
                            Logger.LogError(ex.Message);
                            return;
                        }
                    }
                    else
                    {
                        foreach( var outputInfo in componentInfo.OutputInfos )
                        {
                            Logger.LogInfo(
                                "Generating output: {0}",
                                outputInfo.GetInstanceFor(idlFileName).OutputName);

                            GeneratePlatformCode(
                                idlDocument,
                                outputInfo.GetInstanceFor(idlFileName),
                                outputNameVariables);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// </summary>
        /// <param name="idlDocument">
        /// </param>
        /// <param name="outputInstanceInfo">
        /// </param>
        /// <param name="outputNameVariables">
        /// </param>
        /// <remarks>
        /// The following parameters are passed to the template:
        /// _guard_name - Capitalized document name with all punctuation characters replaced with underscore.
        /// _template_name - The full name of XSL template used to create output the document.
        /// _document_name - XML IDL document name.
        /// _document_base_name - XML IDL document base name (i.e. without extension).
        /// _document_full_path - The full path to the XML IDL document.
        /// _output_[component]_[output] - The name of file generated by specified component output.
        /// </remarks>
        private void GeneratePlatformCode(
            XmlDocument idlDocument,
            OutputInstanceInfo outputInstanceInfo,
            List<KeyValuePair<string, string>> outputNameVariables)
        {
            Debug.Assert(outputInstanceInfo.OutputInfo.TemplateFileName != null, "Template file name is not specified.");

            var transform = new XslCompiledTransform();
            transform.Load(
                Path.Combine(_startupDirectory, outputInstanceInfo.OutputInfo.TemplateFileName),
                XsltSettings.TrustedXslt,
                new XmlLocalResolver());

            var transformArguments = new XsltArgumentList();
            transformArguments.AddParam("_guard_name", string.Empty, outputInstanceInfo.GuardName);
            transformArguments.AddParam("_template_name", string.Empty, outputInstanceInfo.OutputInfo.TemplateFileName);
            transformArguments.AddParam("_document_name", string.Empty, outputInstanceInfo.DocumentName);
            transformArguments.AddParam("_document_base_name", string.Empty, outputInstanceInfo.DocumentBaseName);
            transformArguments.AddParam("_document_full_path", string.Empty, outputInstanceInfo.DocumentPath);

            foreach( var variable in outputNameVariables )
            {
                transformArguments.AddParam(variable.Key, string.Empty, variable.Value);
            }

            try
            {
                // TODO: Handle output generation in the temporary directory.
                string outputFileName = Path.Combine(_outputDirectory, outputInstanceInfo.OutputName);
                var streamWriter = new StreamWriter(outputFileName, false, new UTF8Encoding(false));

                transform.Transform(idlDocument, transformArguments, streamWriter);
            }
            catch( IOException ex )
            {
                Logger.LogError("Error while attempting to generate output: {0}", ex.Message);
                throw new ApplicationErrorException();
            }
            catch( XsltException ex )
            {
                Logger.LogError("Error while attempting to generate output: {0}", ex.Message);
                throw new ApplicationErrorException();
            }
        }

        private static void OnDocumentValidation(object sender, ValidationEventArgs e)
        {
            switch( e.Severity )
            {
                case XmlSeverityType.Warning:
                    Logger.LogWarning(e.Message);
                    break;

                case XmlSeverityType.Error:
                    Logger.LogError(e.Message);
                    break;
            }
        }
    }
}

