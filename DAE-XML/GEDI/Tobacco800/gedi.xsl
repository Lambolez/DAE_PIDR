<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" indent="yes"/>

<xsl:variable name="alpha"/>

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
      <id><xsl:value-of select="$str"/></id>
      <name><xsl:value-of select="$str"/></name>
      <description><xsl:value-of select="$str"/></description>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- The transform template for USER element whose path on the parse tree is /GEDI/USER.-->
<xsl:template match="/GEDI/USER">	
  <contributor_list_item>
  <id><xsl:value-of select="@name" /></id>
  </contributor_list_item>
</xsl:template>

<!-- The transform template for DL_DOCUMENT element whose path on the parse tree is /GEDI/DL_DOCUMENT.-->
<xsl:template match="/GEDI/DL_DOCUMENT">
  <!-- For each DL_PAGE element whose path on the parse tree is /GEDI/DL_DOCUMENT/DL_PAGE, use the template to transform-->
  <xsl:apply-templates select="./DL_PAGE">
    <xsl:with-param name="str" select="position()"/>
  </xsl:apply-templates>
</xsl:template>

<!-- The transform template for DL_PAGE element.-->
<xsl:template match="DL_PAGE">
  <xsl:param name="str"/>
  <xsl:variable name="imagename" select="@src"/>
  <page_image>
    <id><xsl:value-of select="$imagename"/>-<xsl:value-of select="@pageID" /></id>
    <xsl:choose>
	<!-- According to image's name to give different path.-->
    <xsl:when test="starts-with($imagename, 'a')='true'">
      <path>tobacco800/a/<xsl:value-of select="@src" /></path>
    </xsl:when>
    <xsl:when test="starts-with($imagename, 'b')='true'">
      <path>tobacco800/b/<xsl:value-of select="@src" /></path>
    </xsl:when>
    <xsl:when test="starts-with($imagename, 'c')='true'">
      <path>tobacco800/c/<xsl:value-of select="@src" /></path>
    </xsl:when>
    <xsl:otherwise>
      <path>tobacco800/more/<xsl:value-of select="@src" /></path>
    </xsl:otherwise>
    </xsl:choose>
    <width><xsl:value-of select="@width" /></width>
    <height><xsl:value-of select="@height" /></height>
    <contributor_list>
	  <!-- For each USER element whose path on the parse tree is /GEDI/USER, use the template to transform-->
      <xsl:apply-templates select="/GEDI/USER"/>
    </contributor_list>
    <type_list>
      <type_list_item>
	<id><xsl:value-of select="@gedi_type" /></id>
	<name><xsl:value-of select="@gedi_type" /></name>
	<description>This is the <xsl:value-of select="@gedi_type" /> datatype.</description>
      </type_list_item>
      <type_list_item>
	<xsl:call-template name="getExtension">
	  <xsl:with-param name="str" select="@src"/>
	  <xsl:with-param name="deli" select="'.'"/>
	  </xsl:call-template>
      </type_list_item>
    </type_list>
    <!--<value_list>
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
    </value_list>-->
    <image_dataset_list>
      <image_dataset_list_item>
		<!-- According to image's name to give different dataset id.-->
      <xsl:choose>
        <xsl:when test="starts-with($imagename, 'a')='true'">
          <id>tobacco800/a</id>
        </xsl:when>
        <xsl:when test="starts-with($imagename, 'b')='true'">
          <id>tobacco800/b</id>
        </xsl:when>
        <xsl:when test="starts-with($imagename, 'c')='true'">
          <id>tobacco800/c</id>
        </xsl:when>
        <xsl:otherwise>
          <id>tobacco800/more</id>
        </xsl:otherwise>
      </xsl:choose>
     </image_dataset_list_item>
    </image_dataset_list>
    <page_element_list>
      <xsl:apply-templates select="./DL_ZONE">
	<xsl:with-param name="imagename" select="@src"/>
	<xsl:with-param name="zonecount" select="position()"/>
      </xsl:apply-templates>
    </page_element_list>
  </page_image>
</xsl:template>

