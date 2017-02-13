<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" encoding="UTF-8" indent="yes"/> 
 
 <!-- Find the root entry of the document -->
 <xsl:template match="/">
	<metadata>
		<!-- For each DL_PAGE element whose path on the parse tree is /GEDI/DL_DOCUMENT/DL_PAGE, use the template to transform-->
		<xsl:apply-templates select="/GEDI/DL_DOCUMENT/DL_PAGE"/>
	</metadata>
  </xsl:template>
  
  <!-- The template to split a string according to a delimiter, the parameter str is the string, the parameter deli is the delimiter  -->
  <xsl:template name="split">
	<xsl:param name="str"/>
	<xsl:param name="deli"/>
	<xsl:choose>
		<xsl:when test="contains($str,$deli)">
			<associated_pe_list_item>
				<id><xsl:value-of select="substring-before($str,$deli)"/></id>
			</associated_pe_list_item>
			<xsl:call-template name="split">
				<xsl:with-param name="str" select="substring-after($str,$deli)"/>
				<xsl:with-param name="deli" select="$deli"/>
			</xsl:call-template>
		</xsl:when>
		<xsl:otherwise>
			<associated_pe_list_item>
				<id><xsl:value-of select="$str"/></id>
			</associated_pe_list_item>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<!-- The template to get the extension of a string that is after a delimiter, the parameter str is the string, the parameter deli is the delimiter  -->
