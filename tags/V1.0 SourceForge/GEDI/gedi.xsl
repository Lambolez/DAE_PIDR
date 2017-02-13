<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" encoding="UTF-8" indent="yes"/> 
 
 <xsl:template match="/">
	<metadata>
		<xsl:apply-templates select="/GEDI/DL_DOCUMENT/DL_PAGE"/>
	</metadata>
  </xsl:template>
  
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

  <xsl:template match="/GEDI/USER">	
		<contributor_list_item>
			<id><xsl:value-of select="@name" /></id>
			<name><xsl:value-of select="@name" /></name>
		</contributor_list_item>
  </xsl:template>
  
  <xsl:template match="/GEDI/DL_DOCUMENT/DL_PAGE">
    <page_image>
      <id><xsl:value-of select="@pageID" /></id>
	  <width><xsl:value-of select="@width" /></width>
	  <height><xsl:value-of select="@height" /></height>
	  <contributor_list>
		<xsl:apply-templates select="/GEDI/USER"/>
	  </contributor_list>
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
		<xsl:apply-templates select="./DL_ZONE"/>
	  </page_element_list>
    </page_image>
  </xsl:template>
 
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
			<type_list>
				<type_list_item>
					<id><xsl:value-of select="@gedi_type" /></id>
					<name>name: <xsl:value-of select="@gedi_type" /></name>
					<description>This is the <xsl:value-of select="@gedi_type" /> datatype.</description>
				</type_list_item>
			</type_list>
			<value_list>
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
