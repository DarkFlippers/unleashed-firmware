########################################
Set-StrictMode -Version 3.0            #
$ErrorActionPreference = "Stop"        #
########################################

[string]$FirmwareVersion = $args[0]
[string]$ReleaseVersion = $args[1]
[string]$RepoSelf = [string]$args[2]
[string]$RepoUnleashed = $args[3]
[bool]$ForGithubActions = $false

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
    RELEASE_VERSION = $ReleaseVersion
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

$NewVersionFw = $false
Write-Host ('Latest firmware {0}' -f $FirmwareVersionNumber)

$Output.REMOTE_TAG_INFO = Write-Host ('[{0}]({1}/releases/tag/{2})' `
        -f $LatestFirmware, $RepoUnleashed, $LatestFirmware)
if (($FirmwareVersionNumber -gt $StoredFirmwareVersionNumber) -and ( $Delta -gt [TimeSpan]::FromMinutes(10)))
{
    $Output.REMOTE_TAG_INFO = $LatestFirmware
    $NewVersionFw = $true
}
elseif ($FirmwareVersionNumber -lt $StoredFirmwareVersionNumber)
{
    Write-Error ('::error title=Invalid check of stored::Version in repo: {0}, but we think it is {1}' `
            -f $FirmwareVersionNumber, $StoredFirmwareVersionNumber)
    exit 1
}

$PublishDates = (gh api -H "Accept: application/vnd.github+json" `
        -H "X-GitHub-Api-Version: 2022-11-28" "/repos/$RepoSelf/releases?per_page=1" | ConvertFrom-Json | Select-Object published_at, created_at)
$LastPublished = ($null -eq $PublishDates.published_at ? $PublishDates.created_at : $PublishDates.published_at)
$Delta = ([DateTime]::Now - $LastPublished)

$Release = (gh api -H "Accept: application/vnd.github+json" -H "X-GitHub-Api-Version: 2022-11-28" "/repos/$RepoSelf/tags?per_page=1" | ConvertFrom-Json).name
Write-Host ('Release {0}' -f $Release) -ForegroundColor Gray -BackgroundColor Blue
$LatestTag = $Release.Substring(1)

$CurrentVersion = [version]::Parse($ReleaseVersion)
$ParsedRepoVersion = [version]::Parse($LatestTag)

Write-Host ('Current tag:Repos tag {0}, {1}' -f $CurrentVersion, $ParsedRepoVersion) `
    -ForegroundColor Gray -BackgroundColor Blue
Write-Debug ('::debug Current tag:Repos tag {0}, {1}' -f $CurrentVersion, $ParsedRepoVersion)

if (($CurrentVersion -lt $ParsedRepoVersion) -and ( $Delta -gt [TimeSpan]::FromMinutes(10)))
{
    $Tag = ('{0}.{1}.{2}' -f $ParsedRepoVersion.Major, $ParsedRepoVersion.Minor, $ParsedRepoVersion.Build)

    $Output.RELEASE_VERSION = $Tag
    $Output.RELEASE_TYPE = 2

    Write-Host ('::warning title=New release!::Release {0}' -f $Tag)
}
elseif ( $NewVersionFw )
{
    $Tag = ('{0}.{1}.{2}' -f $CurrentVersion.Major, $CurrentVersion.Minor, ($CurrentVersion.Build + 1))

    $Output.RELEASE_VERSION = $Tag
    $Output.RELEASE_TYPE = 1

    Write-Host ('::warning title=Firmware was changed!::New version is {0}, need to create release {1}' -f $LatestFirmware, $Tag)
}
elseif ( ($Delta -gt [TimeSpan]::FromMinutes(10)) -and ($CurrentVersion -gt $ParsedRepoVersion))
{
    Write-Host ('::warning title=Invalid version!::Version in settings: {0}, but repo version is {1}. Going to change variable' `
        -f $CurrentVersion, $ParsedRepoVersion)

    $Output.RELEASE_VERSION = $ParsedRepoVersion
    $Output.RELEASE_TYPE = 3
}
else
{
    # none to release
    Write-Host 'No new versions, sorry'
}

$Output.CURRENT_TAG = $LatestTag

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
