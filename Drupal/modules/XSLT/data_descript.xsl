<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
xmlns:ns1="http://tempuri.org"
>
<!-- Declare global variables -->
<xsl:variable name="D_PATH" select="'./?q=descript/dataitem/'" />
<xsl:variable name="F_PATH" select="'./?q=browse/dataitem/download/'" />
<xsl:variable name="B_PATH" select="'./?q=browse/dataitem/'" />

<xsl:template match="/">

<!-- count first -->
<!-- count items in each category -->
	<xsl:variable name="cnt_file" select="count(SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/file_info)" />
	<xsl:variable name="cnt_dataset" select="count(SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/dataset_info)" /> 
	<xsl:variable name="cnt_page_element" select="count(SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/page_element_info)" /> 
	<xsl:variable name="cnt_page_image" select="count(SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/page_image_info)" /> 
	<xsl:variable name="cnt_property_value" select="count(SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/property_value_info)" /> 
	<xsl:variable name="cnt_unknown" select="count(SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/unknown_data_item)" />

<!-- a lot of hard coded html stuff here -->
<html>
<head>
<title>Data Item Description | Document Analysis and Exploitation</title>
<link rel="shortcut icon" href="./misc/favicon.ico" type="image/x-icon" />
<link type="text/css" rel="stylesheet" media="all" href="./sites/default/modules/dae_ui/jquery.alerts.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./modules/aggregator/aggregator.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./modules/book/book.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./modules/node/node.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./modules/system/defaults.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./modules/system/system.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./modules/system/system-menus.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./modules/user/user.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./sites/all/modules/ckeditor/ckeditor.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./sites/all/modules/simplenews/simplenews.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./sites/all/modules/tableofcontents/tableofcontents.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./sites/default/modules/ctools/css/ctools.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./sites/default/modules/dae_ui/default.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./sites/default/modules/dae_ui/stylesheets/jquery-ui.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./sites/default/modules/dae_ui/stylesheets/jquery.Jcrop.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./modules/forum/forum.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="./sites/default/files/color/garland/style.css?J" />
<link type="text/css" rel="stylesheet" media="print" href="./themes/garland/print.css?J" />

<script type="text/javascript" src="./misc/jquery.js?J"></script>
<script type="text/javascript" src="./misc/drupal.js?J"></script>
<script type="text/javascript" src="./sites/default/modules/dae_ui/jquery.alerts.js?J"></script>
<script type="text/javascript" src="./sites/all/modules/tableofcontents/jquery.scrollTo-min.js?J"></script>
<script type="text/javascript" src="./sites/all/modules/tableofcontents/jquery.localscroll-min.js?J"></script>
<script type="text/javascript" src="./sites/all/modules/tableofcontents/tableofcontents.js?J"></script>
<script type="text/javascript" src="./sites/default/modules/dae_ui/javascript/dae_data_pages.js?J"></script>
<script type="text/javascript" src="./sites/default/modules/dae_ui/dae_ui_effects.js?J"></script>
<script type="text/javascript" src="./sites/default/modules/dae_ui/javascript/jquery-ui.js?J"></script>
<script type="text/javascript">
<!--//--><![CDATA[//><!--
jQuery.extend(Drupal.settings, { "basePath": "./" });
//--><!]]>
</script>
<!--[if lt IE 7]>
<link type="text/css" rel="stylesheet" media="all" href="./themes/garland/fix-ie.css" />    <![endif]-->
</head>


<body class="sidebar-left">

<!-- Layout -->
  <div id="header-region" class="clear-block"></div>

    <div id="wrapper">
    <div id="container" class="clear-block">

      <div id="header">
        <div id="logo-floater">

        <h1><a href="./" title="Document Analysis and Exploitation"><img src="./sites/default/files/new_garland_logo.png" alt="Document Analysis and Exploitation" id="logo" /><span>Document Analysis and Exploitation</span></a></h1>        </div>

                  <ul class="links primary-links"><li class="menu-120 first"><a href="./" title="Home">Home</a></li>
</ul>                                  
      </div> <!-- /header -->

              <div id="sidebar-left" class="sidebar">
                    <div id="block-user-1" class="clear-block block block-user">

  <h2>Navigation</h2>

  <div class="content"><ul class="menu"><li class="leaf first"><a href="./?q=browse" title="Browse the DAE database.">Browse Data</a></li>
<li class="leaf"><a href="./?q=algorithms" title="Available Algorithms">Algorithms</a></li>

</ul></div>
</div>

        </div>
      
      <div id="center"><div id="squeeze"><div class="right-corner"><div class="left-corner">
                                        
<!-- start real xslt - xiz307 -->
										<h2>Data Item Description : 
<span class="dae-data-browse-functions">


