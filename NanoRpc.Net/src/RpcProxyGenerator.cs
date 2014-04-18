// <summary> 
// Provides low level implementation of the proxy generator.
// </summary>

#if DEBUG
#define USE_SYMBOL_WRITER
#define USE_SOURCE_WRITER
#endif

namespace NanoRpc
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
#if USE_SYMBOL_WRITER
    using System.Diagnostics.SymbolStore;
#endif
#if USE_SOURCE_WRITER
    using System.IO;
#endif
    using System.Linq;
    using System.Reflection;
    using System.Reflection.Emit;
    using Google.ProtocolBuffers;

    /// <summary>
    /// Provides low level implementation of the proxy generator.
    /// </summary>
    /// <remarks>
    /// The generator creates types for two distinct proxy types.
    /// <list type="table">
    /// <listheader>
    /// <term>Type</term>
    /// <description>Function</description>
    /// </listheader>
    /// <item>
    /// <term>Normal</term>
    /// <description>The type used for proxying singleton and transient objects on the client side.</description>
    /// </item>
    /// <item>
    /// <term>Event Source</term>
    /// <description>The type used for proxying event interface on the server side.</description>
    /// </item>
    /// </list>
    /// <para>
    /// The generation strategy is different depending on compilation mode.
    /// When compiled in debug mode, each interface gets its own assembly and module in that assembly.
    /// When compiled in release mode, assembly is created per namespace, but all interfaces 
    /// in a single namespace share the same module.
    /// </para>
    /// </remarks>
    internal class RpcProxyGenerator
    {
        // Cached MethodInfos.
        private static readonly MethodInfo RpcCallBuilderAddParametersMethodInfo =
            typeof( RpcCall.Builder ).GetMethod(
                "AddParameters",
                new[] { typeof( RpcParameter.Builder ) });

        private static readonly MethodInfo RpcMessageSenderSendMethodInfo =
            typeof( IRpcMessageSender ).GetMethod("Send", new[] { typeof( RpcMessage ) });

        private static readonly MethodInfo RpcClientSendWithResultMethodInfo =
            typeof( IRpcClient ).GetMethod("SendWithResult", new[] { typeof( RpcMessage ) });

        private static readonly MethodInfo ObjectFinalizeMethodInfo =
            typeof( object ).GetMethod(
                "Finalize",
                BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.ExactBinding);

        private const string ProxyModuleNameSuffix = ".Proxy";
        private const string ProxyModuleFileNameSuffix = ".Proxy.mod";
        private const string ProxyTypeSuffix = "Proxy";
        private const string ProxyAssemblyNameSuffix = ".Proxy";

#if DEBUG
        private const string ProxyAssemblyFileNameSuffix = ".Proxy.dll";
#endif

        /// <summary>
        /// The interface type to create proxy for.
        /// </summary>
        private readonly Type _interfaceType;

        /// <summary>
        /// Indicates whether the interface to be implemented 
        /// is an event source interface.
        /// </summary>
        private readonly bool _isEventSource;

        /// <summary>
        /// Caches created proxy types by interface type.
        /// </summary>
        private static readonly Dictionary<Type, Type> TypeCache = new Dictionary<Type, Type>();

        /// <summary>
        /// Caches assembly builders by namespace.
        /// </summary>
        private static readonly Dictionary<string, AssemblyBuilder> AssemblyBuilders = new Dictionary<string, AssemblyBuilder>();

        private static readonly object SyncRoot = new object();

        private AssemblyBuilder _assemblyBuilder;
        private ModuleBuilder _moduleBuilder;

#if USE_SYMBOL_WRITER
        private ISymbolDocumentWriter _symDocument;
        private ISymbolWriter _symWriter;
        private int _sourceLine;
#endif

#if USE_SOURCE_WRITER
        private StreamWriter _sourceWriter;
#endif

        private FieldBuilder _clientFieldBuilder;
        private FieldBuilder _objectIdFieldBuilder;
        private TypeBuilder _proxyClassBuilder;
        private FieldBuilder _senderFieldBuilder;

        private RpcProxyGenerator(Type interfaceType, bool isEventSource)
        {
            _interfaceType = interfaceType;
            _isEventSource = isEventSource;
        }

        /// <summary>
        /// Gets proxy type for the specified interface.
        /// </summary>
        /// <param name="interfaceType">
        /// The interface type.
        /// </param>
        /// <param name="isEventSource">
        /// Specifies whether the <paramref name="interfaceType"/> is an event source inteface.
        /// </param>
        /// <returns>
        /// Proxy type.
        /// </returns>
        /// <remarks>
        /// The method caches generated type.
        /// </remarks>
        public static Type GetProxyType(Type interfaceType, bool isEventSource)
        {
            CodeContracts.NotNull(interfaceType, "interfaceType");
            CodeContracts.IsInterface(interfaceType, "interfaceType");

            Type proxyType;

            lock( TypeCache )
            {
                if( !TypeCache.TryGetValue(interfaceType, out proxyType) )
                {
                    proxyType = new RpcProxyGenerator(interfaceType, isEventSource).CreateProxyType();
                    TypeCache.Add(interfaceType, proxyType);
                }
            }

            return proxyType;
        }

        private static string TypeToCSharpTypeString(Type parameterType)
        {
            if( parameterType == typeof(void))
            {
                return "void";
            }

            if( parameterType == typeof(bool))
            {
                return "bool";
            }

            if( parameterType == typeof(int))
            {
                return "int";
            }

            if( parameterType == typeof(long))
            {
                return "long";
            }

            if( parameterType == typeof(double))
            {
                return "double";
            }

            return parameterType.FullName;
        }

// ReSharper disable MemberCanBeMadeStatic.Local
// ReSharper disable UnusedParameter.Local
        [Conditional("USE_SOURCE_WRITER"), Conditional("USE_SYMBOL_WRITER")]
        private void WriteSourceLine(string text)
        {
#if USE_SOURCE_WRITER
            _sourceWriter.WriteLine(text);
#endif
#if USE_SYMBOL_WRITER
            _sourceLine++;
#endif
        }

        [Conditional("USE_SOURCE_WRITER"), Conditional("USE_SYMBOL_WRITER")]
        private void WriteSourceLine(string text, params object[] arguments)
        {
#if USE_SOURCE_WRITER
            _sourceWriter.WriteLine(text, arguments);
#endif
#if USE_SYMBOL_WRITER
            _sourceLine++;
#endif
        }
// ReSharper restore UnusedParameter.Local
// ReSharper restore MemberCanBeMadeStatic.Local

        private Type CreateProxyType()
        {
            lock( SyncRoot )
            {
                if( !AssemblyBuilders.TryGetValue(CreateProxyAssemblyName(), out _assemblyBuilder) )
                {
                    var assemblyName = new AssemblyName(CreateProxyAssemblyName());
#if DEBUG
                    assemblyName.Flags |= AssemblyNameFlags.EnableJITcompileOptimizer;
                    assemblyName.Flags |= AssemblyNameFlags.EnableJITcompileTracking;
#endif

                    _assemblyBuilder = AppDomain.CurrentDomain.DefineDynamicAssembly(
                        assemblyName,
                        AssemblyBuilderAccess.RunAndSave);

                    AssemblyBuilders.Add(CreateProxyAssemblyName(), _assemblyBuilder);
                    ApplyDebuggableAttributes();
                }
#if DEBUG
                _moduleBuilder = _assemblyBuilder.DefineDynamicModule(CreateProxyModuleName(), CreateProxyModuleFileName(), true);
#else
                _moduleBuilder = _assemblyBuilder.GetDynamicModule(CreateProxyModuleName());
                if( _moduleBuilder == null )
                {
                    _moduleBuilder = _assemblyBuilder.DefineDynamicModule(CreateProxyModuleName(), CreateProxyModuleFileName(), true);
                }
#endif
            }

#if USE_SYMBOL_WRITER
            _symDocument = _moduleBuilder.DefineDocument(
                CreateProxyTypeName() + ".cs",
                SymLanguageType.CSharp,
                SymLanguageVendor.Microsoft,
                SymDocumentType.Text);
            _symWriter = _moduleBuilder.GetSymWriter();
#endif

#if USE_SOURCE_WRITER
            _sourceWriter = new StreamWriter(CreateProxyTypeName() + ".cs");
#endif
            WriteSourceLine("using System;");
            WriteSourceLine("using NanoRpc.Net;");
            WriteSourceLine(string.Empty);

            _proxyClassBuilder = _moduleBuilder.DefineType(
                CreateProxyTypeName(),
                TypeAttributes.Class | TypeAttributes.Public,
                typeof( object ),
                new[] { _interfaceType });

            WriteSourceLine("public class {0} : {1}", _proxyClassBuilder.Name, _interfaceType.Name);
            WriteSourceLine("{");

            CreateFields();
            GenerateConstructors();
            BuildDestructor();
            GenerateProperties();
            GenerateMethods();

            WriteSourceLine("}");

#if USE_SOURCE_WRITER
            _sourceWriter.Close();
#endif

            var proxyType = _proxyClassBuilder.CreateType();

#if DEBUG
            _assemblyBuilder.Save(CreateProxyAssemblyFileName());
#endif

            return proxyType;
        }

        /// <summary>
        /// Marks the generated code as debuggable. 
        /// </summary>
        /// <remarks>
        /// See http://blogs.msdn.com/rmbyers/archive/2005/06/26/432922.aspx for explanation.
        /// </remarks>
        [Conditional("USE_SYMBOL_WRITER")]
        private void ApplyDebuggableAttributes()
        {
            var debuggableAttributeType = typeof( DebuggableAttribute );
            var debuggableAttributeContructor =
                debuggableAttributeType.GetConstructor(
                    new[] { typeof( DebuggableAttribute.DebuggingModes ) });

            const DebuggableAttribute.DebuggingModes debuggableAttributeFlags =
                DebuggableAttribute.DebuggingModes.DisableOptimizations |
                DebuggableAttribute.DebuggingModes.Default;

            var customAttributeBuilder = new CustomAttributeBuilder(
                debuggableAttributeContructor,
                new object[] { debuggableAttributeFlags });
            _assemblyBuilder.SetCustomAttribute(customAttributeBuilder);
        }

        private string CreateProxyAssemblyName()
        {
#if DEBUG
            return _interfaceType.FullName + ProxyAssemblyNameSuffix;
#else
            return _interfaceType.Namespace + ProxyAssemblyNameSuffix;
#endif
        }

#if DEBUG
        private string CreateProxyAssemblyFileName()
        {
            return _interfaceType.FullName + ProxyAssemblyFileNameSuffix;
        }
#endif

        private string CreateProxyModuleName()
        {
            return _interfaceType.FullName + ProxyModuleNameSuffix;
        }

        private string CreateProxyModuleFileName()
        {
            return _interfaceType.FullName + ProxyModuleFileNameSuffix;
        }

        private string CreateProxyTypeName()
        {
            return _interfaceType.FullName + ProxyTypeSuffix;
        }

        private void CreateFields()
        {
            if( _isEventSource )
            {
                _senderFieldBuilder = _proxyClassBuilder.DefineField(
                    "_sender",
                    typeof( IRpcMessageSender ),
                    FieldAttributes.Private);

                WriteSourceLine("\tprivate IRpcMessageSender _sender;");
            }
            else
            {
                _clientFieldBuilder = _proxyClassBuilder.DefineField(
                    "_client",
                    typeof( IRpcClient ),
                    FieldAttributes.Private);

                _objectIdFieldBuilder = _proxyClassBuilder.DefineField(
                    "_objectId",
                    typeof( uint ),
                    FieldAttributes.Private);

                WriteSourceLine("\tprivate IRpcClient _client;");
                WriteSourceLine("\tprivate uint _objectId;");
            }
        }

        private void GenerateConstructors()
        {
            if( _isEventSource )
            {
                GenerateEventSourceProxyConstructor();
            }
            else
            {
                GenerateClientProxyConstructor();
                GenerateTransientObjectProxyConstructor();
            }
        }

        /// <summary>
        /// Generates constructor used for service proxies.
        /// </summary>
        /// <remarks>
        /// The generated constructor has the following declaration: 
        /// <c>.ctor( IRpcClient client ).</c>
        /// </remarks>
        private void GenerateClientProxyConstructor()
        {
            var constructorParameterTypes = new[] { _clientFieldBuilder.FieldType };
            var constructorBuilder = _proxyClassBuilder.DefineConstructor(
                MethodAttributes.Public,
                CallingConventions.Standard,
                constructorParameterTypes);

            constructorBuilder.DefineParameter(1, ParameterAttributes.None, "client");
            ILGenerator ilcode = constructorBuilder.GetILGenerator();

            WriteSourceLine("\t{0}( IRpcClient client )", _proxyClassBuilder.Name);

            // Call default constructor
            WriteSourceLine("\t{");
            MarkSequencePoint(ilcode);

            ilcode.Emit(OpCodes.Ldarg_0);
            ilcode.Emit(OpCodes.Call, typeof( object ).GetConstructor(Type.EmptyTypes));

            // Initialize the field
            WriteSourceLine("\t\t_client = client;");
            MarkSequencePoint(ilcode);

            ilcode.Emit(OpCodes.Ldarg_0);
            ilcode.Emit(OpCodes.Ldarg_1);
            ilcode.Emit(OpCodes.Stfld, _clientFieldBuilder);

            WriteSourceLine("\t}");
            MarkSequencePoint(ilcode);

            ilcode.Emit(OpCodes.Ret);
        }

        /// <summary>
        /// Generates constructor used for event source proxies.
        /// </summary>
        /// <remarks>
        /// The generated constructor has the following declaration: 
        /// <c>.ctor( IRpcMessageSender sender ).</c>
        /// </remarks>
        private void GenerateEventSourceProxyConstructor()
        {
            var constructorParameterTypes = new[] { _senderFieldBuilder.FieldType };
            var constructorBuilder = _proxyClassBuilder.DefineConstructor(
                MethodAttributes.Public,
                CallingConventions.Standard,
                constructorParameterTypes);

            constructorBuilder.DefineParameter(1, ParameterAttributes.None, "sender");
            ILGenerator ilcode = constructorBuilder.GetILGenerator();

            WriteSourceLine("\t{0}( IRpcMessageSender sender )", _proxyClassBuilder.Name);

            // Call default constructor
            WriteSourceLine("\t{");
            MarkSequencePoint(ilcode);

            ilcode.Emit(OpCodes.Ldarg_0);
            ilcode.Emit(OpCodes.Call, typeof( object ).GetConstructor(Type.EmptyTypes));

            // Initialize the field
            WriteSourceLine("\t\t_sender = sender;");
            MarkSequencePoint(ilcode);

            ilcode.Emit(OpCodes.Ldarg_0);
            ilcode.Emit(OpCodes.Ldarg_1);
            ilcode.Emit(OpCodes.Stfld, _senderFieldBuilder);

            WriteSourceLine("\t}");
            MarkSequencePoint(ilcode);

            ilcode.Emit(OpCodes.Ret);
        }

        /// <summary>
        /// Generates constructor used for transient object proxies.
        /// </summary>
        /// <remarks>
        /// The generated constructor has the following declaration: 
        /// <c>.ctor( IRpcClient client, uint objectId ).</c>
        /// </remarks>
        private void GenerateTransientObjectProxyConstructor()
        {
            var constructorParameterTypes = new[]
                                                {
                                                    _clientFieldBuilder.FieldType,
                                                    typeof( uint )
                                                };

            var constructorBuilder = _proxyClassBuilder.DefineConstructor(
                MethodAttributes.Public,
                CallingConventions.Standard,
                constructorParameterTypes);

            constructorBuilder.DefineParameter(1, ParameterAttributes.None, "client");
            constructorBuilder.DefineParameter(2, ParameterAttributes.None, "objectId");
            ILGenerator ilcode = constructorBuilder.GetILGenerator();

            WriteSourceLine("\t{0}( IRpcClient client, uint objectId )", _proxyClassBuilder.Name);

            // Call default constructor
            WriteSourceLine("\t{");
            MarkSequencePoint(ilcode);

            ilcode.Emit(OpCodes.Ldarg_0);
            ilcode.Emit(OpCodes.Call, typeof( object ).GetConstructor(Type.EmptyTypes));

            // Initialize the field
            WriteSourceLine("\t\t_client = client;");
            WriteSourceLine("\t\t_objectId = objectId;");
            MarkSequencePoint(ilcode);

            ilcode.Emit(OpCodes.Ldarg_0);
            ilcode.Emit(OpCodes.Ldarg_1);
            ilcode.Emit(OpCodes.Stfld, _clientFieldBuilder);
            ilcode.Emit(OpCodes.Ldarg_0);
            ilcode.Emit(OpCodes.Ldarg_2);
            ilcode.Emit(OpCodes.Stfld, _objectIdFieldBuilder);

            WriteSourceLine("\t}");
            MarkSequencePoint(ilcode);

            ilcode.Emit(OpCodes.Ret);
        }

        /// <summary>
        /// Creates C# destructor for normal proxies.
        /// </summary>
        /// <remarks>
        /// The generated IL code corresponds to what C# compiler generates for a class
        /// destructor.
        /// </remarks>
        private void BuildDestructor()
        {
            if( !_isEventSource )
            {
                var destructorBuilder = _proxyClassBuilder.DefineMethod(
                    "Finalize",
                    MethodAttributes.Family | MethodAttributes.HideBySig | MethodAttributes.Virtual,
                    CallingConventions.Standard);

                var ilcode = destructorBuilder.GetILGenerator();

                WriteSourceLine("\t~{0}()", _proxyClassBuilder.Name);
                WriteSourceLine("\t{");

                // try {
                ilcode.BeginExceptionBlock();

                WriteSourceLine("\t\tRpcProxyHelpers.SendDeleteMessage( _objectId, _client );");
                MarkSequencePoint(ilcode);

                // RpcProxyHelpers.SendDeleteMessage( _objectId, _client );
                ilcode.Emit(OpCodes.Ldarg_0);
                ilcode.Emit(OpCodes.Ldfld, _objectIdFieldBuilder);
                ilcode.Emit(OpCodes.Ldarg_0);
                ilcode.Emit(OpCodes.Ldfld, _clientFieldBuilder);
                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.SendDeleteMessageMethodInfo);

                // }
                // finally {
                ilcode.BeginFinallyBlock();

                // Here we assume that proxy is derived from System.Object for the purpose
                // of caching System.Object.Finalize() method information.
                // If implementation of the proxy changes such that proxy base class changes,
                // then appropriate base class Finalize method must be called.
                Debug.Assert(
                    _proxyClassBuilder.BaseType == typeof( object ), "Proxy class is not derived from System.Object.");

                // base.Finalize();
                ilcode.Emit(OpCodes.Ldarg_0);
                ilcode.Emit(OpCodes.Call, ObjectFinalizeMethodInfo);
                ilcode.EndExceptionBlock();

                WriteSourceLine("\t}");
                MarkSequencePoint(ilcode);

                ilcode.Emit(OpCodes.Ret);

                // }
            }
        }

        /// <summary>
        /// Implements properties of the interface.
        /// </summary>
        private void GenerateProperties()
        {
            var properties = _interfaceType.FindMembers(
                MemberTypes.Property,
                BindingFlags.Instance | BindingFlags.Public,
                (info, criteria) => true,
                null);

            if( _isEventSource )
            {
                if( properties.Length != 0 )
                {
                    throw new Exception("Event source interface must not have properties.");
                }
            }

            // At the CLR level there is no such thing as property.
            // Property is a language specific feature around
            // pair of methods.
            // In this implementation we implement all methods in
            // GenerateMethods.
            // If for whatever reason we need to distinguish
            // properties and methods, then in GenerateMethods
            // FindMembers must be called with custom binding
            // expression that would exclude members with 
            // MethodAttributes.SpecialName attribute.
#if false
            else
            {
                foreach( PropertyInfo propertyInfo in properties )
                {
                    BuildProperty(propertyInfo);
                }
            }
#endif
        }

