<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" encoding="ISO-8859-1" indent="yes"/> 

<!-- <mapping>
	Ballot Annotation Area
	Description String
	Numeric Identifier
	Alphanumeric Identifier
	boolean
</mapping> -->

<!-- Note weird use of attributes like variable="legalvote" value="0" -->

<xsl:template match="/annotations">
	<xsl:apply-templates select="race"/>
</xsl:template>

<xsl:template match="cand">

      <xsl:variable name="X1" select="entry[@textvariable='x1']/@value"/>
      <xsl:variable name="X2" select="entry[@textvariable='x2']/@value"/>
      <xsl:variable name="Y1" select="entry[@textvariable='y1']/@value"/>
      <xsl:variable name="Y2" select="entry[@textvariable='y2']/@value"/>

      <xsl:if test="$X1 != ''">

      <page_element_list_item>
	  <id>
	      <xsl:value-of select="parent::node()/@id"/>_<xsl:value-of select="@id"/>
	  </id>
	  <description>Ballot Annotation Area</description>

	  <topleftx>
	      cand<xsl:value-of select="@id"/>_X
	      <!--xsl:value-of select="2 * $X1"/-->
	  </topleftx>

	  <toplefty>
	      cand<xsl:value-of select="@id"/>_Y
	      <!--xsl:value-of select="2 * $Y1"/-->
	  </toplefty>

 	  <width>
	      cand<xsl:value-of select="@id"/>_W
	      <!--xsl:value-of select="2 * ( $X2 - $X1 )"/-->
	  </width>

	  <height>
	      cand<xsl:value-of select="@id"/>_H
	      <!--xsl:value-of select="2 * ( $Y2 - $Y1 )"/-->
	  </height>     

	  <type_list>
	    <type_list_item>
	      <id>Ballot Annotation Area</id>
	    </type_list_item>
	  </type_list>

	  <value_list>
	    <value_list_item>
	      <id>raceNameValue_<xsl:value-of select="parent::node()/@id"/>_<xsl:value-of select="@id"/></id>
	      <description>Name of the Ballot Race</description>
	      <value><xsl:value-of select="parent::node()/@race"/></value>
	      <property>
		<id>Ballot Race Name</id>
		<description>This property describes the name of a ballot's race</description>
		<type_list>
		   <type_list_item>
		      <id>Description String</id>
		   </type_list_item>
		</type_list>
	      </property>
	    </value_list_item>
	    <value_list_item>
	      <id>raceIdValue_<xsl:value-of select="parent::node()/@id"/>_<xsl:value-of select="@id"/></id>
	      <description>Identifier of the Ballot Race</description>
	      <value><xsl:value-of select="parent::node()/@id"/></value>
	      <property>
		<id>Ballot Race Identifier</id>
		<description>This property describes the indentifier of a ballot's race</description>
		<type_list>
		   <type_list_item>
		      <id>Numeric Identifier</id>
		   </type_list_item>
		</type_list>
	      </property>
	    </value_list_item>
	    <value_list_item>
	      <id>candIdValue_<xsl:value-of select="parent::node()/@id"/>_<xsl:value-of select="@id"/></id>
	      <description>Identifier of the ballot candidate</description>
	      <value><xsl:value-of select="@id"/></value>
	      <property>
		<id>Ballot Candidate Identifier</id>
		<description>This property describes the indentifier of a ballot's candidate</description>
		<type_list>
		   <type_list_item>
		      <id>Alphanumeric Identifier</id>
		   </type_list_item>
		</type_list>
	      </property>
	    </value_list_item>
	    <value_list_item>
	      <id>candNameValue_<xsl:value-of select="parent::node()/@id"/>_<xsl:value-of select="@id"/></id>
	      <description>Name of the ballot candidate</description>
	      <value><xsl:value-of select="@name"/></value>
	      <property>
		<id>Ballot Candidate Name</id>
		<description>This property describes the name of a ballot's candidate</description>
		<type_list>
		   <type_list_item>
		      <id>Description String</id>
		   </type_list_item>
		</type_list>
	      </property>
	    </value_list_item>
	    <value_list_item>
	      <id>legalVote_<xsl:value-of select="parent::node()/@id"/>_<xsl:value-of select="@id"/></id>
	      <description>Validity of expressed vote</description>
	      <value><xsl:value-of select="@value"/></value>
	      <property>
		<id>Ballot Vote Legality</id>
		<description>This property describes validity of an expressed vote</description>
		<type_list>
		   <type_list_item>
		      <id>boolean</id>
		   </type_list_item>
		</type_list>
	      </property>
	    </value_list_item>
	    <value_list_item>
	      <id>markType_<xsl:value-of select="parent::node()/@id"/>_<xsl:value-of select="@id"/></id>
	      <description>The type of marking detected</description>
		<!-- We make the (perhaps wrong) assumption that all "radiobutton" nodes
			have the same "value" attribute -->
	      <value><xsl:value-of select="radiobutton/@value"/></value>
	      <property>
		<id>Ballot Vote Markup Type</id>
		<description>This property describes the markup type detected for expressing a vote</description>
		<type_list>
		   <type_list_item>
		      <id>Description String</id>
		   </type_list_item>
		</type_list>
	      </property>
	    </value_list_item>
	    <value_list_item>
	      <id>Annotator_<xsl:value-of select="parent::node()/@id"/>_<xsl:value-of select="@id"/></id>
	      <description></description>
	      <value>BallotAnnotatorNameTag</value>
	      <property>
	        <id>Ballot Annotator</id>
	      </property>
	    </value_list_item>
          </value_list>

      </page_element_list_item> 

      </xsl:if>


</xsl:template>

<xsl:template match="race">
    <xsl:apply-templates select="cand"/>
</xsl:template>


<xsl:template match="entry">

    <xsl:if test="@value != ''">
      <xsl:choose>
	 <xsl:when test="@textvariable='x1'">
	  <topleftx>
	          <xsl:value-of select="@value"/>
	  </topleftx>
         </xsl:when>
	 <xsl:when test="@textvariable='y1'">
	  <toplefty>
	          <xsl:value-of select="@value"/>
	  </toplefty>
         </xsl:when>
      </xsl:choose>
    </xsl:if>

</xsl:template>

</xsl:stylesheet>



