<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
				version="1.0"
				xml:space="default"
				xmlns:xmlidl="urn:odgs-oce-net:schemas-nano-rpc-idl" >
	<xsl:param name="_guard_name" />
	<xsl:param name="_template_name" />
	<xsl:param name="_document_name" />
	<xsl:param name="_output_stub_header" />

	<xsl:output method="text" encoding="utf-8" />

	<xsl:variable name="typemap" select="document('cpp-types.xml')" />
	<xsl:include href="common.xsl" />

	<xsl:template match="/xmlidl:idl">
		<xsl:text>// Generated by the IdlCompiler. DO NOT EDIT!&#10;</xsl:text>
		<xsl:text>// source: </xsl:text><xsl:value-of select="$_document_name"/><xsl:text>&#10;</xsl:text>
		<xsl:text>// template: </xsl:text><xsl:value-of select="$_template_name"/><xsl:text>&#10;</xsl:text>

		<xsl:text>&#10;</xsl:text>

		<xsl:text><![CDATA[#include "]]></xsl:text>
		<xsl:value-of select="$_output_stub_header" />
		<xsl:text><![CDATA["]]></xsl:text>
		<xsl:text>&#10;&#10;</xsl:text>
		<xsl:text><![CDATA[#include <exception>]]>&#10;</xsl:text>
		<xsl:text><![CDATA[#include <string>]]>&#10;</xsl:text>
		<xsl:text>&#10;&#10;</xsl:text>

		<xsl:if test="boolean(@namespace)">
			<xsl:text>namespace </xsl:text>
			<xsl:value-of select="@namespace" />
			<xsl:text> {&#10;&#10;</xsl:text>
		</xsl:if>
		
		<xsl:apply-templates select="xmlidl:interfaces/xmlidl:interface" />

		<xsl:if test="boolean(@namespace)">
			<xsl:text>} // namespace&#10;&#10;</xsl:text>
		</xsl:if>
		
		<xsl:text>&#10;&#10;</xsl:text>
	</xsl:template>

	<xsl:template match="xmlidl:interface" >
		<xsl:text><![CDATA[const char *]]></xsl:text>
		<xsl:value-of select="@name" />
		<xsl:text>_Stub::</xsl:text>
		<xsl:text><![CDATA[GetInterfaceName() const]]></xsl:text>
		<xsl:text>&#10;{&#10;&#09;return &quot;</xsl:text>
		
		<xsl:if test="boolean(../../@namespace)">
			<xsl:value-of select="../../@namespace" />
		</xsl:if>
		<xsl:text>.</xsl:text>
		<xsl:value-of select="@name"/>

		<xsl:text>&quot;;&#10;}&#10;&#10;&#10;</xsl:text>

		<xsl:text><![CDATA[void ]]></xsl:text>
		<xsl:value-of select="@name" />
		<xsl:text>_Stub::</xsl:text>
		<xsl:text><![CDATA[CallMethod( const NanoRpc::RpcCall &rpc_call, NanoRpc::RpcResult *rpc_result )]]></xsl:text>
		<xsl:text>&#10;{&#10;</xsl:text>

		<xsl:apply-templates select="xmlidl:property | xmlidl:method" mode="generate_call_dispatcher"/>
		
		<xsl:text>&#10;&#09;return;&#10;}&#10;&#10;&#10;</xsl:text>
	</xsl:template>

	<xsl:template match="xmlidl:property" mode="generate_call_dispatcher">
		<xsl:text>&#09;</xsl:text>
		<xsl:choose>
			<xsl:when test="position() = 1" >
				<xsl:text>if</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>else if</xsl:text>
			</xsl:otherwise>
		</xsl:choose>

		<xsl:text>( rpc_call.method() == &quot;get_</xsl:text>
		<xsl:value-of select="@name" />
		<xsl:text>&quot; ) {&#10;</xsl:text>

		<xsl:apply-templates select="." mode="get" />

		<xsl:text>&#09;}&#10;</xsl:text>
		<!-- 
			This is a not schema-aware processor, so we cannot rely on datatype of 
			attributes and elements declared in schema!
		-->
		<xsl:if test="count(@readonly)=0 or @readonly='false' or @readonly='no' or @readonly='0'" >
			<xsl:text>&#09;</xsl:text>
			<xsl:text>else if( rpc_call.method() == &quot;set_</xsl:text>
			<xsl:value-of select="@name" />
			<xsl:text>&quot; ) {&#10;</xsl:text>

			<xsl:apply-templates select="." mode="set" />

			<xsl:text>&#09;}&#10;</xsl:text>
		</xsl:if>
	</xsl:template>

	<xsl:template match="xmlidl:property" mode="get" >
		<xsl:call-template name="declare_return_variable"/>

		<xsl:call-template name="result_assignment_for_simple_type"/>
		<xsl:text>impl_->get_</xsl:text>
		<xsl:value-of select="@name" />
		<xsl:text>(</xsl:text>
		<xsl:call-template name="generate_call_out_argument" />
		<xsl:text>);&#10;</xsl:text>

		<xsl:call-template name="serialize_return_value" />
	</xsl:template>

	<xsl:template match="xmlidl:property" mode="set" >
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="map_type"/><xsl:text> value;&#10;</xsl:text>

		<xsl:call-template name="deserialize_argument" >
			<xsl:with-param name="name" select="'value'"/>
		</xsl:call-template>
		
		<xsl:text>&#09;&#09;impl_->set_</xsl:text>
		<xsl:value-of select="@name" />
		<xsl:text>( value );&#10;</xsl:text>
	</xsl:template>

	<xsl:template match="xmlidl:method" mode="generate_call_dispatcher">
		<xsl:text>&#09;</xsl:text>
		<xsl:choose>
			<xsl:when test="position() = 1" >
				<xsl:text>if</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>else if</xsl:text>
			</xsl:otherwise>
		</xsl:choose>

		<xsl:text>( rpc_call.method() == &quot;</xsl:text>
		<xsl:value-of select="@name" />
		<xsl:text>&quot; ) {&#10;</xsl:text>

		<xsl:apply-templates select="." />

		<xsl:text>&#09;}&#10;</xsl:text>
	</xsl:template>

	<xsl:template match="xmlidl:method" >
		<xsl:apply-templates select="xmlidl:arguments" mode="declare_temp_variables" />
		<xsl:apply-templates select="xmlidl:returns" mode="declare_return_variable"/>
		<xsl:apply-templates select="xmlidl:arguments" mode="deserialize" />

		<xsl:choose>
			<xsl:when test="count( xmlidl:returns ) != 0">
				<xsl:call-template name="result_assignment_for_simple_type" >
					<xsl:with-param name="type" select="xmlidl:returns/@type"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>&#09;&#09;</xsl:text>
			</xsl:otherwise>
		</xsl:choose>

		<xsl:text>impl_-></xsl:text>
		<xsl:value-of select="@name" />
		<xsl:text>(</xsl:text>
		<xsl:apply-templates select="xmlidl:arguments" mode="generate_call_arguments" />
		<xsl:call-template name="generate_method_call_out_argument" />
		<xsl:text>);&#10;</xsl:text>

		<xsl:apply-templates select="xmlidl:returns" mode="serialize" />
	</xsl:template>

	<xsl:template match="xmlidl:argument" mode="declare_temp_variables">
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="map_type"/><xsl:text> </xsl:text><xsl:value-of select="@name"/>
		<xsl:text>;&#10;</xsl:text>
	</xsl:template>

	<xsl:template match="xmlidl:returns" mode="declare_return_variable">
		<xsl:call-template name="declare_return_variable"/>
	</xsl:template>

	<xsl:template match="xmlidl:argument" mode="deserialize">
		<xsl:call-template name="deserialize_argument" />
	</xsl:template>

	<xsl:template name="result_assignment_for_simple_type" >
		<xsl:param name="type" select="@type" />
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:if test="$type='bool' or $type='int' or $type='long' or $type='double' or
				count( /xmlidl:idl/xmlidl:enumerations/xmlidl:enum[@name=$type] ) != 0 or
				count( /xmlidl:idl/xmlidl:interfaces/xmlidl:interface[@name=$type] ) != 0" >
			<xsl:text>result = </xsl:text>
		</xsl:if>
	</xsl:template>

	<xsl:template match="xmlidl:argument" mode="generate_call_arguments" >
		<xsl:value-of select="@name"/>
		<xsl:if test="position() != last()">
			<xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>

	<xsl:template name="generate_method_call_out_argument" >
		<xsl:if test="count( xmlidl:returns ) != 0" >
			<xsl:call-template name="generate_call_out_argument">
				<xsl:with-param name="type" select="xmlidl:returns/@type" />
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<xsl:template name="generate_call_out_argument" >
		<xsl:param name="type" select="@type" />
		<xsl:if test="not( $type='bool' or $type='int' or $type='long' or $type='double' or 
						count( /xmlidl:idl/xmlidl:enumerations/xmlidl:enum[@name=$type] ) != 0 or
						count( /xmlidl:idl/xmlidl:interfaces/xmlidl:interface[@name=$type] ) != 0 )">
			<xsl:if test="count( xmlidl:arguments/xmlidl:argument ) != 0">
				<xsl:text>, </xsl:text>
			</xsl:if>
			<xsl:text><![CDATA[&result]]></xsl:text>
		</xsl:if>
	</xsl:template>

	<xsl:template match="xmlidl:returns" mode="serialize" >
		<xsl:call-template name="serialize_return_value" />
	</xsl:template>

	<xsl:template name="declare_return_variable">
		<xsl:variable name="type" select="@type"/>
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:choose>
			<xsl:when test="count( /xmlidl:idl/xmlidl:interfaces/xmlidl:interface[@name=$type] ) != 0">
				<xsl:value-of select="$type"/>
				<xsl:text> *</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:call-template name="map_type" />
				<xsl:text> </xsl:text>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:text>result;&#10;</xsl:text>
	</xsl:template>

	<xsl:template name="deserialize_argument">
		<xsl:param name="arg_index" select="position() - 1" />
		<xsl:param name="type" select="@type" />
		<xsl:param name="name" select="@name" />
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:choose>
			<xsl:when test="$type='bool'" >
				<xsl:value-of select="$name"/>
				<xsl:text> = </xsl:text>
				<xsl:text><![CDATA[rpc_call.parameters().Get(]]></xsl:text>
				<xsl:value-of select="$arg_index"/>
				<xsl:text><![CDATA[).bool_value();]]></xsl:text>
			</xsl:when>
			<xsl:when test="$type='int'" >
				<xsl:value-of select="$name"/>
				<xsl:text> = </xsl:text>
				<xsl:text><![CDATA[rpc_call.parameters().Get(]]></xsl:text>
				<xsl:value-of select="$arg_index"/>
				<xsl:text><![CDATA[).int32_value();]]></xsl:text>
			</xsl:when>
			<xsl:when test="$type='long'" >
				<xsl:value-of select="$name"/>
				<xsl:text> = </xsl:text>
				<xsl:text><![CDATA[rpc_call.parameters().Get(]]></xsl:text>
				<xsl:value-of select="$arg_index"/>
				<xsl:text><![CDATA[).int64_value();]]></xsl:text>
			</xsl:when>
			<xsl:when test="$type='double'" >
				<xsl:value-of select="$name"/>
				<xsl:text> = </xsl:text>
				<xsl:text><![CDATA[rpc_call.parameters().Get(]]></xsl:text>
				<xsl:value-of select="$arg_index"/>
				<xsl:text><![CDATA[).double_value();]]></xsl:text>
			</xsl:when>
			<xsl:when test="$type='string'" >
				<xsl:text><![CDATA[NanoRpc::Utf8ToWideString( rpc_call.parameters().Get(]]></xsl:text>
				<xsl:value-of select="$arg_index"/>
				<xsl:text><![CDATA[).string_value(), &]]></xsl:text>
				<xsl:value-of select="$name"/>
				<xsl:text><![CDATA[);]]></xsl:text>
			</xsl:when>
			<xsl:when test="count( /xmlidl:idl/xmlidl:enumerations/xmlidl:enum[@name=$type] ) != 0">
				<xsl:value-of select="$name"/>
				<xsl:text> = </xsl:text>
				<xsl:text>static_cast&lt;</xsl:text>
				<xsl:value-of select="$type"/>
				<xsl:text>&gt;( </xsl:text>
				<xsl:text><![CDATA[rpc_call.parameters().Get(]]></xsl:text>
				<xsl:value-of select="$arg_index"/>
				<xsl:text><![CDATA[).int32_value() );]]></xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$name"/>
				<xsl:text><![CDATA[.ParseFromString( rpc_call.parameters().Get( ]]></xsl:text>
				<xsl:value-of select="$arg_index"/>
				<xsl:text><![CDATA[ ).proto_value() );]]></xsl:text>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:text>&#10;</xsl:text>
	</xsl:template>

	<xsl:template name="serialize_return_value" >
		<xsl:variable name="type" select="@type" />
		<xsl:choose>
			<xsl:when test="$type='bool'" >
				<xsl:text>&#09;&#09;</xsl:text>
				<xsl:text>rpc_result->mutable_call_result()->set_bool_value( result );</xsl:text>
			</xsl:when>
			<xsl:when test="$type='int'" >
				<xsl:text>&#09;&#09;</xsl:text>
				<xsl:text>rpc_result->mutable_call_result()->set_int32_value( result );</xsl:text>
			</xsl:when>
			<xsl:when test="$type='long'" >
				<xsl:text>&#09;&#09;</xsl:text>
				<xsl:text>rpc_result->mutable_call_result()->set_int64_value( result );</xsl:text>
			</xsl:when>
			<xsl:when test="$type='double'" >
				<xsl:text>&#09;&#09;</xsl:text>
				<xsl:text>rpc_result->mutable_call_result()->set_double_value( result );</xsl:text>
			</xsl:when>
			<xsl:when test="$type='string'" >
				<xsl:text>&#09;&#09;</xsl:text>
				<xsl:text>NanoRpc::WideToUtf8String( result, rpc_result->mutable_call_result()->mutable_string_value() );</xsl:text>
			</xsl:when>
			<xsl:when test="count( /xmlidl:idl/xmlidl:enumerations/xmlidl:enum[@name=$type] ) != 0" >
				<xsl:text>&#09;&#09;</xsl:text>
				<xsl:text>rpc_result->mutable_call_result()->set_int32_value( result );</xsl:text>
			</xsl:when>
			<xsl:when test="count( /xmlidl:idl/xmlidl:interfaces/xmlidl:interface[@name=$type] ) != 0" >
				<xsl:text>&#09;&#09;NanoRpc::RpcObjectId object_id = object_manager_->RegisterInstance( new </xsl:text>
				<xsl:value-of select="$type"/>
				<xsl:text>_Stub( object_manager_, result ) );&#10;</xsl:text>
				<xsl:text>&#09;&#09;rpc_result->mutable_call_result()->set_object_id_value( object_id );&#10;</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>&#09;&#09;</xsl:text>
				<xsl:text>std::string str;</xsl:text>
				<xsl:text>&#10;&#09;&#09;</xsl:text>
				<xsl:text><![CDATA[result.SerializeToString( &str );]]></xsl:text>
				<xsl:text>&#10;&#09;&#09;</xsl:text>
				<xsl:text>rpc_result->mutable_call_result()->set_proto_value( str );</xsl:text>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:text>&#10;</xsl:text>
	</xsl:template>

</xsl:stylesheet>

