<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" indent="yes"/>

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

<!-- The transform template for textGt element which is the root of the parse tree.-->
<xsl:template match="/textGt">
  <page_image>
    <id><xsl:value-of select="@image_name"/></id>
    <width><xsl:value-of select="@width"/></width>
    <height><xsl:value-of select="@height"/></height>
	<!-- For each textLine element under the textGT, use the template to transform-->
    <page_element_list>
      <xsl:apply-templates select="textLine">
	<xsl:with-param name="pos" select="position()"/>
	<xsl:with-param name="image_name" select="@image_name"/>
      </xsl:apply-templates>
    </page_element_list>
  </page_image>  
</xsl:template>

<!-- The transform template for word element.-->
<xsl:template match="word">
  <xsl:param name="pos"/>
  <xsl:param name="textLine_name"/>
  <page_element_list_item>
    <id><xsl:value-of select="$textLine_name"/>-word-<xsl:value-of select="@id" /></id>
    <topleftx><xsl:value-of select="@Xmin"/></topleftx>
    <toplefty><xsl:value-of select="@Ymin"/></toplefty>
    <width><xsl:value-of select="number(@Xmax)-number(@Xmin)"/></width>
    <height><xsl:value-of select="number(@Ymax)-number(@Ymin)"/></height>
    <value_list>
		<!-- Transform textLine_name as a user-defined property.-->
		<value_list_item>
			<id/>
			<value><xsl:value-of select="$textLine_name"/></value>
			<property>
				<id>CVC-Page-Element-Included</id>
				<name>CVC-Page-Element-Included</name>
				<description>CVC-Page-Element-Included</description>
			</property>
		</value_list_item>
		<!-- Transform Transcription as a user-defined property.-->
      <value_list_item>
	<id/>
	<value><xsl:value-of select="@Transcription"/></value>
	<property>
	  <id>CVC-Transcription</id>
	  <name>CVC-Transcription</name>
	  <description>CVC-Transcription</description>
	  <type_list>
	    <type_list_item>
	      <id>CVC-Page-Element</id>
	      <name>CVC-Page-Element</name>
	      <description>CVC-Page-Element</description>
	    </type_list_item>
	  </type_list>
	</property>
      </value_list_item>
    </value_list>
  </page_element_list_item>
	<!-- For each atom element under the word element, use the template to transform-->
  <xsl:apply-templates select="atom">
    <xsl:with-param name="pos" select="position()"/>
    <xsl:with-param name="word_name" select="concat($textLine_name, concat('-word-', @id))"/>
  </xsl:apply-templates>
</xsl:template>

<!-- The transform template for cc element.-->
<xsl:template match="cc">
  <xsl:param name="pos"/>
  <xsl:param name="atom_name"/>
  <page_element_list_item>
    <id><xsl:value-of select="$atom_name"/>-cc-<xsl:value-of select="@id" /></id>
    <value_list>
	<value_list_item>
		<id/>
		<value><xsl:value-of select="$atom_name"/></value>
		<property>
			<id>CVC-Page-Element-Included</id>
			<name>CVC-Page-Element-Included</name>
			<description>CVC-Page-Element-Included</description>
		</property>
	</value_list_item>
      <value_list_item>
	<id/>
	<value><xsl:value-of select="@perimeterLength"/></value>
	<property>
	  <id>CVC-perimeterLength</id>
	  <name>CVC-perimeterLength</name>
	  <description>CVC-perimeterLength</description>
	  <type_list>
	    <type_list_item>
	      <id>CVC-cc</id>
	      <name>CVC-cc</name>
	      <description>CVC-cc</description>
	    </type_list_item>
	  </type_list>
	</property>
      </value_list_item>
	<!-- For each xpoints, ypoints element under the area element and the skeleton element, use each template to transform-->
      <xsl:apply-templates select="area/xpoints"/>
      <xsl:apply-templates select="area/ypoints"/>
      <xsl:apply-templates select="skeleton/xpoints"/>
      <xsl:apply-templates select="skeleton/ypoints"/>
    </value_list>
  </page_element_list_item>
</xsl:template>

<!-- The transform template for xpoints element whose path on the parse tree is skeleton/xpoints.-->
<xsl:template match="skeleton/xpoints">
  <value_list_item>
    <id/>
    <value><xsl:value-of select="text()"/></value>
	 <!-- Transform xpoints as a user-defined property.-->
    <property>
      <id>CVC-cc-skeleton-xpoints</id>
      <name>CVC-cc-skeleton-xpoints</name>
      <description>CVC-cc-skeleton-xpoints</description>
      <type_list>
	<type_list_item>
	  <id>CVC-cc</id>
	  <name>CVC-cc</name>
	  <description>CVC-cc</description>
	</type_list_item>
      </type_list>
    </property>
  </value_list_item>
</xsl:template>

<!-- The transform template for ypoints element whose path on the parse tree is skeleton/ypoints.-->
<xsl:template match="skeleton/ypoints">
  <value_list_item>
    <id/>
    <value><xsl:value-of select="text()"/></value>
	 <!-- Transform ypoints as a user-defined property.-->
    <property>
      <id>CVC-cc-skeleton-ypoints</id>
      <name>CVC-cc-skeleton-ypoints</name>
      <description>CVC-cc-skeleton-ypoints</description>
      <type_list>
	<type_list_item>
	  <id>CVC-cc</id>
	  <name>CVC-cc</name>
	  <description>CVC-cc</description>
	</type_list_item>
      </type_list>
    </property>
  </value_list_item>
</xsl:template>

