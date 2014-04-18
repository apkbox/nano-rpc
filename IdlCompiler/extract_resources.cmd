if '%1'=='' goto end
if '%2'=='' goto end

mkdir "%2\templates"

copy "%1\src\config\IdlCompiler.xml" "%2\IdlCompiler.xml"
copy "%1\src\schemas\nano-rpc-idl-compiler-config.xsd" "%2\nano-rpc-idl-compiler-config.xsd"

copy "%1\src\schemas\nano-rpc-idl.xsd" "%2"
copy "%1\src\schemas\nano-rpc-idl-pb.xsd" "%2"


copy "%1\src\templates\common.xsl" "%2\templates"
copy "%1\src\templates\cpp-interfaces.xsl" "%2\templates"
copy "%1\src\templates\cpp-proxy-header.xsl" "%2\templates"
copy "%1\src\templates\cpp-proxy-source.xsl" "%2\templates"
copy "%1\src\templates\cpp-stub-header.xsl" "%2\templates"
copy "%1\src\templates\cpp-stub-source.xsl" "%2\templates"
copy "%1\src\templates\cpp-types.xml" "%2\templates"
copy "%1\src\templates\csharp-interfaces.xsl" "%2\templates"
copy "%1\src\templates\csharp-types.xml" "%2\templates"
copy "%1\src\templates\protobuf.xsl" "%2\templates"
copy "%1\src\templates\protobuf-types.xml" "%2\templates"

:end
