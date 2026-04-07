param([string]$cmd = "ping")
$ErrorActionPreference = "Stop"

$port = New-Object System.IO.Ports.SerialPort 'COM3', 115200, 'None', 8, 'One'
$port.ReadTimeout = 5000
$port.WriteTimeout = 2000
$port.DtrEnable = $false
$port.RtsEnable = $false

try {
    $port.Open()
    Start-Sleep -Milliseconds 300
    $port.DiscardInBuffer()

    $bytes = [System.Text.Encoding]::ASCII.GetBytes("$cmd`r`n")
    $port.Write($bytes, 0, $bytes.Length)
    Start-Sleep -Milliseconds 2000

    $n = $port.BytesToRead
    if ($n -gt 0) {
        $buf = New-Object byte[] $n
        [void]$port.Read($buf, 0, $n)
        Write-Host ([System.Text.Encoding]::ASCII.GetString($buf))
    }

    $port.Close()
    $port.Dispose()
} catch {
    Write-Host "Error: $($_.Exception.Message)"
    try { $port.Close() } catch {}
    try { $port.Dispose() } catch {}
}
