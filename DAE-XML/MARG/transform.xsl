<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" encoding="ISO-8859-1" indent="yes"/> 

 <!-- Find the root entry of the document -->
 <xsl:template match="/">
	<metadata>
	  <mapping>
	    <pair>
	      <!-- This xsl file requires that $imagename and $imagepath are provided externally -->
	      <local><xsl:value-of select="$imagepath"/></local>
	      <db><xsl:value-of select="$imageid"/></db>
	    </pair>
	    <pair>
	      <local>MARG Zone</local>
	      <db>MARG_Zone</db>
	    </pair>
	    <pair>
	      <local>MARG Line</local>
	      <db>MARG_Line</db>
	    </pair>
	    <pair>
	      <local>MARG Word</local>
	      <db>MARG_Word</db>
	    </pair>
	    <pair>
	      <local>MARG Character</local>
	      <db>MARG_Character</db>
	    </pair>
	    <pair>
	      <local>Page Element Inclusion</local>
	      <db>Page_Element_Inclusion</db>
	    </pair>
	    <pair>
	      <local>MARG Classification Category</local>
	      <db>MARG_Classification_Category</db>
	    </pair>
	    <pair>
	      <local>MARG Classification Type</local>
	      <db>MARG_Classification_Type</db>
	    </pair>
	    <pair>
	      <local>MARG PointSize</local>
	      <db>MARG_PointSize</db>
	    </pair>
	    <pair>
	      <local>MARG Style</local>
	      <db>MARG_Style</db>
	    </pair>
	    <pair>
	      <local>MARG Color</local>
	      <db>MARG_Color</db>
	    </pair>
	    <pair>
	      <local>MARG Next Zone</local>
	      <db>MARG_Next_Zone</db>
	    </pair>
	    <pair>
	      <local>MARG Next Line</local>
	      <db>MARG_Next_Line</db>
	    </pair>
	    <pair>
	      <local>MARG Next Word</local>
	      <db>MARG_Next_Word</db>
	    </pair>
	    <pair>
	      <local>MARG Next Character</local>
	      <db>MARG_Next_Character</db>
	    </pair>
	    <pair>
	      <local>MARG Text</local>
	      <db>MARG_Text</db>
	    </pair>
	  </mapping>

	  <!-- For each PAGE element whose path on the parse tree is /PAGE, use the template to transform-->
	  <xsl:apply-templates select="/Page"/>
	</metadata>
  </xsl:template>
<!--

-->

<xsl:template name="Inclusion">
  <xsl:param name="parentId"/>
  <xsl:param name="childId"/>

  <value_list_item>
    <id>Included_in_<xsl:value-of select="$childId"/></id>
    <description>page_element_inclusion</description>
    <value replace="true"><xsl:value-of select="$parentId"/></value>
    <property>
      <id>Page Element Inclusion</id>
    </property>
  </value_list_item>
</xsl:template>

<xsl:template name="BoundingBox">
  <xsl:param name="node"/>
  <xsl:param name="typeName"/>

  <!-- Making a very brutal assumption about ZoneCorners
      notably that Vertex[1] is the upper left corner, and that Vertex[3] is the lower right -->
  <topleftx><xsl:value-of select="$node/Vertex[1]/@x"/></topleftx>
  <toplefty><xsl:value-of select="$node/Vertex[1]/@y"/></toplefty>
  <width><xsl:value-of select="number($node/Vertex[3]/@x)-$node/Vertex[1]/@x"/></width>
  <height><xsl:value-of select="number($node/Vertex[3]/@y)-$node/Vertex[1]/@y"/></height>     
  <type_list>
    <type_list_item>
      <id><xsl:value-of select="$typeName"/></id>
      <!--<name><xsl:value-of select="$typeName"/></name>-->
    </type_list_item>
  </type_list>
</xsl:template>

<xsl:template match="/Page">

    <page_image>
        <xsl:variable name="page_id" select="PageID/@Value"/>
	
	<!-- This xsl file requires that $imagename and $imagepath are provided externally -->
        <id><xsl:value-of select="$imagepath"/></id>
        <!--<name><xsl:value-of select="$imagename"/></name>-->
        <!--<path><xsl:value-of select="$imagepath"/></path>-->

    <page_element_list>

	<xsl:apply-templates select="Zone">
	  <xsl:with-param name="parent_id" select="$page_id"/>
	</xsl:apply-templates>

    </page_element_list>

   </page_image>
