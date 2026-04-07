$repoRoot = Split-Path -Parent $PSScriptRoot
$java = "C:\Users\A-A-Ron\.mplab\app-finder\apps\zulu-jre-21\v21.0.9\zulu21.46.19-ca-jre21.0.9-win_x64\bin\java.exe"
$mdbBase = "C:\Users\A-A-Ron\.mplab\app-finder\apps\mplab_backend\0.2.655\mplab_backend\mdbcore"
$loader = "$mdbBase\modules\com-microchip-mplab-mdbcore-loader.jar"
$script = Join-Path $repoRoot "mdb_program.txt"

& $java -jar $loader $script
