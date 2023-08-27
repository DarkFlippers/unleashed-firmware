########################################
Set-StrictMode -Version 3.0            #
$ErrorActionPreference = "Stop"        #
########################################

[string]$FirmwareVersion = $args[0]
[string]$RepoUnleashed = $args[1]
[bool]$ForGithubActions = $true

################################################################################################################################
function CleanInput
{
    param(
        [string]
        $DurtyString
    )
    return $DurtyString -replace ('[^a-zA-Z\d_\-\,\.\t\n\r\:\;]', '')
}

################################################################################################################################

$Output = @{
    CURRENT_TAG = $FirmwareVersion
    REMOTE_TAG_INFO = $FirmwareVersion
    RELEASE_TYPE = 0
}

$Release = @(`
    (CleanInput `
        (gh release list -L 1 --repo $RepoUnleashed)`
    ) -split "`t")

$FirmwareVersionNumber = 0
$StoredFirmwareVersionNumber = 0
$LatestFirmware = $Release[2]
if ($Release[2] -match '\-(\d+)$')
{
    $FirmwareVersionNumber = [int]($Matches[1])
}
else
{
    Write-Error ('::error title=Invalid firmware number::Error during execution this tags {0}' -f $FirmwareVersionNumber)
    exit 1
}
if ($FirmwareVersion -match '\-(\d+)$')
{
    $StoredFirmwareVersionNumber = [int]($Matches[1])
}
else
{
    Write-Error ('::error title=Invalid STORED firmware number::Error during execution this version {0}' -f $FirmwareVersion)
    exit 1
}
$Delta = ( [DateTime]::Now - [DateTime]::Parse($Release[3]))
Write-Host "FirmwareVersionNumber: $FirmwareVersionNumber, Delta: $Delta"
#exit 0

Write-Host ('Latest firmware {0}' -f $FirmwareVersionNumber)

$Output.REMOTE_TAG_INFO = Write-Host ('[{0}]({1}/releases/tag/{2})' `
        -f $LatestFirmware, $RepoUnleashed, $LatestFirmware)
if (($FirmwareVersionNumber -gt $StoredFirmwareVersionNumber) -and ( $Delta -gt [TimeSpan]::FromMinutes(10)))
{
    $Output.REMOTE_TAG_INFO = $LatestFirmware
    $Output.RELEASE_TYPE = 1
}
elseif ($FirmwareVersionNumber -lt $StoredFirmwareVersionNumber)
{
    Write-Error ('::error title=Invalid check of stored::Version in repo: {0}, but we think it is {1}' `
            -f $FirmwareVersionNumber, $StoredFirmwareVersionNumber)
    $Output.RELEASE_TYPE = 0
    exit 1
}
else
{
    # none to release
    Write-Host 'No new versions, sorry'
}

if ($ForGithubActions)
{
    $Plain = New-Object -TypeName "System.Text.StringBuilder";
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
