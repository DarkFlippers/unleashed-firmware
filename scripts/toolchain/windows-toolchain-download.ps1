Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"
[Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"
# TODO FL-3536: fix path to download_dir
$download_dir = (Get-Item "$PSScriptRoot\..\..").FullName
$toolchain_version = $args[0]
$toolchain_target_path = $args[1]

$toolchain_url = "https://update.flipperzero.one/builds/toolchain/gcc-arm-none-eabi-12.3-x86_64-windows-flipper-$toolchain_version.zip"
$toolchain_dist_folder = "gcc-arm-none-eabi-12.3-x86_64-windows-flipper"
$toolchain_zip = "$toolchain_dist_folder-$toolchain_version.zip"

$toolchain_zip_temp_path = "$download_dir\$toolchain_zip"
$toolchain_dist_temp_path = "$download_dir\$toolchain_dist_folder"

try {

if (Test-Path -LiteralPath "$toolchain_target_path") {
	Write-Host -NoNewline "Removing old Windows toolchain.."
	Remove-Item -LiteralPath "$toolchain_target_path" -Force -Recurse
	Write-Host "done!"
}

if (Test-path -LiteralPath  "$toolchain_target_path\..\current") {
	Write-Host -NoNewline "Unlinking 'current'.."
    Remove-Item -LiteralPath "$toolchain_target_path\..\current" -Force
	Write-Host "done!"
}

if (!(Test-Path -LiteralPath "$toolchain_zip_temp_path" -PathType Leaf)) {
    Write-Host -NoNewline "Downloading Windows toolchain.."
    $wc = New-Object net.webclient
    $wc.Downloadfile("$toolchain_url", "$toolchain_zip_temp_path")
    Write-Host "done!"
}

if (!(Test-Path -LiteralPath "$toolchain_target_path\..")) {
    New-Item "$toolchain_target_path\.." -ItemType Directory -Force
}

if (Test-Path -LiteralPath "$toolchain_dist_temp_path") {
    Write-Host "Cleaning up temp toolchain path.."
    Remove-Item -LiteralPath "$toolchain_dist_temp_path" -Force -Recurse
}

Write-Host -NoNewline "Extracting Windows toolchain.."
# This is faster than Expand-Archive
Add-Type -Assembly "System.IO.Compression.Filesystem"
[System.IO.Compression.ZipFile]::ExtractToDirectory("$toolchain_zip_temp_path", "$download_dir")
# Expand-Archive -LiteralPath "$toolchain_zip_temp_path" -DestinationPath "$download_dir"

Write-Host -NoNewline "moving.."
Move-Item -LiteralPath "$toolchain_dist_temp_path" -Destination "$toolchain_target_path" -Force
Write-Host -NoNewline "linking to 'current'.."
cmd /c mklink /J "$toolchain_target_path\..\current" "$toolchain_target_path"
Write-Host "done!"

Write-Host -NoNewline "Cleaning up temporary files.."
Remove-Item -LiteralPath "$toolchain_zip_temp_path" -Force
Write-Host "done!"

} catch {
    Write-Host "An error occurred"
    Write-Host $_
    Write-Host "Please close VSCode and any other programs that may be using the toolchain and try again."
    $host.SetShouldExit(1)
    Exit 1
}
