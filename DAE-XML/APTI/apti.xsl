<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" encoding="UTF-8" indent="yes"/> 
 
  <xsl:template match="/wordImage">
    <page_image>
      <id><xsl:value-of select="@id" /></id>
	  <xsl:apply-templates select="specs"/>
	  <copyright_list/>
	  <contributor_list>
		<contributor_list_item>
			<id>APTI</id>
			<name>APTI</name>
		</contributor_list_item>
	  </contributor_list>
	  <value_list>
		<xsl:apply-templates select="font"/>
		<xsl:apply-templates select="generation"/>
		<xsl:apply-templates select="content"/>
	  </value_list>
	  <page_element_list>
		<xsl:apply-templates select="content/paw"/>
	  </page_element_list>
    </page_image>
  </xsl:template>
 
 <xsl:template match="content/paw">
	<page_element_list_item>
		<id><xsl:value-of select="@id" /></id>
		<name>paw <xsl:value-of select="@id" /></name>
		<description>paw <xsl:value-of select="@id" /></description>
		<value_list>
			<value_list_item>
				<id/>
				<value><xsl:value-of select="@id" /></value>
				<property>
					<id>APTI paw ordering</id>
					<name>APTI paw ordering</name>
					<description>APTI paw ordering</description>
					<type_list>
						<type_list_item>
							<id>APTI Paw</id>
							<name>APTI Paw</name>
							<description>APTI Paw</description>
						</type_list_item>
					</type_list>
				</property>
			</value_list_item>
			<value_list_item>
				<id/>
				<value><xsl:value-of select="@nbChars" /></value>
				<property>
					<id>APTI paw characters count</id>
					<name>APTI paw characters count</name>
					<description>APTI paw characters count</description>
					<type_list>
						<type_list_item>
							<id>APTI Paw</id>
						</type_list_item>
					</type_list>
				</property>
			</value_list_item>
			<value_list_item>
				<id/>
				<value><xsl:value-of select="." /></value>
				<property>
					<id>APTI paw text</id>
					<name>APTI paw text</name>
					<description>APTI paw text content</description>
					<type_list>
						<type_list_item>
							<id>APTI Paw</id>
						</type_list_item>
					</type_list>
				</property>
			</value_list_item>
		</value_list>
	</page_element_list_item>
 </xsl:template>
  
 <xsl:template match="content">
	  <value_list_item>
		<id/>
		<value><xsl:value-of select="@transcription" /></value>
		<property>
			<id>APTI wordImage transcription</id>
			<name>APTI wordImage transcription</name>
			<description>APTI wordImage transcription</description>
			<type_list>
				<type_list_item>
					<id>APTI wordImage</id>
				</type_list_item>
			</type_list>
		</property>
	  </value_list_item>
	  <value_list_item>
		<id/>
		<value><xsl:value-of select="@nbPaws" /></value>
		<property>
			<id>APTI wordImage number of page elements</id>
			<type_list>
				<type_list_item>
					<id>APTI wordImage</id>
				</type_list_item>
			</type_list>
		</property>
	  </value_list_item>
  </xsl:template>
  
 <xsl:template match="generation">
	  <value_list_item>
		<id/>
		<value><xsl:value-of select="@type" /></value>
		<property>
			<id>APTI wordImage render type</id>
			<type_list>
				<type_list_item>
					<id>APTI wordImage</id>
				</type_list_item>
			</type_list>
		</property>
	  </value_list_item>
	  <value_list_item>
		<id/>
		<value><xsl:value-of select="@renderer" /></value>
		<property>
			<id>APTI renderer</id>
			<type_list>
				<type_list_item>
					<id>APTI wordImage</id>
				</type_list_item>
			</type_list>
		</property>
	  </value_list_item>
	  <value_list_item>
		<id/>
		<value><xsl:value-of select="@filtering" /></value>
		<property>
			<id>APTI render filtering</id>
			<type_list>
				<type_list_item>
					<id>APTI wordImage</id>
				</type_list_item>
			</type_list>
		</property>
	  </value_list_item>
  </xsl:template>
  
 <xsl:template match="font">
	  <value_list_item>
		<id/>
		<value><xsl:value-of select="@name" /></value>
		<property>
			<id>APTI font name</id>
			<type_list>
				<type_list_item>
					<id>APTI wordImage</id>
					<name>APTI wordImage</name>
					<description>This is the APTI wordImage datatype.</description>
				</type_list_item>
			</type_list>
		</property>
	  </value_list_item>
	  <value_list_item>
		<id/>
		<value><xsl:value-of select="@fontStyle" /></value>
		<property>
			<id>APTI font style</id>
			<type_list>
				<type_list_item>
					<id>APTI wordImage</id>
				</type_list_item>
			</type_list>
		</property>
	  </value_list_item>
	  <value_list_item>
		<id/>
		<value><xsl:value-of select="@size" /></value>
		<property>
			<id>APTI font size</id>
			<type_list>
				<type_list_item>
					<id>APTI wordImage</id>
				</type_list_item>
			</type_list>
		</property>
	  </value_list_item>
  </xsl:template>
  
  <xsl:template match="specs">
	<width><xsl:value-of select="@width" /></width>
	<height><xsl:value-of select="@width" /></height>
    <type_list>
	  <type_list_item>
		<id><xsl:value-of select="@encoding" /></id>
		<name><xsl:value-of select="@encoding" /></name>
		<description>This is a datatype of <xsl:value-of select="@encoding" /> images.</description>
	  </type_list_item>
	  <type_list_item>
		<id>APTI wordImage</id>
		<name>APTI wordImage</name>
		<description>This is a datatype of APTI wordImages.</description>
	  </type_list_item>
    </type_list>
  </xsl:template>
 
</xsl:stylesheet>
