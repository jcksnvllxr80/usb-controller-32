$ErrorActionPreference = "Stop"

$port = New-Object System.IO.Ports.SerialPort 'COM3', 115200, 'None', 8, 'One'
$port.ReadTimeout = 2000
$port.WriteTimeout = 2000
$port.DtrEnable = $false
$port.RtsEnable = $false

try {
    $port.Open()
    Write-Host "Port opened successfully"

    Start-Sleep -Milliseconds 500
    $port.DiscardInBuffer()

    function Send-Command($cmd, $wait = 1500) {
        $port.DiscardInBuffer()
        $bytes = [System.Text.Encoding]::ASCII.GetBytes("$cmd`r`n")
        $port.Write($bytes, 0, $bytes.Length)
        Start-Sleep -Milliseconds $wait
        $n = $port.BytesToRead
        Write-Host "=== $cmd ($n bytes) ==="
        if ($n -gt 0) {
            $buf = New-Object byte[] $n
            [void]$port.Read($buf, 0, $n)
            $text = [System.Text.Encoding]::ASCII.GetString($buf)
            Write-Host $text
        }
    }

    Send-Command "ping"
    Send-Command "usb"
    Send-Command "enum"
    Send-Command "usblog" 3000
    Send-Command "status"

    $port.Close()
    $port.Dispose()
    Write-Host "Port closed cleanly"
} catch {
    Write-Host "Error: $($_.Exception.Message)"
    try { $port.Close() } catch {}
    try { $port.Dispose() } catch {}
}