<!-- Overview -->	
	<xsl:if test="$cnt_dataset &gt; 0">
		<a href="#dataset">Datasets (<xsl:value-of select="$cnt_dataset"/>)</a> &#160;
	</xsl:if>
	<xsl:if test="$cnt_file &gt; 0">
		<a href="#file">Files (<xsl:value-of select="$cnt_file"/>)</a> &#160;
	</xsl:if>
	<xsl:if test="$cnt_page_image &gt; 0">
		<a href="#page_image">Page Images (<xsl:value-of select="$cnt_page_image"/>)</a> &#160;
	</xsl:if>
	<xsl:if test="$cnt_page_element &gt; 0">
		<a href="#page_element">Page Elements (<xsl:value-of select="$cnt_page_element"/>)</a> &#160;
	</xsl:if>
	<xsl:if test="$cnt_property_value &gt; 0">
		<a href="#property_value">Property Values (<xsl:value-of select="$cnt_property_value"/>)</a> &#160;
	</xsl:if>
	<xsl:if test="$cnt_unknown &gt; 0">
		<a href="#unknown">Unknown Items (<xsl:value-of select="$cnt_unknown"/>)</a> &#160;
	</xsl:if>

	</span> 
	</h2>
<div class="clear-block">

<!-- Dataset -->
	<xsl:if test="$cnt_dataset &gt; 0">
		
		<h3><a name="dataset">Datasets</a></h3>
		<table border="1">
			<tr>
				<th>ID</th>
				<th>Name</th>
				<th>Description</th>
				<th>Copyright</th>
				<th>Types</th>
				<th>includes Datasets</th>
				<th>includes Files</th>
				<th>includes Page Images</th>
				<th>includes Page Elements</th>
				<th>includes Property Values</th>
			</tr>
			<xsl:for-each select="SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/dataset_info">
			<xsl:sort select="id" data-type="number"/>
				<tr>
					<td>
						<xsl:apply-templates select="id"/>
					</td>
					<td><xsl:value-of select="name"/></td>
					<td><xsl:value-of select="description"/></td>
					<td>
						<xsl:apply-templates select="copyright_list"/>
					</td>
					<td>
						<xsl:apply-templates select="type_list"/>
					</td>
					<td>
						<xsl:apply-templates select="dataset_list"/>
					</td>
					<td>
						<xsl:apply-templates select="file_list"/>
					</td>
					<td>
						<xsl:apply-templates select="page_image_list"/>
					</td>
					<td>
						<xsl:apply-templates select="page_element_list"/>
					</td>
					<td>
						<xsl:apply-templates select="property_value_list"/>
					</td>
				</tr>
			</xsl:for-each>
		</table>
	</xsl:if>

<!-- File -->
	<xsl:if test="$cnt_file &gt; 0">
		<h3><a name="file">Files</a></h3>
		<table border="1">
			<tr>
				<th>ID</th>
				<th>Name</th>
				<th>Description</th>
				<th>Copyright</th>
				<th>Path</th>
			</tr>
			<xsl:for-each select="SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/file_info">
			<xsl:sort select="id" data-type="number"/>
				<tr>
					<td>
						<xsl:apply-templates select="id"/>
					</td>
					<td><xsl:value-of select="name"/></td>
					<td><xsl:value-of select="description"/></td>
					<td>
						<xsl:apply-templates select="copyright_list"/>
					</td>
					<td>
						<a>
							<xsl:attribute name="href">
								<xsl:value-of select="concat($F_PATH, ./id)"/>
							</xsl:attribute>
							Download
						</a>
					</td>
				</tr>
			</xsl:for-each>
		</table>
	</xsl:if>
	
<!-- Page Image -->
	<xsl:if test="$cnt_page_image &gt; 0">
		<h3><a name="page_image">Page Images</a></h3>
		<table border="1">
			<tr>
				<th>ID</th>
				<th>Description</th>
				<th>Copyright</th>
				<th>vdpi</th>
				<th>hdpi</th>
				<th>width</th>
				<th>height</th>
				<th>skew</th>
				<th>type_list</th>
				<th>page_element_list</th>
				<th>Path</th>
				
			</tr>
			<xsl:for-each select="SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/page_image_info">
			<xsl:sort select="id" data-type="number"/>
				<tr>
					<td>
						<xsl:apply-templates select="id"/>
					</td>
					<td><xsl:value-of select="description"/></td>
					<td>
						<xsl:apply-templates select="copyright_list"/>
					</td>
					<td><xsl:value-of select="vdpi"/></td>
					<td><xsl:value-of select="hdpi"/></td>
					<td><xsl:value-of select="width"/></td>
					<td><xsl:value-of select="height"/></td>
					<td><xsl:value-of select="skew"/></td>
					<td>
						<xsl:apply-templates select="type_list"/>
					</td>
					<td>
						<xsl:apply-templates select="page_element_list"/>
					</td>
					<td>
						<a>
							<xsl:attribute name="href">
								<xsl:value-of select="concat($F_PATH, ./id)"/>
							</xsl:attribute>
							Download
						</a>
					</td>
				</tr>
			</xsl:for-each>
		</table>
	</xsl:if>
	
