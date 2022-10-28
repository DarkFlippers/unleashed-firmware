Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"
[Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"
$repo_root = (Get-Item "$PSScriptRoot\..\..").FullName
$toolchain_version = $args[0]
$toolchain_url = "https://update.flipperzero.one/builds/toolchain/gcc-arm-none-eabi-10.3-x86_64-windows-flipper-$toolchain_version.zip"
$toolchain_zip = "gcc-arm-none-eabi-10.3-x86_64-windows-flipper-$toolchain_version.zip"
$toolchain_dir = "gcc-arm-none-eabi-10.3-x86_64-windows-flipper"

if (Test-Path -LiteralPath "$repo_root\toolchain\x86_64-windows") {
	Write-Host -NoNewline "Removing old Windows toolchain.."
	Remove-Item -LiteralPath "$repo_root\toolchain\x86_64-windows" -Force -Recurse
	Write-Host "done!"
}
if (!(Test-Path -Path "$repo_root\$toolchain_zip" -PathType Leaf)) {
    Write-Host -NoNewline "Downloading Windows toolchain.."
    $wc = New-Object net.webclient
    $wc.Downloadfile("$toolchain_url", "$repo_root\$toolchain_zip")
    Write-Host "done!"
}

if (!(Test-Path -LiteralPath "$repo_root\toolchain")) {
    New-Item "$repo_root\toolchain" -ItemType Directory
}

Write-Host -NoNewline "Extracting Windows toolchain.."
Add-Type -Assembly "System.IO.Compression.Filesystem"
[System.IO.Compression.ZipFile]::ExtractToDirectory("$repo_root\$toolchain_zip", "$repo_root\")
Move-Item -Path "$repo_root\$toolchain_dir" -Destination "$repo_root\toolchain\x86_64-windows"
Write-Host "done!"

Write-Host -NoNewline "Cleaning up temporary files.."
Remove-Item -LiteralPath "$repo_root\$toolchain_zip" -Force
Write-Host "done!"