<xsl:template name="getExtension">
	<xsl:param name="str"/>
	<xsl:param name="deli"/>
	
	<xsl:choose>
		<xsl:when test="contains($str,$deli)">
			<xsl:call-template name="getExtension">
				<xsl:with-param name="str" select="substring-after($str,$deli)"/>
				<xsl:with-param name="deli" select="$deli"/>
			</xsl:call-template>
		</xsl:when>
		<xsl:otherwise>
			<name><xsl:value-of select="$str"/></name>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

	<!-- The transform template for USER element whose path on the parse tree is /GEDI/USER.-->
  <xsl:template match="/GEDI/USER">	
		<contributor_list_item>
			<id><xsl:value-of select="@name" /></id>
			<name><xsl:value-of select="@name" /></name>
		</contributor_list_item>
  </xsl:template>
  
	<!-- The transform template for DL_PAGE element whose path on the parse tree is /GEDI/DL_DOCUMENT/DL_PAGE.-->
  <xsl:template match="/GEDI/DL_DOCUMENT/DL_PAGE">
    <page_image>
      <id><xsl:value-of select="@pageID" /></id>
	  <width><xsl:value-of select="@width" /></width>
	  <height><xsl:value-of select="@height" /></height>
	  <contributor_list>
		<!-- For each USER element whose path on the parse tree is /GEDI/USER, use the template to transform-->
		<xsl:apply-templates select="/GEDI/USER"/>
	  </contributor_list>
	 <!-- Transform gedi_type as a user-defined property.-->
	  <type_list>
		<type_list_item>
			<id><xsl:value-of select="@gedi_type" /></id>
			<name>name: <xsl:value-of select="@gedi_type" /></name>
			<description>This is the <xsl:value-of select="@gedi_type" /> datatype.</description>
		</type_list_item>
		<type_list_item>
			<id/>
			<xsl:call-template name="getExtension">
				<xsl:with-param name="str" select="@src"/>
				<xsl:with-param name="deli" select="'.'"/>
			</xsl:call-template>
			<!--<description>This is the <xsl:value-of select="@gedi_type" /> datatype.</description>-->
		</type_list_item>
	  </type_list>
	 <!-- Transform English as a user-defined property.-->
	  <value_list>
		<value_list_item>
			<id/>
			<value><xsl:value-of select="@English" /></value>
			<property>
				<id>IsEnglish</id>
				<name>IsEnglish</name>
				<description>This property tells if the language is English or not.</description>
				<type_list>
					<type_list_item>
						<id><xsl:value-of select="@gedi_type" /></id>
					</type_list_item>
				</type_list>
			</property>
		</value_list_item>
	  </value_list>
	  <page_element_list>
		<!-- For each DL_ZONE element under the DL_PAGE element, use the template to transform-->
		<xsl:apply-templates select="./DL_ZONE"/>
	  </page_element_list>
    </page_image>
  </xsl:template>
 
 <!-- The transform template for DL_ZONE element.-->
 <xsl:template match="DL_ZONE">
		<page_element_list_item>
			<id><xsl:value-of select="@id" /></id>
			<width><xsl:value-of select="@width" /></width>
			<height><xsl:value-of select="@height" /></height>
			<polygon><xsl:value-of select="@polygon" /></polygon>
			<associated_pe_list>
				<xsl:call-template name="split">
				<xsl:with-param name="str" select="@elements"/>
				<xsl:with-param name="deli" select="';'"/>
			</xsl:call-template>
			</associated_pe_list>
			 <!-- Transform gedi_type as a user-defined property.-->
			<type_list>
				<type_list_item>
					<id><xsl:value-of select="@gedi_type" /></id>
					<name>name: <xsl:value-of select="@gedi_type" /></name>
					<description>This is the <xsl:value-of select="@gedi_type" /> datatype.</description>
				</type_list_item>
			</type_list>
			<value_list>
				 <!-- Transform col as a user-defined property.-->
				<value_list_item>
					<id/>
					<value><xsl:value-of select="@col" /></value>
					<property>
						<id>GEDI_col</id>
						<name>GEDI col</name>
						<description>This property tells if the number of columns of a GEDI DL_ZONE.</description>
						<type_list>
							<type_list_item>
								<id><xsl:value-of select="@gedi_type" /></id>
							</type_list_item>
						</type_list>
					</property>
				</value_list_item>
				 <!-- Transform row as a user-defined property.-->
				<value_list_item>
					<id/>
					<value><xsl:value-of select="@row" /></value>
					<property>
						<id>GEDI_row</id>
						<name>GEDI row</name>
						<description>This property tells if the number of rows of a GEDI DL_ZONE.</description>
						<type_list>
							<type_list_item>
								<id><xsl:value-of select="@gedi_type" /></id>
							</type_list_item>
						</type_list>
					</property>
				</value_list_item>
				 <!-- Transform HandPrint as a user-defined property.-->
				<value_list_item>
					<id/>
					<value><xsl:value-of select="@HandPrint" /></value>
					<property>
						<id>GEDI_HandPrint</id>
						<name>GEDI HandPrint</name>
						<description>This property tells the HandPrint property value of a GEDI DL_ZONE.</description>
						<type_list>
							<type_list_item>
								<id><xsl:value-of select="@gedi_type" /></id>
							</type_list_item>
						</type_list>
					</property>
				</value_list_item>
				 <!-- Transform RLEIMAGE as a user-defined property.-->
				<value_list_item>
					<id/>
					<value><xsl:value-of select="@RLEIMAGE" /></value>
					<property>
						<id>GEDI_RLEIMAGE</id>
						<name>GEDI RLEIMAGE</name>
						<description>This property gives the RLEIMAGE of a GEDI DL_ZONE.</description>
						<type_list>
							<type_list_item>
								<id><xsl:value-of select="@gedi_type" /></id>
							</type_list_item>
						</type_list>
					</property>
				</value_list_item>
				 <!-- Transform @orientationD as a user-defined property.-->
				<value_list_item>
					<id/>
					<value><xsl:value-of select="@orientationD" /></value>
					<property>
						<id>GEDI_orientationD</id>
						<name>GEDI orientationD</name>
						<description>This property gives the orientationD of a GEDI DL_ZONE.</description>
						<type_list>
							<type_list_item>
								<id><xsl:value-of select="@gedi_type" /></id>
							</type_list_item>
						</type_list>
					</property>
				</value_list_item>
			</value_list>
		</page_element_list_item>
  </xsl:template>
  
</xsl:stylesheet>
