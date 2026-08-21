$wimPath = "C:\Windows\System32\Recovery\winre.wim"
$mountDir = "C:\winre_mount"
$myAppSource = "$PSScriptRoot\w7repair.exe"

if (-not (Test-Path $mountDir)) { New-Item -ItemType Directory -Path $mountDir | Out-Null }

Write-Host "Stopping WinRE......"
reagentc /disable

Write-Host "Mounting winre.wim......"
dism /Mount-Image /ImageFile:$wimPath /Index:1 /MountDir:$mountDir

$targetDir = "$mountDir\MyTools"
if (-not (Test-Path $targetDir)) { New-Item -ItemType Directory -Path $targetDir | Out-Null }
Copy-Item -Path $myAppSource -Destination "$targetDir\MyRepairTool.exe" -Force

$iniPath = "$mountDir\Windows\System32\winpeshl.ini"
$iniContent = @"
[LaunchApp]
AppPath = X:\MyTools\w7repair.exe
"@
Set-Content -Path $iniPath -Value $iniContent -Encoding ASCII

Write-Host "Completing Installation......"
dism /Unmount-Image /MountDir:$mountDir /Commit

Remove-Item -Path $mountDir -Recurse -Force

reagentc /disable
reagentc /setreimage /path C:\Windows\System32\Recovery
reagentc /enable

Write-Host "Installation Complete."