</xsl:template>

<xsl:template match="Zone">
    <xsl:param name="parent_id" />

    <xsl:variable name="zone_id">
      <xsl:value-of select="$parent_id" />_<xsl:value-of select="ZoneID/@Value"/>
    </xsl:variable>

    <page_element_list_item>
      <id>Zone_<xsl:value-of select="$zone_id"/></id>
      <description>Zone</description>

      <xsl:call-template name="BoundingBox">
	<xsl:with-param name="node" select="ZoneCorners"/>
	<xsl:with-param name="typeName">MARG Zone</xsl:with-param>
      </xsl:call-template> 
      <value_list>
  
	    <!-- The following list of transformations is quite straightforward:
		  - Checking whether the Tag+Attribute exsist
		  - If yes, transform them into a page_element_property_value -->
	    
            <xsl:if test="Classification/Category/@Value != ''">
	      <value_list_item>
		<id>Classification_Category_<xsl:value-of select="$zone_id"/></id>
		<description>Classification Category</description>
		<value><xsl:value-of select="Classification/Category/@Value"/></value>
		<property>
		  <id>MARG Classification Category</id>
		</property>
	      </value_list_item>
	    </xsl:if>
            <xsl:if test="Classification/Type/@Value != ''">
	      <value_list_item>
		<id>Classification_Type_<xsl:value-of select="$zone_id"/></id>
		<description>Classification Type</description>
		<value><xsl:value-of select="Classification/Type/@Value"/></value>
		<property>
		  <id>MARG Classification Type</id>
		</property>
	      </value_list_item>
	    </xsl:if>

	<!-- XSL treatment is not complete, and should handle the full DTD -->

	<xsl:apply-templates select="Color">
	  <xsl:with-param name="parent_id" select="$zone_id"/>
	</xsl:apply-templates>
	<xsl:apply-templates select="ZoneNext">
	  <xsl:with-param name="parent_id" select="$parent_id"/>
	  <xsl:with-param name="zone_id" select="$zone_id"/>
	</xsl:apply-templates>
	<xsl:apply-templates select="GT_Text">
	  <xsl:with-param name="parent_id" select="$zone_id"/>
	</xsl:apply-templates>
	</value_list>
      </page_element_list_item> 

      <xsl:apply-templates select="Line">
	 <xsl:with-param name="parent_id" select="$zone_id"/>
      </xsl:apply-templates>

</xsl:template>

<xsl:template match="Line">
    <xsl:param name="parent_id"/>

    <xsl:variable name="line_id">
      <xsl:value-of select="$parent_id"/>_<xsl:value-of select="LineID/@Value"/>
    </xsl:variable>

    <page_element_list_item>
      <id>Line_<xsl:value-of select="$line_id"/></id>
      <description>Line</description>

      <xsl:call-template name="BoundingBox">
	  <xsl:with-param name="node" select="LineCorners"/>
	  <xsl:with-param name="typeName">MARG Line</xsl:with-param>
      </xsl:call-template> 

      <value_list>
  
	<!-- The following list of transformations is quite straightforward:
		  - Checking whether the Tag+Attribute exsist
		  - If yes, transform them into a page_element_property_value -->

	<xsl:call-template name="Inclusion">
	  <xsl:with-param name="parentId">Zone_<xsl:value-of select="$parent_id"/></xsl:with-param>
	  <xsl:with-param name="childId" select="$line_id"/>
	</xsl:call-template> 	    
	<xsl:apply-templates select="Color">
	  <xsl:with-param name="parent_id" select="$line_id"/>
	</xsl:apply-templates>
	<xsl:apply-templates select="LineNext">
	  <xsl:with-param name="parent_id" select="$parent_id"/>
	  <xsl:with-param name="line_id" select="$line_id"/>
	</xsl:apply-templates>
	<xsl:apply-templates select="GT_Text">
	  <xsl:with-param name="parent_id" select="$line_id"/>
	</xsl:apply-templates>
      </value_list>
    </page_element_list_item> 

      <xsl:apply-templates select="Word">
	 <xsl:with-param name="parent_id" select="$line_id"/>
      </xsl:apply-templates>

      <xsl:apply-templates select="Character">
	 <xsl:with-param name="parent_id" select="$line_id"/>
	 <xsl:with-param name="parent_type" select='Line'/>
	  </xsl:apply-templates>

