# Audit the Git index, including staged content. Human provenance review is still required.
[CmdletBinding()]
param()
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$files = @(& git -C $repoRoot -c core.quotepath=false ls-files)
if ($LASTEXITCODE -ne 0 -or $files.Count -eq 0) { throw 'Cannot read a nonempty Git index' }
$required = @('README.md','CONTRIBUTING.md','AGENTS.md','.gitignore','.gitattributes',
    'docs/clean-room/POLICY.md','provenance/README.md','docs/charter.md',
    'docs/architecture/overview.md','docs/adr/README.md','docs/roadmap.md',
    'docs/development/windows.md','docs/development/licensing.md',
    'scripts/toolchain-lock.json','provenance/sources.md','docs/testing/methodology.md')
$fileSet = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::Ordinal)
$caseSet = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
$contents = @{}
$allowedExtensions = @('.md','.json','.yml','.yaml','.ps1','.c','.h','.cmake','.txt','.s','.ld','.inc')
$allowedNames = @('.gitignore','.gitattributes','CODEOWNERS','CMakeLists.txt')
$forbiddenDirectories = '(^|/)(reference|private|quarantine|local|\.tools|\.downloads|build|out|artifacts)(/|$)'
$secretSignatures = '(?m)-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----|\bgh[pousr]_[A-Za-z0-9]{30,}\b|\bgithub_pat_[A-Za-z0-9_]{50,}\b|\bAKIA[0-9A-Z]{16}\b|\bsk-(?:proj-)?[A-Za-z0-9_-]{40,}\b'
foreach ($path in $files) {
    if ($path -notmatch '^[A-Za-z0-9._/-]+$') { throw "Unexpected path characters: $path" }
    if (-not $caseSet.Add($path)) { throw "Case-colliding tracked path: $path" }
    [void]$fileSet.Add($path)
    if ($path -match $forbiddenDirectories) { throw "Excluded payload directory tracked: $path" }
    $leaf = [IO.Path]::GetFileName($path)
    $extension = [IO.Path]::GetExtension($path).ToLowerInvariant()
    if ($allowedExtensions -notcontains $extension -and $allowedNames -notcontains $leaf) {
        throw "Non-source/text payload needs explicit policy review: $path"
    }
    if ($leaf -match '^(\.env($|\.)|credentials|secrets|id_rsa|id_ed25519)') {
        throw "Secret-like filename tracked: $path"
    }
    $indexSize = & git -C $repoRoot cat-file -s ":$path"
    if ($LASTEXITCODE -ne 0 -or [long]$indexSize -gt 1048576) { throw "Unexpected index size: $path" }
    $content = (& git -C $repoRoot show ":$path") -join "`n"
    if ($LASTEXITCODE -ne 0) { throw "Cannot read indexed file: $path" }
    if ($content.Contains([char]0)) { throw "Binary content in text file: $path" }
    if ($content -match $secretSignatures) { throw "Possible credential in $path (value withheld)" }
    if ($extension -eq '.json') { $null = $content | ConvertFrom-Json }
    $contents[$path] = $content
}
foreach ($path in $required) {
    if (-not $fileSet.Contains($path)) { throw "Required bootstrap document missing: $path" }
}
foreach ($path in $files | Where-Object { $_ -like '*.md' }) {
    foreach ($match in [regex]::Matches($contents[$path], '(?<!!)\[[^\]]*\]\((?<target>[^)\s]+)\)')) {
        $target = $match.Groups['target'].Value
        if ($target -match '^[A-Za-z][A-Za-z0-9+.-]*:' -or $target.StartsWith('#')) { continue }
        $target = [Uri]::UnescapeDataString(($target -split '[#?]', 2)[0])
        if (-not $target) { continue }
        $parent = Split-Path -Parent (Join-Path $repoRoot $path)
        $absolute = [IO.Path]::GetFullPath((Join-Path $parent $target))
        $prefix = $repoRoot.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
        if (-not $absolute.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) {
            throw "Local documentation link leaves repository: $path -> $target"
        }
        $relative = $absolute.Substring($prefix.Length).Replace('\','/')
        if (-not $fileSet.Contains($relative)) { throw "Broken indexed documentation link: $path -> $target" }
    }
}
& git -C $repoRoot diff --cached --check
if ($LASTEXITCODE -ne 0) { throw 'Staged whitespace check failed' }
Write-Output "Repository guard PASS: $($files.Count) indexed text/source files; required docs, local links, JSON, payload exclusions, and common credential signatures checked."
Write-Output 'Originality, reference rights, and full secret detection still require human review.'
