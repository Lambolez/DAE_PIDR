<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" indent="yes" omit-xml-declaration="yes"/>

<xsl:template name="split">
  <xsl:param name="str1" />
  <xsl:param name="str2" />
  <xsl:param name="deli" />
  <xsl:param name="result" />
  <xsl:choose>
    <xsl:when test="contains($str1,$deli)">
	(<xsl:value-of select="substring-before($str1,$deli)"/>,<xsl:value-of select="substring-before($str2,$deli)"/>)
	<xsl:call-template name="split">
	<xsl:with-param name="str1" select="substring-after($str1,$deli)"/>
	<xsl:with-param name="str2" select="substring-after($str2,$deli)"/>
	<xsl:with-param name="deli" select="$deli"/>
	</xsl:call-template>
    </xsl:when>    
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
      <id><xsl:value-of select="$str"/></id>
      <name><xsl:value-of select="$str"/></name>
      <description><xsl:value-of select="$str"/></description>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="/textGt">
  <page_image>
    <id><xsl:value-of select="@image_name"/></id>
    <path>cvc_sample_images/<xsl:value-of select="@image_name" /></path>
    <width><xsl:value-of select="@width"/></width>
    <height><xsl:value-of select="@height"/></height>
    <image_dataset_list>
      <image_dataset_list_item>
        <id>cvc_sample_images</id>
      </image_dataset_list_item>
    </image_dataset_list>
    <page_element_list>
      <xsl:apply-templates select="textLine">
	<xsl:with-param name="pos" select="position()"/>
	<xsl:with-param name="image_name" select="@image_name"/>
      </xsl:apply-templates>
    </page_element_list> 
  </page_image>  
</xsl:template>

<xsl:template match="word">
  <xsl:param name="pos"/>
  <xsl:param name="textLine_name"/>
  <page_element_list_item>
    <id><xsl:value-of select="$textLine_name"/>-word-<xsl:value-of select="@id" /></id>
    <topleftx><xsl:value-of select="@Xmin"/></topleftx>
    <toplefty><xsl:value-of select="@Ymin"/></toplefty>
    <width><xsl:value-of select="number(@Xmax)-number(@Xmin)"/></width>
    <height><xsl:value-of select="number(@Ymax)-number(@Ymin)"/></height>
    <type_list>
      <type_list_item>
	<id>cvc word</id>
	<name>cvc word</name>
	<description>This is the cvc word datatype.</description>
      </type_list_item>      
    </type_list>
    <value_list>
      <value_list_item>
	<id/>
	<value replace="true"><xsl:value-of select="$textLine_name"/></value>
	<property>
	  <id>CVC-Page-Element-Included</id>
	  <name>CVC-Page-Element-Included</name>
	  <description>CVC-Page-Element-Included</description>
	</property>
      </value_list_item>
      <value_list_item>
	<id/>
	<value><xsl:value-of select="@Transcription"/></value>
	<property>
	  <id>CVC-Transcription</id>
	  <name>CVC-Transcription</name>
	  <description>CVC-Transcription</description>
	  <type_list>
	    <type_list_item>
	      <id>cvc word</id>
	      <name>cvc word</name>
	      <description>cvc word</description>
	    </type_list_item>
	  </type_list>
	</property>
      </value_list_item>
    </value_list>
  </page_element_list_item>
  <xsl:apply-templates select="atom">
    <xsl:with-param name="pos" select="position()"/>
    <xsl:with-param name="word_name" select="concat($textLine_name, concat('-word-', @id))"/>
  </xsl:apply-templates>
</xsl:template>