#if false
        private void BuildProperty(PropertyInfo propertyInfo)
        {
            var propertyBuilder = _proxyClassBuilder.DefineProperty(
                propertyInfo.Name,
                PropertyAttributes.None,
                propertyInfo.PropertyType,
                null);

            var methodInfo = propertyInfo.GetGetMethod();
            if( methodInfo != null )
            {
                propertyBuilder.SetGetMethod(BuildMethod(methodInfo));
            }

            methodInfo = propertyInfo.GetSetMethod();
            if( methodInfo != null )
            {
                propertyBuilder.SetSetMethod(BuildMethod(methodInfo));
            }
        }
#endif

        /// <summary>
        /// Generates all methods of the interface.
        /// </summary>
        /// <exception cref="Exception">
        /// The source inteface contains method that returns value.
        /// </exception>
        private void GenerateMethods()
        {
            var methods = _interfaceType.FindMembers(
                MemberTypes.Method,
                BindingFlags.Instance | BindingFlags.Public,
                (info, criteria) => true,
                null);

            foreach( MethodInfo methodInfo in methods )
            {
                if( _isEventSource && methodInfo.ReturnType != typeof( void ) )
                {
                    throw new Exception("Source interface method must not return value.");
                }

                BuildMethod(methodInfo);
            }
        }

        /// <summary>
        /// Generates IL code that implements a single method of the interface.
        /// </summary>
        /// <param name="interfaceMethodInfo">
        /// The method to implement.
        /// </param>
        /// <returns>
        /// Built method.
        /// </returns>
        private void BuildMethod(MethodInfo interfaceMethodInfo)
        {
            // Define method signature
            ParameterInfo[] interfaceMethodParameters = interfaceMethodInfo.GetParameters();

            Type[] proxyMethodParameterTypes = (from pi in interfaceMethodParameters
                                                select pi.ParameterType).ToArray();

            var methodBuilder = _proxyClassBuilder.DefineMethod(
                interfaceMethodInfo.Name,
                MethodAttributes.Public | MethodAttributes.Virtual,
                CallingConventions.HasThis,
                interfaceMethodInfo.ReturnType,
                proxyMethodParameterTypes);

#if USE_SYMBOL_WRITER
            _symWriter.OpenMethod(new SymbolToken(methodBuilder.GetToken().Token));
#endif

            WriteSourceLine(
                "\tpublic {0} {1}({2})",
                TypeToCSharpTypeString( interfaceMethodInfo.ReturnType ),
                interfaceMethodInfo.Name,
                string.Join(",", (from p in interfaceMethodParameters select TypeToCSharpTypeString( p.ParameterType ) + " " + p.Name).ToArray()));
            WriteSourceLine("\t{");

            // Define method body
            ILGenerator ilcode = methodBuilder.GetILGenerator();

            // RpcCall.Builder callBuilder;
            LocalBuilder callBuilderVariable = ilcode.DeclareLocal(typeof( RpcCall.Builder ));
            callBuilderVariable.SetLocalSymInfo("callBuilder");

            WriteSourceLine("\t\tRpcCall.Builder callBuilder;");

            if( _isEventSource )
            {
                // callBuilder = RpcProxyHelpers.CreateServiceRpcCallBuilder( "interfaceName", "methodName" );
                WriteSourceLine(
                    "\t\t\tcallBuilder = RpcProxyHelpers.CreateServiceRpcCallBuilder( \"{0}\", \"{1}\" );",
                    _interfaceType.FullName,
                    interfaceMethodInfo.Name);
                MarkSequencePoint(ilcode);

                ilcode.Emit(OpCodes.Ldstr, _interfaceType.FullName);
                ilcode.Emit(OpCodes.Ldstr, interfaceMethodInfo.Name);
                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.CreateServiceRpcCallBuilderMethodInfo);
                ilcode.Emit(OpCodes.Stloc, callBuilderVariable);
            }
            else
            {
                // if( _objectId == 0 ) {
                WriteSourceLine("\t\tif( _objectId == 0 ) {");
                MarkSequencePoint(ilcode);

                var objectIdNonZeroLabel = ilcode.DefineLabel();
                ilcode.Emit(OpCodes.Ldarg_0);
                ilcode.Emit(OpCodes.Ldfld, _objectIdFieldBuilder);
                ilcode.Emit(OpCodes.Ldc_I4_0);
                ilcode.Emit(OpCodes.Bne_Un, objectIdNonZeroLabel);

                WriteSourceLine(
                    "\t\t\tcallBuilder = RpcProxyHelpers.CreateServiceRpcCallBuilder( \"{0}\", \"{1}\" );",
                    _interfaceType.FullName,
                    interfaceMethodInfo.Name);
                MarkSequencePoint(ilcode);

                // callBuilder = RpcProxyHelpers.CreateServiceRpcCallBuilder( "interfaceName", "methodName" );
                ilcode.Emit(OpCodes.Ldstr, _interfaceType.FullName);
                ilcode.Emit(OpCodes.Ldstr, interfaceMethodInfo.Name);
                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.CreateServiceRpcCallBuilderMethodInfo);
                ilcode.Emit(OpCodes.Stloc, callBuilderVariable);
                var endOfConditionLabel = ilcode.DefineLabel();
                ilcode.Emit(OpCodes.Br, endOfConditionLabel);

                // } else {
                WriteSourceLine("\t\t}");
                WriteSourceLine("\t\telse {");
                WriteSourceLine(
                    "\t\t\tcallBuilder = RpcProxyHelpers.CreateObjectRpcCallBuilder( _objectId, \"{0}\" );",
                    interfaceMethodInfo.Name);
                MarkSequencePoint(ilcode);

                ilcode.MarkLabel(objectIdNonZeroLabel);

                // callBuilder = RpcProxyHelpers.CreateObjectRpcCallBuilder( _objectId, "methodName" );
                ilcode.Emit(OpCodes.Ldarg_0);
                ilcode.Emit(OpCodes.Ldfld, _objectIdFieldBuilder);
                ilcode.Emit(OpCodes.Ldstr, interfaceMethodInfo.Name);
                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.CreateObjectRpcCallBuilderMethodInfo);
                ilcode.Emit(OpCodes.Stloc, callBuilderVariable);

                // }
                WriteSourceLine("\t\t}");

                ilcode.MarkLabel(endOfConditionLabel);
            }

            WriteSourceLine(string.Empty);

            // Initialize callBuilder with method parameters.
            // Serialize each parameter and add to the RpcCall (builder).
            foreach( var parameterInfo in interfaceMethodParameters )
            {
                methodBuilder.DefineParameter(parameterInfo.Position + 1, parameterInfo.Attributes, parameterInfo.Name);

                WriteSourceLine("\t\tvar {0}Parameter = RpcProxyHelper.CreateRpcParameterBuilder( {0} );", parameterInfo.Name);
                MarkSequencePoint(ilcode);

                // Push call builder instance (for AddParameters call).
                ilcode.Emit(OpCodes.Ldloc, callBuilderVariable);

                // var temp = RpcProxyHelper.CreateRpcParameterBuilder( <method_parameter> );
                ilcode.Emit(OpCodes.Ldarg, parameterInfo.Position + 1);
                BuildMethodParameterSerializer(ilcode, parameterInfo.ParameterType);

                WriteSourceLine("\t\tcallBuilder.AddParameters( {0}Parameter );", parameterInfo.Name);
                MarkSequencePoint(ilcode);

                // (void)callBuilder.AddParameters( temp );
                ilcode.Emit(OpCodes.Call, RpcCallBuilderAddParametersMethodInfo);
                ilcode.Emit(OpCodes.Pop); // Ignore returned value.
            }

            WriteSourceLine("");

            // RpcMessage rpcMessage = RpcProxyHelpers.BuildRpcMessage( callBuilder );
            WriteSourceLine("\t\tRpcMessage rpcMessage = RpcProxyHelpers.BuildRpcMessage( callBuilder );");
            MarkSequencePoint(ilcode);

            LocalBuilder rpcMessageVariable = ilcode.DeclareLocal(typeof( RpcMessage ));
            rpcMessageVariable.SetLocalSymInfo("rpcMessage");
            ilcode.Emit(OpCodes.Ldloc, callBuilderVariable);
            ilcode.Emit(OpCodes.Call, RpcProxyHelpers.BuildRpcMessageMethodInfo);
            ilcode.Emit(OpCodes.Stloc, rpcMessageVariable);

            if( _isEventSource )
            {
                // _sender.Send( rpcMessage );
                WriteSourceLine("\t\t_sender.Send( rpcMessage );");
                MarkSequencePoint(ilcode);

                ilcode.Emit(OpCodes.Ldarg_0);
                ilcode.Emit(OpCodes.Ldfld, _senderFieldBuilder);
                ilcode.Emit(OpCodes.Ldloc, rpcMessageVariable);
                ilcode.Emit(OpCodes.Callvirt, RpcMessageSenderSendMethodInfo);
            }
            else
            {
                // var rpcResult = _client.SendWithResult( rpcMessage );
                // rpcResult is not stored in a variable, but stays on the stack
                WriteSourceLine("\t\tvar rpcResult = _client.SendWithResult( rpcMessage );");
                MarkSequencePoint(ilcode);

                ilcode.Emit(OpCodes.Ldarg_0);
                ilcode.Emit(OpCodes.Ldfld, _clientFieldBuilder);
                ilcode.Emit(OpCodes.Ldloc, rpcMessageVariable);
                ilcode.Emit(OpCodes.Callvirt, RpcClientSendWithResultMethodInfo);

                // RpcResult object is on the stack. Duplicate for the next condition and branching.
                ilcode.Emit(OpCodes.Dup);

                // if( rpcResult == null ) {
                WriteSourceLine("\t\tif( rpcResult == null ) {");
                MarkSequencePoint(ilcode);

                var jmp = ilcode.DefineLabel();
                ilcode.Emit(OpCodes.Dup);
                ilcode.Emit(OpCodes.Ldnull);
                ilcode.Emit(OpCodes.Ceq);
                ilcode.Emit(OpCodes.Brfalse, jmp);

                // throw new NullReferenceException( "SendWithResult method returned null result." );
                WriteSourceLine(
                    "\t\t\tthrow new NullReferenceException( \"SendWithResult method returned null result.\" );");
                MarkSequencePoint(ilcode);

                ilcode.Emit(OpCodes.Ldstr, "SendWithResult method returned null result.");
                ilcode.Emit(OpCodes.Newobj, typeof( NullReferenceException ).GetConstructor(new[] { typeof( string ) }));
                ilcode.Emit(OpCodes.Throw);

                // }
                WriteSourceLine("\t\t}");
                MarkSequencePoint(ilcode);

                ilcode.MarkLabel(jmp);

                var returnValueLabel = ilcode.DefineLabel();

                // Check if the call succeeded.
                // if( rpcResult.Status != RpcStatus.RpcSucceeded ) {
                WriteSourceLine("\t\tif( rpcResult.Status != RpcStatus.RpcSucceeded ) {");
                MarkSequencePoint(ilcode);

                ilcode.Emit(OpCodes.Call, typeof( RpcResult ).GetProperty("Status").GetGetMethod());
                ilcode.Emit(OpCodes.Ldc_I4, (int) RpcStatus.RpcSucceeded);
                ilcode.Emit(OpCodes.Beq, returnValueLabel);

                // throw RpcException( rpcResult.Status );
                WriteSourceLine("\t\t\tthrow RpcException( rpcResult.Status );");
                MarkSequencePoint(ilcode);

                ilcode.Emit(OpCodes.Call, typeof( RpcResult ).GetProperty("Status").GetGetMethod());
                ilcode.Emit(OpCodes.Newobj, typeof( RpcException ).GetConstructor(new[] { typeof( RpcStatus ) }));
                ilcode.Emit(OpCodes.Throw); // We return from here due to exception
                // }
                // End of if statement
                WriteSourceLine("\t\t}");
                MarkSequencePoint(ilcode);

                ilcode.MarkLabel(returnValueLabel);

                // rpcResult is on the stack
                WriteSourceLine("\t\tvar rpcParameter = rpcResult.CallResult;");
                MarkSequencePoint(ilcode);

                // var rpcParameter = rpcResult.CallResult;
                // rpcParameter is not stored in a variable, but stays on the stack
                ilcode.Emit(OpCodes.Call, typeof( RpcResult ).GetProperty("CallResult").GetGetMethod());

                // return rpcParameter.<value property>;

                // Source code is generated inside GenerateReturnValue, because
                // it is not trivial for certain types.
                GenerateReturnValue(ilcode, interfaceMethodInfo.ReturnType, true, "rpcParameter");
            }

            ilcode.Emit(OpCodes.Ret);

            WriteSourceLine("\t}");

