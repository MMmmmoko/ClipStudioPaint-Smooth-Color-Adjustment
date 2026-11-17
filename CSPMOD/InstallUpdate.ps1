param(
    [int]$HostPid,
    [string]$InstallDirectory,
    [string]$ArchivePath,
    [string]$JobId,
    [string]$Digest,
    [string]$DialogTitle,
    [string]$FailureMessage,
    [string]$RestartFailureMessage
)

$ErrorActionPreference = 'Stop'
$exitCode = 2
$ready = $cancel = $hostProcess = $archive = $zip = $lock = $null
$files = @()
$createdDirectories = @()
$recovery = @()
$installed = $false
$hostExited = $false

function Get-SafeAttributes([string]$Path) {
    try {
        $attributes = [IO.File]::GetAttributes($Path)
    } catch [IO.FileNotFoundException], [IO.DirectoryNotFoundException] {
        return $null
    }
    if ($attributes -band [IO.FileAttributes]::ReparsePoint) {
        throw "Unexpected reparse point: $Path"
    }
    return $attributes
}

function Test-SafeDirectory([string]$Path) {
    $attributes = Get-SafeAttributes $Path
    if ($null -ne $attributes -and -not ($attributes -band [IO.FileAttributes]::Directory)) {
        throw "Expected a directory: $Path"
    }
    return ($null -ne $attributes)
}

function Show-UpdateError([string]$Message) {
    Add-Type -AssemblyName System.Windows.Forms
    $null = [Windows.Forms.MessageBox]::Show($Message, $DialogTitle,
        [Windows.Forms.MessageBoxButtons]::OK, [Windows.Forms.MessageBoxIcon]::Error)
}