<xsl:template match="skeleton">
  <xsl:param name="pos"/>
  <xsl:param name="cc_name"/>
  <page_element_list_item>
    <id><xsl:value-of select="$cc_name"/>-skeleton</id>
    <type_list>
      <type_list_item>
        <id>cvc skeleton</id>
        <name>cvc skeleton</name>
        <description>This is the cvc skeleton datatype.</description>
      </type_list_item>
    </type_list>
    <value_list>
	  <!-- this value_list_item describes that this cc is included in a atom -->
      <value_list_item>
	    <id/>
	    <value replace="true"><xsl:value-of select="$cc_name"/></value>
	    <property>
	      <id>CVC-Page-Element-Included</id>
          <name>CVC-Page-Element-Included</name>
	      <description>CVC-Page-Element-Included</description>
	    </property>
      </value_list_item>
    </value_list>
	<mask>
	    <xsl:variable name="xpoints" select="xpoints"/>
		<xsl:variable name="ypoints" select="ypoints"/>
	    <xsl:variable name="points">
		  <xsl:call-template name="split">
	        <xsl:with-param name="str1" select="concat($xpoints, ' ')"/>
	        <xsl:with-param name="str2" select="concat($ypoints, ' ')"/>
	        <xsl:with-param name="deli" select="' '"/>
          </xsl:call-template>
		</xsl:variable>
		<xsl:value-of select="translate(translate($points,'&#10;', ''), '&#09;', '')" />
		</mask>
  </page_element_list_item>
</xsl:template>

	  
<xsl:template match="cc">
  <xsl:param name="pos"/>
  <xsl:param name="atom_name"/>
  <page_element_list_item>
    <id><xsl:value-of select="$atom_name"/>-cc-<xsl:value-of select="@id" /></id>
    <type_list>
      <type_list_item>
        <id>cvc cc</id>
        <name>cvc cc</name>
        <description>This is the cvc cc datatype.</description>
      </type_list_item>
    </type_list>
    <value_list>
	  <!-- this value_list_item describes that this cc is included in a atom -->
      <value_list_item>
	    <id/>
	    <value replace="true"><xsl:value-of select="$atom_name"/></value>
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
	          <id>cvc cc</id>
	          <name>cvc cc</name>
	          <description>cvc cc</description>
	        </type_list_item>
	      </type_list>
	    </property>
      </value_list_item>
    </value_list>
	<mask>
	    <xsl:variable name="xpoints" select="area/xpoints"/>
		<xsl:variable name="ypoints" select="area/ypoints"/>
	    <xsl:variable name="points">
		  <xsl:call-template name="split">
	        <xsl:with-param name="str1" select="concat($xpoints, ' ')"/>
	        <xsl:with-param name="str2" select="concat($ypoints, ' ')"/>
	        <xsl:with-param name="deli" select="' '"/>
          </xsl:call-template>
		</xsl:variable>
		<xsl:value-of select="translate(translate($points,'&#10;', ''), '&#09;', '')" />
		</mask>
  </page_element_list_item>
  <xsl:apply-templates select="skeleton">
    <xsl:with-param name="pos" select="position()"/>
    <xsl:with-param name="cc_name" select="concat($atom_name, concat('-cc-', @id))"/>
  </xsl:apply-templates>
</xsl:template>

<xsl:template match="skeleton/xpoints">
  <xsl:value-of select="text()"/>
</xsl:template>

<xsl:template match="skeleton/ypoints">
  <xsl:value-of select="text()"/>
</xsl:template>

<xsl:template match="area/xpoints">
  <boundary_list_item>
    <type>area</type>
	<description>This is a area boundary - xpoints</description>
    <value><xsl:value-of select="text()"/></value>
  </boundary_list_item>
</xsl:template>

<xsl:template match="area/ypoints">
  <boundary_list_item>
    <type>area</type>
	<description>This is a area boundary - ypoints</description>
    <value><xsl:value-of select="text()"/></value>
  </boundary_list_item>
</xsl:template>

