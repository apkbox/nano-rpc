<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet 
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		version="1.0" 
		xml:space="default">

	<xsl:template name="map_type">
		<xsl:param name="type_to_map" select="@type" />
		
		<xsl:choose>
			<!-- Check if no type specified (could be return type) -->
			<xsl:when test="string-length( $type_to_map ) = 0">
				<!-- TODO: <xsl:when test="string-length( $typemap ) != 0" > -->
				<xsl:variable name="type_entry" select="$typemap/types/type[@name='void']" />
				<xsl:choose>
					<!-- Check is type is in the map -->
					<xsl:when test="count( $type_entry ) = 0">
						<xsl:value-of select="$type_to_map" />
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="$type_entry/@mapped" />
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			
			<!-- Type is specified and require translation -->
			<xsl:otherwise>
				<xsl:choose>
					<!-- Check is type map exists -->
					<xsl:when test="string-length( $typemap ) != 0" >
						<xsl:variable name="type_entry" select="$typemap/types/type[@name=$type_to_map]" />
						<xsl:choose>
							<!-- Check is type is in the map -->
							<xsl:when test="count( $type_entry ) = 0">
								<xsl:value-of select="$type_to_map" />
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select="$type_entry/@mapped" />
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>

					<!-- Stick the type without translation -->
					<xsl:otherwise>
						<!-- TODO: Shout out a warning? -->
						<xsl:text>&lt;UNKNOWN_TYPE&gt;</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
		
	</xsl:template>
	
</xsl:stylesheet>

