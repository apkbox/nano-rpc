// <summary> 
// Provides XML document loading, validation and caching.
// </summary>

// TODO: Check namespace mismatch in imported documents. The imported documents are allowed not to have a namespace.

namespace IdlCompiler
{
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Xml;
    using System.Xml.Schema;
    using Odgs.Common;

    /// <summary>
    /// Provides XML document loading, validation and caching.
    /// </summary>
    internal class CachingDocumentLoader
    {
        private readonly Dictionary<string, XmlDocument> _documentCache = new Dictionary<string, XmlDocument>();
        private readonly Dictionary<string, XmlSchema> _schemaCache = new Dictionary<string, XmlSchema>();

        private const string XmlidlNamespaceName = "urn:odgs-oce-net:schemas-nano-rpc-idl";

        public void AddSchema(string schemaPath)
        {
            XmlSchema schema;

            if( !_schemaCache.TryGetValue(schemaPath, out schema) )
            {
                var schemaReader = new XmlTextReader(schemaPath);
                schema = XmlSchema.Read(schemaReader, OnDocumentValidation);
                _schemaCache.Add(schemaPath, schema);
            }
        }

        /// <summary>
        /// Reads XML document and validates against specified schema.
        /// </summary>
        /// <param name="documentPath">
        /// Path to the XML document.
        /// </param>
        /// <returns>
        /// Document object if successful.
        /// </returns>
        /// <remarks>
        /// Both document and schema are cached. The returned document object is cloned from cache.
        /// </remarks>
        public XmlDocument LoadAndValidate(string documentPath)
        {
            var imports = new HashSet<string>();
            imports.Add(documentPath);
            return LoadAndValidate(documentPath, imports);
        }

        private XmlDocument LoadAndValidate(string documentPath, HashSet<string> imports)
        {
            XmlDocument document;

            if( !_documentCache.TryGetValue(documentPath, out document) )
            {
                document = new XmlDocument();

                foreach( var schema in _schemaCache.Values )
                    document.Schemas.Add(schema);

                document.Load(documentPath);
                ProcessImports(documentPath, document, imports);
                document.Validate(OnDocumentValidation);
                _documentCache.Add(documentPath, document);
            }

            return document.Clone() as XmlDocument;
        }

        private void ProcessImports(string documentPath, XmlDocument xmlDocument, HashSet<string> imports)
        {
            var ns = new XmlNamespaceManager(new NameTable());
            ns.AddNamespace("d", XmlidlNamespaceName);

            var importNodes = xmlDocument.SelectNodes("/d:idl/d:import", ns);
            if( importNodes != null )
            {
                List<string> importFiles = new List<string>();

                for( int i = 0; i < importNodes.Count; i++ )
                {
                    var importNode = ((XmlElement) importNodes[i]);
                    var src = importNode.GetAttribute("src");
                    importFiles.Add(src);
                    importNode.ParentNode.RemoveChild(importNode);
                }

                string documentBasePath = Path.GetDirectoryName(documentPath);
                if( documentBasePath == null )
                    documentBasePath = ".";

                foreach( var importFile in importFiles )
                {
                    var importFileName = Path.Combine(documentBasePath, importFile);
                    if( !imports.Contains(importFileName) )
                    {
                        XmlDocument xmlIncludeDocument = LoadAndValidate(
                            importFileName, imports);
                        MergeDocuments(xmlDocument, xmlIncludeDocument);
                        imports.Add(importFileName);
                    }
                    else
                    {
                        Logger.LogWarning("'{0}' file already imported. Skipping.", importFileName);
                    }
                }
            }
        }

        private void MergeDocuments(XmlDocument xmlDocument, XmlDocument xmlIncludeDocument)
        {
            XmlElement idlNode = xmlDocument.DocumentElement;
            Debug.Assert(idlNode != null);

            var ns = new XmlNamespaceManager(new NameTable());
            ns.AddNamespace("d", XmlidlNamespaceName);

            var enumerationsNode = idlNode.SelectSingleNode("d:enumerations", ns);
            var typesNode = idlNode.SelectSingleNode("d:types", ns);
            var interfacesNode = idlNode.SelectSingleNode("d:interfaces", ns);

            if( enumerationsNode == null )
            {
                enumerationsNode = xmlDocument.CreateElement("d:enumerations", ns.LookupNamespace("d"));
                idlNode.PrependChild(enumerationsNode);
            }

            if( typesNode == null )
            {
                typesNode = xmlDocument.CreateElement("d:types", ns.LookupNamespace("d"));
                idlNode.InsertAfter(typesNode, enumerationsNode);
            }

            if( interfacesNode == null )
            {
                interfacesNode = xmlDocument.CreateElement("d:interfaces", ns.LookupNamespace("d"));
                idlNode.InsertAfter(interfacesNode, typesNode);
            }

            ImportSelectedNodes(xmlIncludeDocument, enumerationsNode, "d:idl/d:enumerations/d:enum", ns);
            ImportSelectedNodes(xmlIncludeDocument, typesNode, "d:idl/d:types/d:type", ns);
            ImportSelectedNodes(xmlIncludeDocument, interfacesNode, "d:idl/d:interfaces/d:interface", ns);
        }

        private static void ImportSelectedNodes(XmlDocument sourceDocument, XmlNode targetNode, string xpathSelector,
                                                XmlNamespaceManager ns)
        {
            Debug.Assert(targetNode != null);
            Debug.Assert(targetNode.OwnerDocument != null);

            var sourceNodes = sourceDocument.SelectNodes(xpathSelector, ns);
            if( sourceNodes != null )
            {
                foreach( XmlNode node in sourceNodes )
                {
                    var importedNode = targetNode.OwnerDocument.ImportNode(node, true);
                    targetNode.AppendChild(importedNode);
                }
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

