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
  </dc:description>

  <xsl:variable name="UC" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ '" />
  <xsl:variable name="DN" select="'abcdefghijklmnopqrstuvwxyz-'" />
  <xsl:variable name="FN" select="'abcdefghijklmnopqrstuvwxyz_'" />

  <xsl:template match='/STRUCTUREDEFS'>
      <smbios>
      <xsl:apply-templates />
      </smbios>
  </xsl:template>

  <xsl:template match='STRUCTURE'>
      <xsl:apply-templates select='FIELD[@usage="BITFIELD"]' mode='bitfield' />
      <structure type="{@type}">
          <class-name><xsl:value-of select='translate(@description," ","")' /></class-name>
	  <display-name><xsl:value-of select='translate(@description,$UC,$DN)' /></display-name>
          <description><xsl:value-of select='@description' /></description>
      <fields>
      <xsl:apply-templates />
      </fields>
      </structure>
  </xsl:template>

  <xsl:template match='FIELD[@usage="BITFIELD"]' mode='bitfield'>
      <bitfield>
	  <display-name><xsl:value-of select='translate(@name,$UC,$DN)' /></display-name>
          <field-name><xsl:value-of select='translate(@name,$UC,$FN)' /></field-name>
          <description><xsl:value-of select='@name' /></description>
          <xsl:apply-templates />
      </bitfield>
  </xsl:template>

  <xsl:template match='FIELD'>
      <field offset="{@offset}" length="{@length}" value="{@usage}">
	  <display-name><xsl:value-of select='translate(@name,$UC,$DN)' /></display-name>
          <field-name><xsl:value-of select='translate(@name,$UC,$FN)' /></field-name>
          <description><xsl:value-of select='@name' /></description>
      </field>
  </xsl:template>

  <xsl:template match='BITS'>
      <bits start="{@lsb}" end="{@msb}">
	  <display-name><xsl:value-of select='translate(@name,$UC,$DN)' /></display-name>
          <field-name><xsl:value-of select='translate(@name,$UC,$FN)' /></field-name>
          <description><xsl:value-of select='@name' /></description>
      </bits>
  </xsl:template>

</xsl:stylesheet>
