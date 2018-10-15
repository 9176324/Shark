<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">
<xsl:script>
	var g_nSeq = 0;
	function getSeqNum()
	{
		var strSeqNum = "0000000" + ((g_nSeq++ + 0x100000000).toString(16));
		return strSeqNum.substring(strSeqNum.length - 8);
	}
</xsl:script>
<xsl:template match="/">
<xsl:for-each select="DEFECTS/DEFECT[DEFECTCODE != 98101]" xml:space="preserve"><xsl:eval>getSeqNum()</xsl:eval> <xsl:value-of select="SFA/LINE" />	<xsl:value-of select="SFA/COLUMN" />	<xsl:value-of select="SFA/FILENAME" />	<xsl:value-of select="SFA/FILEPATH" />	<xsl:value-of select="DEFECTCODE" />	<xsl:apply-templates select="DESCRIPTION" />	<xsl:value-of select="RANK" />	<xsl:value-of select="MODULE" />	<xsl:value-of select="RUNID" />	<xsl:apply-templates select="FUNCTION" />	<xsl:value-of select="FUNCLINE" />	<xsl:for-each select="PATH/SFA" xml:space="preserve"><xsl:value-of select="LINE" />,<xsl:value-of select="COLUMN" />,<xsl:value-of select="FILENAME" />,<xsl:value-of select="FILEPATH" />;</xsl:for-each>
</xsl:for-each>
</xsl:template>
<xsl:template match="DESCRIPTION"><xsl:eval no-entities="t">this.text.replace(/\s/gm, " ")</xsl:eval></xsl:template>
<xsl:template match="FUNCTION"><xsl:eval no-entities="t">this.text</xsl:eval></xsl:template>
</xsl:stylesheet>