</xsl:template>

<xsl:template match="Word">
    <xsl:param name="parent_id"/>

	<!-- cf. comment below 
    <xsl:variable name="word_id">
      <xsl:value-of select="$parent_id"/>_<xsl:value-of select="WordID/@Value"/>
    </xsl:variable>
     -->
    <!--  The following is actually a hack to handle with original XML files not correctly using the Value attribute -->
    <xsl:variable name="word_id">
      <xsl:value-of select="$parent_id" />_<xsl:number count="*"/>
    </xsl:variable>

    <page_element_list_item>
      <id>Word_<xsl:value-of select="$word_id"/></id>
      <description>Word</description>
	<xsl:call-template name="BoundingBox">
	  <xsl:with-param name="node" select="WordCorners"/>
	  <xsl:with-param name="typeName">MARG Word</xsl:with-param>
	</xsl:call-template> 


      <value_list>
  
	<!-- The following list of transformations is quite straightforward:
		  - Checking whether the Tag+Attribute exsist
		  - If yes, transform them into a page_element_property_value -->

	<xsl:call-template name="Inclusion">
	  <xsl:with-param name="parentId">Line_<xsl:value-of select="$parent_id"/></xsl:with-param>
	  <xsl:with-param name="childId" select="$word_id"/>
	</xsl:call-template>

	<xsl:apply-templates select="Color">
	  <xsl:with-param name="parent_id" select="$word_id"/>
	</xsl:apply-templates>

	<xsl:apply-templates select="WordNext">
	  <xsl:with-param name="parent_id" select="$parent_id"/>
	  <xsl:with-param name="word_id" select="$word_id"/>
	</xsl:apply-templates>

	<xsl:apply-templates select="GT_Text">
	  <xsl:with-param name="parent_id" select="$word_id"/>
	</xsl:apply-templates>
      </value_list>
    </page_element_list_item> 

      <xsl:apply-templates select="Character">
	 <xsl:with-param name="parent_id" select="$word_id"/>
	 <xsl:with-param name="parent_type" select='Word'/>
      </xsl:apply-templates>
</xsl:template>

<xsl:template match="Character">
    <xsl:param name="parent_id"/>
    <xsl:param name="parent_type"/>

	<!-- cf. comment below 
     <xsl:variable name="character_id">
      <xsl:value-of select="$parent_id"/>_<xsl:value-of select="CharacterID/@Value"/>
    </xsl:variable>
     -->
    <!--  The following is actually a hack to handle with original XML files not correctly using the Value attribute -->
    <xsl:variable name="character_id">
      <xsl:value-of select="$parent_id" />_<xsl:number count="*"/>
    </xsl:variable>
    
    <page_element_list_item>
      <id>Character_<xsl:value-of select="$character_id"/></id>
      <description>Character</description>
	<xsl:call-template name="BoundingBox">
	  <xsl:with-param name="node" select="CharacterCorners"/>
	  <xsl:with-param name="typeName">MARG Character</xsl:with-param>
	</xsl:call-template> 


      <value_list>
  
	<!-- The following list of transformations is quite straightforward:
		  - Checking whether the Tag+Attribute exsist
		  - If yes, transform them into a page_element_property_value -->

	<xsl:call-template name="Inclusion">
	  <xsl:with-param name="parentId"><xsl:value-of select="$parent_type"/>_<xsl:value-of select="$parent_id"/></xsl:with-param>
	  <xsl:with-param name="childId" select="$character_id"/>
	</xsl:call-template>

	<xsl:apply-templates select="Color">
	  <xsl:with-param name="parent_id" select="$character_id"/>
	</xsl:apply-templates>

	<xsl:apply-templates select="CharacterNext">
	  <xsl:with-param name="parent_id" select="$parent_id"/>
	  <xsl:with-param name="character_id" select="$character_id"/>
	</xsl:apply-templates>

	<xsl:apply-templates select="GT_Text">
	  <xsl:with-param name="parent_id" select="$character_id"/>
	</xsl:apply-templates>

	<value_list_item>
	  <id>PointSize_<xsl:value-of select="$character_id"/></id>
	  <description>PointSize</description>
	  <value><xsl:value-of select="PointSize/@Value"/></value>
	  <property>
	    <id>MARG PointSize</id>
	  </property>
	</value_list_item>

	<value_list_item>
	  <id>Style_<xsl:value-of select="$character_id"/></id>
	  <description>Style</description>
	  <value><xsl:value-of select="Style/@Value"/></value>
	  <property>
	    <id>MARG Style</id>
	  </property>
	</value_list_item>
      </value_list>

    </page_element_list_item> 