<!-- The transform template for xpoints element whose path on the parse tree is area/xpoints.-->
<xsl:template match="area/xpoints">
  <value_list_item>
    <id/>
	 <!-- Transform xpoints as a user-defined property.-->
    <value><xsl:value-of select="text()"/></value>
    <property>
      <id>CVC-cc-area-xpoints</id>
      <name>CVC-cc-area-xpoints</name>
      <description>CVC-cc-area-xpoints</description>
      <type_list>
	<type_list_item>
	  <id>CVC-cc</id>
	  <name>CVC-cc</name>
	  <description>CVC-cc</description>
	</type_list_item>
      </type_list>
    </property>
  </value_list_item>
</xsl:template>

<!-- The transform template for ypoints element whose path on the parse tree is area/ypoints.-->
<xsl:template match="area/ypoints">
  <value_list_item>
    <id/>
	 <!-- Transform ypoints as a user-defined property.-->
    <value><xsl:value-of select="text()"/></value>
    <property>
      <id>CVC-cc-area-ypoints</id>
      <name>CVC-cc-area-ypoints</name>
      <description>CVC-cc-area-ypoints</description>
      <type_list>
	<type_list_item>
	  <id>CVC-cc</id>
	  <name>CVC-cc</name>
	  <description>CVC-cc</description>
	</type_list_item>
      </type_list>
    </property>
  </value_list_item>
</xsl:template>

<!-- The transform template for cc element.-->
<xsl:template match="atom">
  <xsl:param name="pos"/>
  <xsl:param name="word_name"/>
  <page_element_list_item>
    <id><xsl:value-of select="$word_name"/>-atom-<xsl:value-of select="@id" /></id>
    <value_list>
	<value_list_item>
			<id/>
			<value><xsl:value-of select="$word_name"/></value>
			<property>
				<id>CVC-Page-Element-Included</id>
				<name>CVC-Page-Element-Included</name>
				<description>CVC-Page-Element-Included</description>
			</property>
		</value_list_item>
      <value_list_item>
	<id/>
	<!-- Transform Transcription as a user-defined property.-->
	<value><xsl:value-of select="@Transcription"/></value>
	<property>
	  <id>CVC-Transcription</id>
	  <name>CVC-Transcription</name>
	  <description>CVC-Transcription</description>
	  <type_list>
	    <type_list_item>
	      <id>CVC-Page-Element</id>
	      <name>CVC-Page-Element</name>
	      <description>CVC-Page-Element</description>
	    </type_list_item>
	  </type_list>
	</property>
      </value_list_item>
      <value_list_item>
	<id/>
	<!-- Transform SplitByDesign as a user-defined property.-->
	<value><xsl:value-of select="@SplitByDesign"/></value>
	<property>
	  <id>CVC-SplitByDesign</id>
	  <name>CVC-SplitByDesign</name>
	  <description>CVC-SplitByDesign</description>
	  <type_list>
	    <type_list_item>
	      <id>CVC-atom</id>
	      <name>CVC-atom</name>
	      <description>CVC-atom</description>
	    </type_list_item>
	  </type_list>
	</property>
      </value_list_item>
      <value_list_item>
	<id/>
	<!-- Transform MergedByDesign as a user-defined property.-->
	<value><xsl:value-of select="@MergedByDesign"/></value>
	<property>
	  <id>CVC-MergedByDesign</id>
	  <name>CVC-MergedByDesign</name>
	  <description>CVC-MergedByDesign</description>
	  <type_list>
	    <type_list_item>
	      <id>CVC-atom</id>
	      <name>CVC-atom</name>
	      <description>CVC-atom</description>
	    </type_list_item>
	  </type_list>
	</property>
      </value_list_item>
  </value_list>
  </page_element_list_item>
	<!-- For each cc element under the atom element, use the template to transform-->
  <xsl:apply-templates select="cc">
    <xsl:with-param name="pos" select="position()"/>
    <xsl:with-param name="atom_name" select="concat($word_name, concat('-atom-', @id))"/>
  </xsl:apply-templates>
</xsl:template>

<!-- The transform template for textLine element.-->
<xsl:template match="textLine">
  <xsl:param name="pos"/>
  <xsl:param name="image_name"/>
  <page_element_list_item>
    <id><xsl:value-of select="$image_name"/>-textLine-<xsl:value-of select="@id" /></id>
    <topleftx><xsl:value-of select="@Xmin"/></topleftx>
    <toplefty><xsl:value-of select="@Ymin"/></toplefty>
	<!-- Compute the width and height based on Xmax, Xmin, Ymax and Ymin.-->
    <width><xsl:value-of select="number(@Xmax)-number(@Xmin)"/></width>
    <height><xsl:value-of select="number(@Ymax)-number(@Ymin)"/></height>
    <value_list>
      <value_list_item>
	<id/>
	<!-- Transform Transcription as a user-defined property.-->
	<value><xsl:value-of select="@Transcription"/></value>
	<property>
	  <id>CVC-Transcription</id>
	  <name>CVC-Transcription</name>
	  <description>CVC-Transcription</description>
	  <type_list>
	    <type_list_item>
	      <id>CVC-Page-Element</id>
	      <name>CVC-Page-Element</name>
	      <description>CVC-Page-Element</description>
	    </type_list_item>
	  </type_list>
	</property>
      </value_list_item>
    </value_list>
  </page_element_list_item>
	<!-- For each word element under the textline element, use the template to transform-->
  <xsl:apply-templates select="word">
    <xsl:with-param name="pos" select="position()"/>
    <xsl:with-param name="textLine_name" select="concat($image_name, concat('-textLine-', @id))"/>
  </xsl:apply-templates>
</xsl:template>
  
</xsl:stylesheet>