<xsl:template match="atom">
  <xsl:param name="pos"/>
  <xsl:param name="word_name"/>
  <page_element_list_item>
    <id><xsl:value-of select="$word_name"/>-atom-<xsl:value-of select="@id" /></id>
    <type_list>
      <type_list_item>
        <id>cvc atom</id>
        <name>cvc atom</name>
        <description>This is the cvc atom datatype.</description>
      </type_list_item>
    </type_list>
    <value_list>
      <value_list_item>
	<id/>
	<value replace="true"><xsl:value-of select="$word_name"/></value>
  	<property>
	  <id>CVC-Page-Element-Included</id>
	  <name>CVC-Page-Element-Included</name>
	  <description>CVC-Page-Element-Included</description>
	</property>
      </value_list_item>
      <value_list_item>
	<id/>
	<value><xsl:value-of select="@Transcription"/></value>
	<property>
	  <id>CVC-Transcription</id>
	  <name>CVC-Transcription</name>
	  <description>CVC-Transcription</description>
	  <type_list>
	    <type_list_item>
	      <id>cvc atom</id>
	      <name>cvc atom</name>
	      <description>cvc atom</description>
	    </type_list_item>
	  </type_list>
	</property>
      </value_list_item>
      <value_list_item>
	<id/>
	<value><xsl:value-of select="@SplitByDesign"/></value>
	<property>
	  <id>CVC-SplitByDesign</id>
	  <name>CVC-SplitByDesign</name>
	  <description>CVC-SplitByDesign</description>
	  <type_list>
	    <type_list_item>
	      <id>cvc atom</id>
	      <name>cvc atom</name>
	      <description>CVC atom</description>
	    </type_list_item>
	  </type_list>
	</property>
      </value_list_item>
      <value_list_item>
	<id/>
	<value><xsl:value-of select="@MergedByDesign"/></value>
	<property>
	  <id>CVC-MergedByDesign</id>
	  <name>CVC-MergedByDesign</name>
	  <description>CVC-MergedByDesign</description>
	  <type_list>
	    <type_list_item>
	      <id>cvc atom</id>
	      <name>cvc atom</name>
	      <description>CVC atom</description>
	    </type_list_item>
	  </type_list>
	</property>
      </value_list_item>
  </value_list>
  </page_element_list_item>
  <xsl:apply-templates select="cc">
    <xsl:with-param name="pos" select="position()"/>
    <xsl:with-param name="atom_name" select="concat($word_name, concat('-atom-', @id))"/>
  </xsl:apply-templates>
</xsl:template>

<xsl:template match="textLine">
  <xsl:param name="pos"/>
  <xsl:param name="image_name"/>
  <page_element_list_item>
    <id><xsl:value-of select="$image_name"/>-textLine-<xsl:value-of select="@id" /></id>
    <topleftx><xsl:value-of select="@Xmin"/></topleftx>
    <toplefty><xsl:value-of select="@Ymin"/></toplefty>
    <width><xsl:value-of select="number(@Xmax)-number(@Xmin)"/></width>
    <height><xsl:value-of select="number(@Ymax)-number(@Ymin)"/></height>
    <type_list>
      <type_list_item>
	<id>cvc textline</id>
	<name>cvc textline</name>
	<description>This is the cvc textLine datatype.</description>
      </type_list_item>      
    </type_list>
    <value_list>
      <value_list_item>
	<id/>
	<value><xsl:value-of select="@Transcription"/></value>
	<property>
	  <id>CVC-Transcription</id>
	  <name>CVC-Transcription</name>
	  <description>CVC-Transcription</description>
	  <type_list>
	    <type_list_item>
	      <id>cvc textline</id>
	      <name>cvc textline</name>
	      <description>cvc textline</description>
	    </type_list_item>
	  </type_list>
	</property>
      </value_list_item>
    </value_list>
  </page_element_list_item>
  <xsl:apply-templates select="word">
    <xsl:with-param name="pos" select="position()"/>
    <xsl:with-param name="textLine_name" select="concat($image_name, concat('-textLine-', @id))"/>
  </xsl:apply-templates>
</xsl:template>
  
</xsl:stylesheet>