#if USE_SYMBOL_WRITER
            _symWriter.CloseMethod();
#endif

            return;
        }

        /// <summary>
        /// Generates IL code that serializes method parameters into 
        /// <see cref="RpcParameter"/>.
        /// </summary>
        /// <remarks>
        /// <para>
        /// The generated IL code expects a value to be serialized on the 
        /// stack. This value is a first argument for one of
        /// <c>RpcProxyHelpers.CreateRpcParameterBuilder</c> method overloads.
        /// </para>
        /// <para>
        /// After <c>RpcProxyHelpers.CreateRpcParameterBuilder</c> method called
        /// the <see cref="RpcParameter.Builder"/> object containing the 
        /// serialized value is placed on the stack.
        /// </para>
        /// <para>
        /// The method generates the following C# equivalent:
        /// <code>
        /// RpcProxyHelpers.CreateRpcParameterBuilder( &lt;value on the stack&gt; );
        /// </code>
        /// </para>
        /// </remarks>
        /// <param name="ilcode">
        /// ILGenerator instance for the method.
        /// </param>
        /// <param name="parameterType">
        /// Method argument type.
        /// </param>
        private void BuildMethodParameterSerializer(ILGenerator ilcode, Type parameterType)
        {
            // RpcProxyHelpers.CreateRpcParameterBuilder( parameter );
            if( typeof( IMessage ).IsAssignableFrom(parameterType) )
            {
                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.GetMethod("CreateRpcParameterBuilder", typeof( IMessage )));
            }
            else if( parameterType == typeof( bool ) )
            {
                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.GetMethod("CreateRpcParameterBuilder", typeof( bool )));
            }
            else if( parameterType == typeof( int ) )
            {
                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.GetMethod("CreateRpcParameterBuilder", typeof( int )));
            }
            else if( parameterType == typeof( long ) )
            {
                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.GetMethod("CreateRpcParameterBuilder", typeof( long )));
            }
            else if( parameterType == typeof( double ) )
            {
                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.GetMethod("CreateRpcParameterBuilder", typeof( double )));
            }
            else if( parameterType == typeof( string ) )
            {
                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.GetMethod("CreateRpcParameterBuilder", typeof( string )));
            }
            else if( parameterType.IsEnum )
            {
                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.GetMethod("CreateRpcParameterBuilder", typeof( int )));
            }
            else
            {
                throw new Exception("Unsupported parameter type");
            }
        }

        /// <summary>
        /// Generates IL code that retrieves value from <see cref="RpcParameter"/>.
        /// </summary>
        /// <remarks>
        /// <para>
        /// The generated IL code expects <see cref="RpcParameter"/> object
        /// on the stack. The value retrieved from the parameter, converted to 
        /// the appropriate client type and placed on the stack.
        /// </para>
        /// <para>
        /// If the generated code is the last before return instruction, then
        /// called may ask to generate tail call when appropriate.
        /// </para>
        /// <para>
        /// The actual generated code dependes on nature of returned value.
        /// The method generates the following C# approximate equivalent:
        /// <code>
        /// &lt;rpcParameterObject&gt;.&lt;type&gt;Value;
        /// </code>
        /// </para>
        /// <para>
        /// It seems that in .NET 3.5 the code generator is smart enough to emit tailcall automatically.
        /// See http://connect.microsoft.com/VisualStudio/feedback/details/166013/c-compiler-should-optimize-tail-calls,
        /// http://blogs.msdn.com/b/clrcodegeneration/archive/2010/05/07/jit-etw-tail-call-event-fail-reasons.aspx
        /// </para>
        /// </remarks>
        /// <param name="ilcode">
        /// Method code generator.
        /// </param>
        /// <param name="returnType">
        /// The expected type.
        /// </param>
        /// <param name="emitTailcall">
        /// If true then emit tail call if possible.
        /// </param>
        /// <param name="objectSymbolName">
        /// Symbolic name of object on the stack.
        /// </param>
        /// <exception cref="Exception">
        /// The type specified in <paramref name="returnType"/> is not supported.
        /// </exception>
        private void GenerateReturnValue(
            ILGenerator ilcode, Type returnType, bool emitTailcall, string objectSymbolName)
        {
            if( returnType == typeof( void ) )
            {
                WriteSourceLine("\t\treturn;");
                MarkSequencePoint(ilcode);

                ilcode.Emit(OpCodes.Pop); // Pop rpcParameter
                ilcode.Emit(OpCodes.Ret);
            }
            else if( returnType == typeof( bool ) )
            {
                WriteSourceLine("\t\treturn {0}.BoolValue;", objectSymbolName);
                MarkSequencePoint(ilcode);

                if( emitTailcall )
                {
                    ilcode.Emit(OpCodes.Tailcall);
                }

                ilcode.Emit(OpCodes.Call, typeof( RpcParameter ).GetProperty("BoolValue").GetGetMethod());
            }
            else if( returnType == typeof( int ) )
            {
                WriteSourceLine("\t\treturn {0}.Int32Value;", objectSymbolName);
                MarkSequencePoint(ilcode);

                if( emitTailcall )
                {
                    ilcode.Emit(OpCodes.Tailcall);
                }

                ilcode.Emit(OpCodes.Call, typeof( RpcParameter ).GetProperty("Int32Value").GetGetMethod());
            }
            else if( returnType == typeof( long ) )
            {
                WriteSourceLine("\t\treturn {0}.Int64Value;", objectSymbolName);
                MarkSequencePoint(ilcode);

                if( emitTailcall )
                {
                    ilcode.Emit(OpCodes.Tailcall);
                }

                ilcode.Emit(OpCodes.Call, typeof( RpcParameter ).GetProperty("Int64Value").GetGetMethod());
            }
            else if( returnType == typeof( double ) )
            {
                WriteSourceLine("\t\treturn {0}.DoubleValue;", objectSymbolName);
                MarkSequencePoint(ilcode);

                if( emitTailcall )
                {
                    ilcode.Emit(OpCodes.Tailcall);
                }

                ilcode.Emit(OpCodes.Call, typeof( RpcParameter ).GetProperty("DoubleValue").GetGetMethod());
            }
            else if( returnType == typeof( string ) )
            {
                WriteSourceLine("\t\treturn {0}.StringValue;", objectSymbolName);
                MarkSequencePoint(ilcode);

                if( emitTailcall )
                {
                    ilcode.Emit(OpCodes.Tailcall);
                }

                ilcode.Emit(OpCodes.Call, typeof( RpcParameter ).GetProperty("StringValue").GetGetMethod());
            }
            else if( returnType.IsEnum )
            {
                WriteSourceLine("\t\treturn ({1}) {0}.Int32Value;", objectSymbolName, returnType.FullName);
                MarkSequencePoint(ilcode);

                LocalBuilder enumVariable = ilcode.DeclareLocal(returnType);
                ilcode.Emit(OpCodes.Call, typeof( RpcParameter ).GetProperty("Int32Value").GetGetMethod());

                // store - load sequence converts integer to enumeration.
                ilcode.Emit(OpCodes.Stloc, enumVariable);
                ilcode.Emit(OpCodes.Ldloc, enumVariable);
            }
            else if( returnType.IsInterface && !_isEventSource )
            {
                WriteSourceLine("\t\tuint objectId = {0}.ObjectIdValue;", objectSymbolName);
                MarkSequencePoint(ilcode);

                var objectIdVariable = ilcode.DeclareLocal(typeof( uint ));
                objectIdVariable.SetLocalSymInfo("objectId");
                ilcode.Emit(OpCodes.Call, typeof( RpcParameter ).GetProperty("ObjectIdValue").GetGetMethod());
                ilcode.Emit(OpCodes.Stloc, objectIdVariable);

                // Load arguments of RpcProxyHelpers.BuildProxy's arguments.
                // Note that we can avoid objectIdVariable if we reorder parameters in BuildProxy method.
                WriteSourceLine(
                    "\t\treturn RpcProxyHelpers.BuildProxyMethod( _client, typeof( {0} ), objectId );",
                    returnType.FullName);
                MarkSequencePoint(ilcode);

                // this._client
                ilcode.Emit(OpCodes.Ldarg_0);
                ilcode.Emit(OpCodes.Ldfld, _clientFieldBuilder);

                // typeof( <interface type> )
                ilcode.Emit(OpCodes.Ldtoken, returnType);
                ilcode.Emit(
                    OpCodes.Call,
                    typeof( Type ).GetMethod("GetTypeFromHandle", new[] { typeof( RuntimeTypeHandle ) }));

                // objectId
                ilcode.Emit(OpCodes.Ldloc, objectIdVariable);

                if( emitTailcall )
                {
                    ilcode.Emit(OpCodes.Tailcall);
                }

                ilcode.Emit(OpCodes.Call, RpcProxyHelpers.BuildProxyMethodInfo);
            }
            else if( typeof( IMessage ).IsAssignableFrom(returnType) )
            {
                WriteSourceLine("\t\treturn {0}.ParseFrom( {1}.ProtoValue );", returnType.FullName, objectSymbolName);
                MarkSequencePoint(ilcode);

                ilcode.Emit(OpCodes.Call, typeof( RpcParameter ).GetProperty("ProtoValue").GetGetMethod());

                if( emitTailcall )
                {
                    ilcode.Emit(OpCodes.Tailcall);
                }

                ilcode.Emit(OpCodes.Call, returnType.GetMethod("ParseFrom", new[] { typeof( ByteString ) }));
            }
            else
            {
                throw new Exception("Unsupported return type");
            }
        }

        [Conditional("USE_SYMBOL_WRITER")]
        private void MarkSequencePoint(ILGenerator ilcode)
        {
#if USE_SYMBOL_WRITER
            ilcode.MarkSequencePoint(_symDocument, _sourceLine, 1, _sourceLine, int.MaxValue);
#endif
        }
    }
}

