#include <libxml/HTMLparser.h>
#include <libxml/uri.h>
#include <libxml/xpath.h>
#include <stdio.h>
#include <string.h>

const char *html = R"(
<!DOCTYPE html>
<html>
  <head>
    <title>Jianwei Xie</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="style.css">
    <meta name="format-detection" content="telephone=no">
  </head>

  <style>
      ul { margin:0; }
      dt { font-style: italic; }
  </style>

<body>

<!-- ---------------------------------------------------------------------------
  Index
-->

<h4>Jianwei Xie</h4>
<dl>
  <dt>2022/02 - now</dt>
  <dd>
      Databricks
      <ul>
        <li>Y3: Uber TL in AI Training Org</li>
        <li>Y2: Uber TL on LLM platform </li>
        <li>Y1: TL in ML Inference team </li>
      </ul>
  </dd>

  <dt>2014/05 - 2022/02</dt>
  <dd>
    <a href="goog.html">Google</a>
  </dd>

  <dt>1999/09 - 2014/05</dt>
  <dd>
    <a href="edu.html">Education</a>
  </dd>

  <dt style="padding-top: 5px;"><emph>etc</emph></dt>
  <dd>
    <a href="etc/etc.html">etc...</a>
  </dd>

</dl>

</body>
</html>
)";

// Helper: evaluate an XPath expression
xmlXPathObjectPtr
getnodeset( xmlDocPtr doc, const xmlChar *xpathExpr )
{
    xmlXPathContextPtr context = xmlXPathNewContext( doc );
    if ( context == NULL ) return NULL;

    xmlXPathObjectPtr result = xmlXPathEvalExpression( xpathExpr, context );
    xmlXPathFreeContext( context );

    if ( result == NULL || xmlXPathNodeSetIsEmpty( result->nodesetval ) ) {
        if ( result ) xmlXPathFreeObject( result );
        return NULL;
    }
    return result;
}

int
main( )
{
    // Parse HTML (tolerant parser)
    htmlDocPtr doc =
        htmlReadMemory( html, (int)strlen( html ),
                        "http://xiejw.xieqi.org/",  // base URL
                        NULL,                       // encoding
                        HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR );

    if ( doc == NULL ) {
        fprintf( stderr, "Failed to parse HTML\n" );
        return 1;
    }

    // Find all <a> elements with href
    xmlXPathObjectPtr result = getnodeset( doc, (const xmlChar *)"//a/@href" );
    if ( result ) {
        xmlNodeSetPtr nodeset = result->nodesetval;
        for ( int i = 0; i < nodeset->nodeNr; i++ ) {
            xmlChar *href = xmlNodeListGetString(
                doc, nodeset->nodeTab[i]->xmlChildrenNode, 1 );
            if ( href ) {
                printf( "Link: %s\n", href );
                xmlFree( href );
            }
        }
        xmlXPathFreeObject( result );
    }

    xmlFreeDoc( doc );
    xmlCleanupParser( );
    return 0;
}
