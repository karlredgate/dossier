<?xml version='1.0' ?>
<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform'
                xmlns:fn='http://www.w3.org/2005/02/xpath-functions'
                xmlns:xsd='http://www.w3.org/2001/XSL/XMLSchema'
                xmlns:xsi='http://www.w3.org/2001/XSL/XMLSchema-instance'
                xmlns:dc="http://purl.org/dc/elements/1.1/"
                version='1.0'>

  <dc:title></dc:title>
  <dc:creator>Karl N. Redgate</dc:creator>
  <dc:description>
  Translate smbios description to Dossier object definitions.
  </dc:description>

  <xsl:output method="text"/>

  <xsl:template match='/smbios'>
      <xsl:text>namespace SMBIOS {&#10;</xsl:text>
      <xsl:apply-templates />
      <xsl:text>}&#10;</xsl:text>
  </xsl:template>

  <xsl:template match='structure'>
      <structure type="{@type}">
          <description><xsl:value-of select='@description' /></description>
      <fields>
      <xsl:apply-templates />
      </fields>
      </structure>
  </xsl:template>

  <xsl:template match='FIELD'>
      <field offset="{@offset}" length="{@length}" value="{@usage}">
          <description><xsl:value-of select='@name' /></description>
      </field>
  </xsl:template>

</xsl:stylesheet>
<!--
  vim:autoindent
  -->
