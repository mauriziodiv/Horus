param()
$json = [Console]::In.ReadToEnd()
$fp = ($json | ConvertFrom-Json).tool_input.file_path
if ($fp -match '\.(cpp|h)$') {
    & 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe' `
        Horus.sln /p:Configuration=Debug /p:Platform=x64 /m /v:minimal /nologo
}