<!-- The transform template for DL_ZONE element.-->
<xsl:template match="DL_ZONE">
  <xsl:param name="imagename"/>
  <xsl:param name="zonecount"/>
  <xsl:variable name="zoneid" select="@id" />
  <page_element_list_item>
    <xsl:choose>
      <xsl:when test="$zoneid='None'">
	    <id><xsl:value-of select="$imagename"/>-unnamed-pe-<xsl:value-of select="position()" /></id>
      </xsl:when>
      <xsl:when test="$zoneid=''">
        <id><xsl:value-of select="$imagename"/>-unnamed-pe-<xsl:value-of select="position()" /></id>
      </xsl:when>
      <xsl:otherwise>
	    <id><xsl:value-of select="$imagename"/>-named-pe-<xsl:value-of select="@id" /></id>
      </xsl:otherwise>
    </xsl:choose>
    <topleftx><xsl:value-of select="@col" /></topleftx>
    <toplefty><xsl:value-of select="@row" /></toplefty>
    <width><xsl:value-of select="@width" /></width>
    <height><xsl:value-of select="@height" /></height>
    <type_list>
      <type_list_item>
	    <id>GEDI DL_ZONE</id>
	    <name>GEDI DL_ZONE</name>
	    <description>This is the GEDI DL_ZONE datatype.</description>
      </type_list_item>
    </type_list>
    <value_list>
	  <xsl:variable name="temp0" select="@AuthorID"/>
	  <xsl:choose>	    
        <xsl:when test="$temp0">
          <value_list_item>
			<id/>
			<value><xsl:value-of select="@AuthorID" /></value>
			<property>
			  <id>gedi_type_<xsl:value-of select="@gedi_type" /></id>
			  <name>gedi_type_<xsl:value-of select="@gedi_type" /></name>
			  <description>This property tells the gedi_type property value of a GEDI DL_ZONE.</description>
			  <type_list>
				<type_list_item>
				  <id>GEDI DL_ZONE</id>
				</type_list_item>
			  </type_list>
			</property>
		  </value_list_item>
        </xsl:when>
      </xsl:choose>
      <!-- Transform HandPrint as a user-defined property.-->
	  <xsl:variable name="temp1" select="@HandPrint"/>
	  <xsl:choose>
        <xsl:when test="$temp1">
          <value_list_item>
			<id/>
			<value><xsl:value-of select="@HandPrint" /></value>
			<property>
			  <id>GEDI_HandPrint</id>
			  <name>GEDI HandPrint</name>
			  <description>This property tells the HandPrint property value of a GEDI DL_ZONE.</description>
			  <type_list>
				<type_list_item>
				  <id>GEDI DL_ZONE</id>
				</type_list_item>
			  </type_list>
			</property>
		  </value_list_item>
        </xsl:when>
      </xsl:choose>
      <!-- Transform RLEIMAGE as a user-defined property.-->
	  <xsl:variable name="temp2" select="@RLEIMAGE"/>
	  <xsl:choose>	    
        <xsl:when test="$temp2">
          <value_list_item>
			<id/>
			<value><xsl:value-of select="@RLEIMAGE" /></value>
			<property>
			  <id>GEDI_RLEIMAGE</id>
			  <name>GEDI RLEIMAGE</name>
			  <description>This property gives the RLEIMAGE of a GEDI DL_ZONE.</description>
			  <type_list>
				<type_list_item>
				  <id>GEDI DL_ZONE</id>
				</type_list_item>
			  </type_list>
			</property>
		  </value_list_item>
        </xsl:when>
      </xsl:choose>
      <!-- Transform orientationD as a user-defined property.-->
	  <xsl:variable name="temp3" select="@orientationD"/>
	  <xsl:choose>	    
        <xsl:when test="$temp3">
          <value_list_item>
			<id/>
			<value><xsl:value-of select="@orientationD" /></value>
			<property>
			  <id>GEDI_orientationD</id>
			  <name>GEDI orientationD</name>
			  <description>This property gives the orientationD of a GEDI DL_ZONE.</description>
			  <type_list>
				<type_list_item>
				<id>GEDI DL_ZONE</id>
				</type_list_item>
			  </type_list>
			</property>
		  </value_list_item>
        </xsl:when>
      </xsl:choose>
      <!-- Transform polygon as a user-defined property.-->
	  <xsl:variable name="temp4" select="@polygon"/>
	  <xsl:choose>	    
        <xsl:when test="$temp4">
          <value_list_item>
			<id/>
			<value><xsl:value-of select="@polygon" /></value>
			<property>
			  <id>GEDI_polygon</id>
			  <name>GEDI polygon</name>
			  <description>This property describes the polygon of a GEDI DL_ZONE.</description>
			  <type_list>
				<type_list_item>
				  <id>GEDI DL_ZONE</id>
				  </type_list_item>
			  </type_list>
			</property>
		  </value_list_item>
        </xsl:when>
      </xsl:choose>
    </value_list>
  </page_element_list_item>
</xsl:template>
  
</xsl:stylesheet>