<!-- Page Element -->
	<xsl:if test="$cnt_page_element &gt; 0">
		<h3><a name="page_element">Page Elements</a></h3>
		<table border="1">
			<tr>
				<th>ID</th>
				<th>Description</th>
				<th>Copyright</th>
				<th>in Page Image</th>
				<th>Type List</th>
				<th>Property Value List</th>
			</tr>
			<xsl:for-each select="SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/page_element_info">
			<xsl:sort select="id" data-type="number"/>
				<tr>
					<td>
						<xsl:apply-templates select="id"/>
					</td>
					<td><xsl:value-of select="description"/></td>
					<td>
						<xsl:apply-templates select="copyright_list"/>
					</td>
					
					<td>
						<a>
							<xsl:attribute name="href">
								<xsl:value-of select="concat($D_PATH, ./in_page_image)"/>
							</xsl:attribute>
							<xsl:value-of select="in_page_image"/>
						</a>
					</td>
					<td>
						<xsl:apply-templates select="type_list"/>
					</td>
					<td>
						<xsl:apply-templates select="property_value_list"/>
					</td>
				</tr>
			</xsl:for-each>
		</table>
	</xsl:if>
	
<!-- Property Values -->
	<xsl:if test="$cnt_property_value &gt; 0">
		<h3><a name="property_value">Property Values</a></h3>
		<table border="1">
			<tr>
				<th>ID</th>
				<th>Description</th>
				<th>Copyright</th>
				<th>in Page Element</th>
				<th>Property</th>
				<th>Value</th>
			</tr>
			<xsl:for-each select="SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/property_value_info">
			<xsl:sort select="id" data-type="number"/>
				<tr>
					<td>
						<xsl:apply-templates select="id"/>
					</td>
					<td><xsl:value-of select="description"/></td>
					<td>
						<xsl:apply-templates select="copyright_list"/>
					</td>
					<td>
						<a>
							<xsl:attribute name="href">
								<xsl:value-of select="concat($D_PATH, ./in_page_element)"/>
							</xsl:attribute>
							<xsl:value-of select="in_page_element"/>
						</a>
					</td>
					<td><xsl:value-of select="property"/></td>
					<td><xsl:value-of select="value"/></td>
				</tr>
			</xsl:for-each>
		</table>
	</xsl:if>
	
<!-- Unknown Items -->
	<xsl:if test="$cnt_unknown &gt; 0">
		<h3><a name="unknown">Unknwon Data Items</a></h3>
		<table border="1">
			<tr>
				<th>ID</th>
				<th>Description</th>
			</tr>
			<xsl:for-each select="SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item/unknown_data_item">
			<xsl:sort select="id" data-type="number"/>
				<tr>
					<td>
						<xsl:apply-templates select="id"/>
					</td>
					<td><xsl:value-of select="description"/></td>
				</tr>
			</xsl:for-each>
		</table>
	</xsl:if>



<!-- end real xslt - xiz307 -->
			</div>
                    <div id="footer"><div id="block-system-0" class="clear-block block block-system">

  <div class="content"><a href="http://drupal.org"><img src="./misc/powered-blue-80x15.png" alt="Powered by Drupal, an open source content management system" title="Powered by Drupal, an open source content management system" width="80" height="15" /></a></div>
</div>
</div>
      </div></div></div></div> <!-- /.left-corner, /.right-corner, /#squeeze, /#center -->

      
    </div> <!-- /container -->
  </div>
<!-- /layout -->

    </body>

</html>
</xsl:template>

<xsl:template match="id">
	<a>
		<xsl:attribute name="href">
			<xsl:value-of select="concat($B_PATH, .)"/>
		</xsl:attribute>
		<xsl:value-of select="."/>
	</a>
</xsl:template>
<xsl:template match="copyright_list | dataset_list | file_list | page_image_list | page_element_list | property_value_list">
	<a>
		<xsl:attribute name="href">
			
			<xsl:value-of select="$D_PATH"/>
			<xsl:for-each select="./item[position() &lt; last()]">
				<xsl:value-of select="."/>,
			</xsl:for-each>
			<xsl:value-of select="./item[last()]"/>
		</xsl:attribute>
		<xsl:for-each select="./item[position() &lt; last()]">
			<xsl:value-of select="."/>, &#160;
		</xsl:for-each>
		<xsl:value-of select="./item[last()]"/>
	</a>
</xsl:template>
<xsl:template match="type_list">
	<xsl:for-each select="./item[position() &lt; last()]">
		<xsl:value-of select="."/>, &#160;
	</xsl:for-each>
	<xsl:value-of select="./item[last()]"/>
</xsl:template>

</xsl:stylesheet>