</xsl:template>

<xsl:template match="Color">
    <xsl:param name="parent_id" />

    <value_list_item>
      <id>Color_<xsl:value-of select="$parent_id"/></id>
      <description>Color</description>
      <value><xsl:value-of select="@Value"/></value>
      <property>
	<id>MARG Color</id>
      </property>
    </value_list_item>
</xsl:template>

<xsl:template match="ZoneNext">
    <xsl:param name="parent_id" />
    <xsl:param name="zone_id" />

    <value_list_item>
      <id>ZoneNext_<xsl:value-of select="$zone_id"/></id>
      <description>Next Zone</description>
      <!-- Be aware of the repace="true" attribute in the value tag, indicating that subsequent treatments should
	   consider the value as a database ID -->
      <value replace="true">Zone_<xsl:value-of select="$parent_id"/>_<xsl:value-of select="@Value"/></value>
      <property>
	<id>MARG Next Zone</id>
      </property>
    </value_list_item>
</xsl:template>

<xsl:template match="LineNext">
    <xsl:param name="parent_id" />
    <xsl:param name="line_id" />

    <value_list_item>
      <id>LineNext_<xsl:value-of select="$line_id"/></id>
      <description>Next Line</description>
      <!-- Be aware of the repace="true" attribute in the value tag, indicating that subsequent treatments should
	   consider the value as a database ID -->
      <value replace="true">Line_<xsl:value-of select="$parent_id"/>_<xsl:value-of select="@Value"/></value>
      <property>
	<id>MARG Next Line</id>
      </property>
    </value_list_item>
</xsl:template>

<xsl:template match="WordNext">
    <xsl:param name="parent_id" />
    <xsl:param name="word_id" />

    <value_list_item>
      <id>WordNext_<xsl:value-of select="$word_id"/></id>
      <description>Next Word</description>
      <!-- Be aware of the repace="true" attribute in the value tag, indicating that subsequent treatments should
	   consider the value as a database ID -->
      <value replace="true">Word_<xsl:value-of select="$parent_id"/>_<xsl:value-of select="@Value"/></value>
      <property>
	<id>MARG Next Word</id>
      </property>
    </value_list_item>
</xsl:template>

<xsl:template match="CharacterNext">
    <xsl:param name="parent_id" />
    <xsl:param name="character_id" />

    <value_list_item>
      <id>CharacterNext_<xsl:value-of select="$character_id"/></id>
      <description>Next Character</description>
      <!-- Be aware of the repace="true" attribute in the value tag, indicating that subsequent treatments should
	   consider the value as a database ID -->
      <value replace="true">Character_<xsl:value-of select="$parent_id"/>_<xsl:value-of select="@Value"/></value>
      <property>
	<id>MARG Next Character</id>
      </property>
    </value_list_item>
</xsl:template>

<!-- This is currently incomplete and should, one day do base64 encoding -->
<!-- I guess it's OK now since the original format is supposed to be valid XML anyway -->
<xsl:template match="GT_Text">
    <xsl:param name="parent_id" />

    <value_list_item>
      <id>Text_<xsl:value-of select="$parent_id"/></id>
      <description>Text</description>
      <value><xsl:value-of select="@Value"/></value>
      <property>
		<id>MARG Text</id>
      </property>
    </value_list_item>
</xsl:template>


</xsl:stylesheet>



