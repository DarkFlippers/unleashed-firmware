########################################
Set-StrictMode -Version 3.0            #
$ErrorActionPreference = "Stop"        #
########################################

function Format-Bytes
{
    param(
        [int]$number
    )
    [string]$sizes = 'KB', 'MB', 'GB', 'TB', 'PB'
    [int64]$num = 0
    for ($x = 0; $x -lt $sizes.count; $x++) {
        if ($number -lt [int64]"1$( $sizes[$x] )")
        {
            if ($x -eq 0)
            {
                return "$number B"
            }
            else
            {
                $num = $number / [int64]"1$( $sizes[$x - 1] )"
                $num = "{0:N2}" -f $num
                return "$num $( $sizes[$x - 1] )"
            }
        }
    }
}

[string]$ExtraTgzName = $args[0]
[string]$DefaultZipName = $args[1]
[string]$DefaultTgzName = $args[2]
[string]$AppDir = $args[3]
[int32]$ForGithubActions = $args[4]

$Output = @{
    TGZ_NAME_EXTRA_APPS = $ExtraTgzName
    TGZ_NAME_DEFAULT_APPS = $DefaultTgzName
    ZIP_NAME_DEFAULT_APPS = $DefaultZipName
    TGZ_TAG_EXTRA_APPS = ''
    TGZ_TAG_DEFAULT_APPS = ''
    ZIP_TAG_DEFAULT_APPS = ''
}

if (!(Test-Path -Path "$AppDir/$DefaultZipName" -PathType Leaf) -or !(Test-Path -Path "$AppDir/$ExtraTgzName" -PathType Leaf) -or !(Test-Path -Path "$AppDir/$DefaultTgzName" -PathType Leaf))
{
    Write-Error '::error title=Files not found::Cannot find files in location'
    exit 1
}

$Size = (Get-Item -Path "$AppDir/$DefaultZipName" | Get-ItemPropertyValue -Name Length)
Write-Output ('Filesize: {0}' -f (Format-Bytes $Size))

$DefaultZipSize = Format-Bytes (Get-Item -Path "$AppDir/$DefaultZipName").Length
$DefaultTgzSize = Format-Bytes (Get-Item -Path "$AppDir/$DefaultTgzName").Length
$ExtraTgzSize = Format-Bytes (Get-Item -Path "$AppDir/$ExtraTgzName").Length

$Output.ZIP_TAG_DEFAULT_APPS = '{0} ({1})' -f $DefaultZipName, $DefaultZipSize
$Output.TGZ_TAG_DEFAULT_APPS = '{0} ({1})' -f $DefaultTgzName, $DefaultTgzSize
$Output.TGZ_TAG_EXTRA_APPS = '{0} ({1})' -f $ExtraTgzName, $ExtraTgzSize

if ($ForGithubActions)
{
    $Plain = New-Object -TypeName "System.Text.StringBuilder"
    $Output.GetEnumerator() | ForEach-Object {
        [void]$Plain.Append($_.Key)
        [void]$Plain.Append('=')
        [void]$Plain.AppendLine($_.Value)
    }
    Write-Output $Plain.ToString()
}
else
{
    $Output
}
