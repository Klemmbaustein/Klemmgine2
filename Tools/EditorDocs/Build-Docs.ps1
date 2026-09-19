
param (
	[string] $OutPath,
	[string] $InPath
)

$ErrorActionPreference = "Stop"

function Format-TextString([string] $text) {
	return $text -ne "" ? ($text -replace '\s+', ' ').trim() : $null
}

function Convert-DocsFunction($xmlData) {
	$result = [ordered] @{
		"description" = Format-TextString -text $xmlData.description
	}

	if ($xmlData.return -ne $null) {
		$result["return"] = Format-TextString -text $xmlData.return.InnerXml
		$result["returnType"] = $xmlData.return.type
	}

	$args = [ordered]@{}

	foreach ($arg in $xmlData.argument) {
		$args[$arg.name] = Format-TextString -text $arg.InnerXml
	}
	$result["arguments"] = $args

	return $result
}

function Convert-DocsType($xmlData) {

	$methods = @{}
	$members = @{}

	foreach ($item in $xmlData.methods.ChildNodes) {
		$methods[$item.name] = Convert-DocsFunction $item
	}
	foreach ($item in $xmlData.members.ChildNodes) {
		$members[$item.name] = @{
			description = Format-TextString -text $item.description
			type = $item.type
		}
	}

	$result = @{
		"description" = Format-TextString -text $xmlData.description
		"methods" = $methods
		"members" = $members
		"super" = $xmlData.super
	}

	return $result
}

function Convert-DocFile([string] $file) {
	$xml = [xml](Get-Content $file)

	$result = @{}

	$result["type"] = $xml.docs.type
	$result["module"] = $xml.docs.module

	$functions = @{}
	$types = @{}

	foreach ($item in $xml.docs.functions.ChildNodes) {
		$functions[$item.name] = Convert-DocsFunction $item
	}

	foreach ($item in $xml.docs.types.ChildNodes) {
		$types[$item.name] = Convert-DocsType $item
	}

	$result["functions"] = $functions
	$result["types"] = $types
	return $(ConvertTo-Json $result -Depth 10)
}

foreach ($file in Get-ChildItem $InPath -filter *.xml)
{
	Convert-DocFile -file $file | Out-File ("$OutPath/$($file.name)" -replace ".xml", ".json")
}