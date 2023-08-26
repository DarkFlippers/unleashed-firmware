########################################
Set-StrictMode -Version 3.0            #
$ErrorActionPreference = "Stop"        #
########################################

function Format-Bytes {
    param(
        [int]$number
    )
    [string]$sizes = 'KB', 'MB', 'GB', 'TB', 'PB'
    [int64]$num = 0
    for ($x = 0; $x -lt $sizes.count; $x++) {
        if ($number -lt [int64]"1$($sizes[$x])") {
            if ($x -eq 0) {
                return "$number B"
            }
            else {
                $num = $number / [int64]"1$($sizes[$x-1])"
                $num = "{0:N2}" -f $num
                return "$num $($sizes[$x-1])"
            }
        }
    }
}

[string]$ZipName = $args[0]
[string]$TgzName = $args[1]
[string]$AppDir = $args[2]
[int32]$ForGithubActions = $args[3]

$Output = @{
    ZIP_NAME = $ZipName
    TGZ_NAME = $TgzName
    ZIP_TAG  = ''
    TGZ_TAG  = ''
}

$Size = (Get-Item -Path "$AppDir/$ZipName" | Get-ItemPropertyValue -Name Length)
Write-Output ('Filesize: {0}' -f (Format-Bytes $Size))

$ZipSize = Format-Bytes (Get-Item -Path "$AppDir/$ZipName").Length
$TgzSize = Format-Bytes (Get-Item -Path "$AppDir/$TgzName" ).Length

$Output.ZIP_TAG = '{0} ({1})' -f $ZipName, $ZipSize
$Output.TGZ_TAG = '{0} ({1})' -f $TgzName, $TgzSize

if ($ForGithubActions) {
    $Plain = New-Object -TypeName "System.Text.StringBuilder"
    $Output.GetEnumerator() | ForEach-Object {
        [void]$Plain.Append($_.Key)
        [void]$Plain.Append('=')
        [void]$Plain.AppendLine($_.Value)
    }
    Write-Output $Plain.ToString()
}
else {
    $Output
}