try {
    $ready = [Threading.EventWaitHandle]::OpenExisting("Local\CSPMOD-ready-$JobId")
    $cancel = [Threading.EventWaitHandle]::OpenExisting("Local\CSPMOD-cancel-$JobId")
    $hostProcess = [Diagnostics.Process]::GetProcessById($HostPid)
    $null = $hostProcess.Handle
    $hostExecutable = $hostProcess.MainModule.FileName

    Add-Type -AssemblyName System.IO.Compression
    $archive = [IO.File]::OpenRead($ArchivePath)
    if ($Digest) {
        $sha = [Security.Cryptography.SHA256]::Create()
        try {
            $actual = [BitConverter]::ToString($sha.ComputeHash($archive)).Replace('-', '')
            if ($actual -ne $Digest) { throw 'Update checksum does not match.' }
            $archive.Position = 0
        } finally { $sha.Dispose() }
    }
    $zip = [IO.Compression.ZipArchive]::new($archive, [IO.Compression.ZipArchiveMode]::Read, $true)

    $allowed = '^(CSPMOD\.dll|CSPPatchTool\.exe|PlugIn/PAINT/(AAload|SmoothFilter_[A-Za-z0-9_]+)\.dll)$'
    $entries = @{}
    $packageRoot = $null
    $expandedSize = 0L
    foreach ($entry in $zip.Entries) {
        $name = $entry.FullName.Replace('\', '/')
        if ($name.EndsWith('/')) { continue }
        $root = ''
        if ($name -match '^(CSPMOD_[A-Za-z0-9_.-]+)/(.+)$') {
            $root = $Matches[1]
            $name = $Matches[2]
        }
        if ($name -notmatch $allowed) { continue }
        if ($null -eq $packageRoot) { $packageRoot = $root }
        $expandedSize += $entry.Length
        if ($root -ne $packageRoot -or $entries.ContainsKey($name) -or
            $entry.Length -lt 2 -or $entry.Length -gt 128MB -or $expandedSize -gt 256MB -or
            $entries.Count -ge 32) {
            throw 'Invalid update archive.'
        }
        $entries.Add($name, $entry)
    }
    if (-not $entries.ContainsKey('CSPMOD.dll')) { throw 'CSPMOD.dll is missing.' }

    $exitCode = 3
    if (-not (Test-SafeDirectory $InstallDirectory)) { throw 'Installation directory is missing.' }
    $lockPath = [IO.Path]::Combine($InstallDirectory, '.cspmod-update.lock')
    $null = Get-SafeAttributes $lockPath
    $lock = [IO.FileStream]::new($lockPath, [IO.FileMode]::OpenOrCreate, [IO.FileAccess]::ReadWrite,
        [IO.FileShare]::None, 4096, [IO.FileOptions]::DeleteOnClose)

    # Stage each candidate on the destination volume before asking the host to close.
    foreach ($name in ($entries.Keys | Sort-Object)) {
        if ($cancel.WaitOne(0)) { $exitCode = 5; return }
        $target = [IO.Path]::Combine($InstallDirectory, $name.Replace('/', '\'))
        $directory = $InstallDirectory
        $parts = $name.Split('/')
        for ($index = 0; $index -lt $parts.Length - 1; $index++) {
            $directory = [IO.Path]::Combine($directory, $parts[$index])
            if (-not (Test-SafeDirectory $directory)) {
                $null = [IO.Directory]::CreateDirectory($directory)
                $createdDirectories += $directory
            }
        }
        $attributes = Get-SafeAttributes $target
        if ($null -ne $attributes -and ($attributes -band
                ([IO.FileAttributes]::Directory -bor [IO.FileAttributes]::ReadOnly))) {
            throw "Update target is not writable: $target"
        }
        $file = [pscustomobject]@{
            Target = $target
            Stage = "$target.$JobId.new"
            Backup = "$target.$JobId.old"
            Staged = $false
            BackedUp = $false
            Replaced = $false
        }
        $files += $file
        $entry = $entries[$name]
        $inputStream = $entry.Open()
        try {
            $exitCode = 2
            if ($inputStream.ReadByte() -ne 77 -or $inputStream.ReadByte() -ne 90) {
                throw "Invalid Windows binary: $name"
            }
            $exitCode = 3
            $outputStream = [IO.File]::Open($file.Stage, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write)
            $file.Staged = $true
            try {
                $outputStream.WriteByte(77)
                $outputStream.WriteByte(90)
                $buffer = [byte[]]::new(65536)
                while (($read = $inputStream.Read($buffer, 0, $buffer.Length)) -gt 0) {
                    if ($cancel.WaitOne(0)) { $exitCode = 5; return }
                    if ($read -gt ($entry.Length - $outputStream.Length)) { throw 'Invalid ZIP entry size.' }
                    $outputStream.Write($buffer, 0, $read)
                }
                if ($outputStream.Length -ne $entry.Length) { throw 'Incomplete ZIP entry.' }
                $outputStream.Flush($true)
            } finally { $outputStream.Dispose() }
        } finally { $inputStream.Dispose() }
    }

    $zip.Dispose()
    $zip = $null
    $archive.Dispose()
    $archive = $null
    [IO.File]::Delete($ArchivePath)

    $null = $ready.Set()
    $exitCode = 5
    $deadline = [DateTime]::UtcNow.AddMinutes(10)
    do {
        if ($cancel.WaitOne(0) -or [DateTime]::UtcNow -ge $deadline) { return }
        $hostExited = $hostProcess.WaitForExit(200)
    } until ($hostExited)
    if ($cancel.WaitOne(0)) { return }

    $exitCode = 4
    foreach ($file in $files) {
        $null = Test-SafeDirectory ([IO.Path]::GetDirectoryName($file.Target))
        if ($null -ne (Get-SafeAttributes $file.Target)) {
            [IO.File]::Move($file.Target, $file.Backup)
            $file.BackedUp = $true
        }
        [IO.File]::Move($file.Stage, $file.Target)
        $file.Replaced = $true
    }
    $installed = $true
    $exitCode = 0
} catch {
    [Console]::Error.WriteLine($_.Exception.Message)
} finally {
    for ($index = $files.Count - 1; $index -ge 0; $index--) {
        $file = $files[$index]
        try {
            if ($installed) {
                if ($file.BackedUp) { [IO.File]::Delete($file.Backup) }
            } else {
                if ($file.Replaced) { [IO.File]::Delete($file.Target) }
                if ($file.BackedUp) { [IO.File]::Move($file.Backup, $file.Target) }
            }
        } catch {
            if ($file.BackedUp) { $recovery += $file.Backup }
        }
        if ($file.Staged) {
            try { [IO.File]::Delete($file.Stage) } catch {}
        }
    }
    if (-not $installed) {
        for ($index = $createdDirectories.Count - 1; $index -ge 0; $index--) {
            try { [IO.Directory]::Delete($createdDirectories[$index]) } catch {}
        }
    }
    foreach ($item in @($zip, $archive, $lock, $hostProcess, $ready, $cancel)) {
        if ($null -ne $item) { $item.Dispose() }
    }
    try {
        [IO.File]::Delete($ArchivePath)
        [IO.Directory]::Delete([IO.Path]::GetDirectoryName($ArchivePath))
    } catch {}
    if ($installed) {
        try {
            $start = [Diagnostics.ProcessStartInfo]::new($hostExecutable)
            $start.WorkingDirectory = [IO.Path]::GetDirectoryName($hostExecutable)
            $start.UseShellExecute = $false
            $restarted = [Diagnostics.Process]::Start($start)
            $restarted.Dispose()
        } catch {
            $exitCode = 6
            [Console]::Error.WriteLine($_.Exception.Message)
            Show-UpdateError $RestartFailureMessage
        }
    } elseif ($hostExited -and $exitCode -eq 4) {
        $message = $FailureMessage
        if ($recovery.Count) { $message += "`n`n" + ($recovery -join "`n") }
        Show-UpdateError $message
    }
    exit $exitCode
}
