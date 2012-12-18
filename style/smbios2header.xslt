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

  <xsl:variable name="smallcase" select="'abcdefghijklmnopqrstuvwxyz'" />
  <xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />

  <xsl:template match='text()' />

  <xsl:template match='/smbios'>
      <xsl:text>namespace SMBIOS {&#10;</xsl:text>
      <xsl:apply-templates />
      <xsl:text>}&#10;</xsl:text>
  </xsl:template>

  <xsl:template match='structure'>
      <xsl:text>class </xsl:text>
      <xsl:value-of select='class-name' />
      <xsl:text> : public Structure {&#10;</xsl:text>
      <xsl:apply-templates />
      <xsl:text>};&#10;</xsl:text>
  </xsl:template>

  <xsl:template match='field[@value="STRUCTURE_TYPE"]' />
  <xsl:template match='field[display-name="length"]' />
  <xsl:template match='field[display-name="handle"]' />

  <xsl:template match='field[@value="STRING"]'>
      <xsl:text>  inline const char *</xsl:text>
      <xsl:value-of select='field-name' />
      <xsl:text>() const { string(</xsl:text>
      <xsl:value-of select='@offset' />
      <xsl:text>); }&#10;</xsl:text>
  </xsl:template>

  <xsl:template match='field'>
      <xsl:text>#error unhandled field type </xsl:text>
      <xsl:value-of select='@value' />
      <xsl:text> name </xsl:text>
      <xsl:value-of select='field-name' />
      <xsl:text>&#10;</xsl:text>
<!--
      <field offset="{@offset}" length="{@length}" value="{@usage}">
          <description><xsl:value-of select='@name' /></description>
      </field>
-->
  </xsl:template>

</xsl:stylesheet>
<!--
  vim:autoindent
  -->
