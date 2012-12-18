<?xml version='1.0' ?>
<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform'
                xmlns:fn='http://www.w3.org/2005/02/xpath-functions'
                xmlns:xsd='http://www.w3.org/2001/XSL/XMLSchema'
                xmlns:xsi='http://www.w3.org/2001/XSL/XMLSchema-instance'
                xmlns:dc="http://purl.org/dc/elements/1.1/"
                version='1.0'>

  <xsl:output method="text"/>

  <dc:title></dc:title>
  <dc:creator>Karl N. Redgate</dc:creator>
  <dc:description>
  </dc:description>

  <xsl:template match='text()' />

  <xsl:template match='/smbios'>
      <xsl:text>SMBIOS </xsl:text>
      <xsl:value-of select='@version' />
      <xsl:text>present.&#10;</xsl:text>

      <xsl:text>N structures occupying N bytes.&#10;</xsl:text>

      <xsl:text>Table at 0x00000000.&#10;</xsl:text>

      <xsl:apply-templates />
      <xsl:value-of select='explanation' /><xsl:text>&#10;</xsl:text>
  </xsl:template>

  <xsl:template match='structure'>
      <xsl:text>&#10;Handle </xsl:text>
      <xsl:value-of select='@handle' />
      <xsl:text>, DMI type </xsl:text>
      <xsl:value-of select='@type' />
      <xsl:text>, N</xsl:text>
      <xsl:text> bytes&#10;</xsl:text>
      <xsl:value-of select='name' /><xsl:text>&#10;</xsl:text>
  </xsl:template>

</xsl:stylesheet>
