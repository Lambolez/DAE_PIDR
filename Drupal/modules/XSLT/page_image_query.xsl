<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE dae_page_image [
    <!ENTITY DAE_BASE "http://dae.cse.lehigh.edu/DAE">
]>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
xmlns:ns1="http://tempuri.org"
>
<!-- Declare global variables -->
<xsl:variable name="D_PATH" select="'&DAE_BASE;/?q=descript/dataitem/'" />

<xsl:template match="/">

<!-- a lot of hard coded html stuff here -->
<html>
<head>
<title>Page Image Query Result | Document Analysis and Exploitation</title>
<link rel="shortcut icon" href="&DAE_BASE;/misc/favicon.ico" type="image/x-icon" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/sites/default/modules/dae_ui/jquery.alerts.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/modules/aggregator/aggregator.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/modules/book/book.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/modules/node/node.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/modules/system/defaults.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/modules/system/system.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/modules/system/system-menus.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/modules/user/user.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/sites/all/modules/ckeditor/ckeditor.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/sites/all/modules/simplenews/simplenews.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/sites/all/modules/tableofcontents/tableofcontents.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/sites/default/modules/ctools/css/ctools.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/sites/default/modules/dae_ui/default.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/sites/default/modules/dae_ui/stylesheets/jquery-ui.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/sites/default/modules/dae_ui/stylesheets/jquery.Jcrop.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/modules/forum/forum.css?J" />
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/sites/default/files/color/garland/style.css?J" />
<link type="text/css" rel="stylesheet" media="print" href="&DAE_BASE;/themes/garland/print.css?J" />

<script type="text/javascript" src="&DAE_BASE;/misc/jquery.js?J"></script>
<script type="text/javascript" src="&DAE_BASE;/misc/drupal.js?J"></script>
<script type="text/javascript" src="&DAE_BASE;/sites/default/modules/dae_ui/jquery.alerts.js?J"></script>
<script type="text/javascript" src="&DAE_BASE;/sites/all/modules/tableofcontents/jquery.scrollTo-min.js?J"></script>
<script type="text/javascript" src="&DAE_BASE;/sites/all/modules/tableofcontents/jquery.localscroll-min.js?J"></script>
<script type="text/javascript" src="&DAE_BASE;/sites/all/modules/tableofcontents/tableofcontents.js?J"></script>
<script type="text/javascript" src="&DAE_BASE;/sites/default/modules/dae_ui/javascript/dae_data_pages.js?J"></script>
<script type="text/javascript" src="&DAE_BASE;/sites/default/modules/dae_ui/dae_ui_effects.js?J"></script>
<script type="text/javascript" src="&DAE_BASE;/sites/default/modules/dae_ui/javascript/jquery-ui.js?J"></script>
<script type="text/javascript">
<!--//--><![CDATA[//><!--
jQuery.extend(Drupal.settings, { "basePath": "&DAE_BASE;/" });
//--><!]]>
</script>
<!--[if lt IE 7]>
<link type="text/css" rel="stylesheet" media="all" href="&DAE_BASE;/themes/garland/fix-ie.css" />    <![endif]-->
</head>


<body class="sidebar-left">

<!-- Layout -->
  <div id="header-region" class="clear-block"></div>

    <div id="wrapper">
    <div id="container" class="clear-block">

      <div id="header">
        <div id="logo-floater">

        <h1><a href="&DAE_BASE;/" title="Document Analysis and Exploitation"><img src="&DAE_BASE;/sites/default/files/new_garland_logo.png" alt="Document Analysis and Exploitation" id="logo" /><span>Document Analysis and Exploitation</span></a></h1>        </div>

                  <ul class="links primary-links"><li class="menu-120 first"><a href="&DAE_BASE;/" title="Home">Home</a></li>
</ul>                                  
      </div> <!-- /header -->

              <div id="sidebar-left" class="sidebar">
                    <div id="block-user-1" class="clear-block block block-user">

  <h2>Navigation</h2>

  <div class="content"><ul class="menu"><li class="leaf first"><a href="&DAE_BASE;/?q=browse" title="Browse the DAE database.">Browse Data</a></li>
<li class="leaf"><a href="&DAE_BASE;/?q=algorithms" title="Available Algorithms">Algorithms</a></li>

</ul></div>
</div>

        </div>
      
      <div id="center"><div id="squeeze"><div class="right-corner"><div class="left-corner">
                                        
<!-- start real xslt - xiz307 -->
<xsl:variable name="cnt" select="count(SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return/item)" />
										<h2>Page Images (<xsl:value-of select="$cnt"/>): 
<span class="dae-data-browse-functions">

<xsl:choose>
        <xsl:when test="$cnt &gt; 0">
			<div>Click to view the descriptions of the Page Images:
				<p><xsl:apply-templates select="SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:soap_call_wrapperResponse/return"/></p>
			</div>
        </xsl:when>
        <xsl:otherwise>
			<div>
			Your query returns empty result set. Try to  <strong><a style="cursor:pointer" onclick="history.go(-1)">modify your query</a></strong>.
			</div>
        </xsl:otherwise>
</xsl:choose>

	<xsl:if test="$cnt &gt; 0">
		
	</xsl:if>

	</span> 
	</h2>
<div class="clear-block">


<!-- end real xslt - xiz307 -->
			</div>
                    <div id="footer"><div id="block-system-0" class="clear-block block block-system">

  <div class="content"><a href="http://drupal.org"><img src="&DAE_BASE;/misc/powered-blue-80x15.png" alt="Powered by Drupal, an open source content management system" title="Powered by Drupal, an open source content management system" width="80" height="15" /></a></div>
</div>
</div>
      </div></div></div></div> <!-- /.left-corner, /.right-corner, /#squeeze, /#center -->

      
    </div> <!-- /container -->
  </div>
<!-- /layout -->

    </body>

</html>
</xsl:template>


<xsl:template match="return1">
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

<xsl:template match="return">
	
			
			
			<xsl:for-each select="./item[position() &lt; last()]">
				<a>
					<xsl:attribute name="href">
						<xsl:value-of select="$D_PATH"/>
						<xsl:value-of select="."/>
					</xsl:attribute>
					<xsl:value-of select="."/>
				</a>, &#160;
			</xsl:for-each>
			
			<a>
					<xsl:attribute name="href">
						<xsl:value-of select="$D_PATH"/>
						<xsl:value-of select="./item[last()]"/>
					</xsl:attribute>
					<xsl:value-of select="./item[last()]"/>
			</a>

</xsl:template>

</xsl:stylesheet>