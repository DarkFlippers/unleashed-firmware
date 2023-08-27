<#
    Create or update release with given tag. Also upload files.

    GH_TOKEN is mandatory!
#>

########################################
Set-StrictMode -Version 3.0            #
$ErrorActionPreference = "Stop"        #
########################################

[string]$InputDir = $args[0]
[string]$FirmwareVersion = $args[1]
[string]$Repo = $args[2]
[string]$Branch = $args[3]

$Files = @(Get-ChildItem -Path "$InputDir/*" -File -Include "*.tgz", "*.zip" -ErrorAction SilentlyContinue)
if ($Files.Count -eq 0)
{
    Write-Error ('::error title=Files not found::Cannot READ files in location: {0}' -f $InputDir)
    exit 2
}

Write-Host "Check if release exists"
gh release create $FirmwareVersion -R $Repo --generate-notes --target $Branch --title "$FirmwareVersion branch $Branch"
if ($LASTEXITCODE -gt 1)
{
    Write-Error ('::error title=Cannot create or check release::Tag: {0}, Repo: {1}, ErrorCode: {2}' -f $FirmwareVersion, $Repo, $LASTEXITCODE)
    exit 255
}

Write-Host "Start upload"
cd $InputDir
$Files | ForEach-Object {
    Write-Host ('Uploading {0}' -f $_.Name)
    gh release upload $FirmwareVersion $_.Name -R $Repo --clobber
}

Write-Host "Release edit"
gh release edit $FirmwareVersion -R $Repo --latest --target $Branch

cd -
