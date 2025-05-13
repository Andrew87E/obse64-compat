# OBSE64GP Build Script
# This script cleans the build directory and runs CMake
param (
    [switch]$y = $false  # -y parameter for auto-yes
)

# Set error action preference to stop on first error
$ErrorActionPreference = "Stop"

# Store original directory
$OriginalDir = Get-Location

# Function to handle errors and return to original directory
function Handle-Error {
    param (
        [string]$ErrorMessage,
        [int]$ErrorCode = 1
    )
    
    Write-Host $ErrorMessage -ForegroundColor Red
    
    # Return to original directory
    Set-Location -Path $OriginalDir
    
    # Exit with error code
    exit $ErrorCode
}

try {
    Write-Host "===== OBSE64GP Build Script =====" -ForegroundColor Cyan

    # Get project and build directories
    $ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    $BuildDir = Join-Path $ProjectDir "build"

    Write-Host "Project directory: $ProjectDir"
    Write-Host "Build directory: $BuildDir"

    # Clean or create build directory
    if (Test-Path $BuildDir) {
        Write-Host "Cleaning build directory..." -ForegroundColor Yellow
        
        # Get all items except .gitignore
        Get-ChildItem -Path $BuildDir -Exclude ".gitignore" | ForEach-Object {
            Remove-Item -Path $_.FullName -Recurse -Force -ErrorAction Stop
        }
        
        Write-Host "Build directory cleaned." -ForegroundColor Green
    } else {
        Write-Host "Creating build directory..." -ForegroundColor Yellow
        New-Item -Path $BuildDir -ItemType Directory -ErrorAction Stop | Out-Null
        Write-Host "Build directory created." -ForegroundColor Green
    }

    # Run CMake to generate build files
    Write-Host ""
    Write-Host "Running CMake..." -ForegroundColor Cyan

    # Change to build directory
    Push-Location $BuildDir -ErrorAction Stop

    # Detect Visual Studio
    $VSVersions = @("2022", "2019", "2017")
    $VSFound = $false

    foreach ($Version in $VSVersions) {
        if (Test-Path "HKLM:\SOFTWARE\Microsoft\VisualStudio\$Version") {
            Write-Host "Detected Visual Studio $Version" -ForegroundColor Green
            & cmake .. -G "Visual Studio $Version" -A x64
            $VSFound = $true
            break
        }
    }

    if (-not $VSFound) {
        Write-Host "No specific Visual Studio version detected, using default generator" -ForegroundColor Yellow
        & cmake ..
    }

    if ($LASTEXITCODE -ne 0) {
        Handle-Error "CMake configuration failed with error code $LASTEXITCODE" $LASTEXITCODE
    }

    Write-Host ""
    Write-Host "===== Build Configuration Completed =====" -ForegroundColor Green
    Write-Host ""
    Write-Host "To build the project:" -ForegroundColor Cyan
    Write-Host " - Open the solution in Visual Studio: $BuildDir\OBSE64GP.sln"
    Write-Host " - Or run 'cmake --build . --config Release' from the build directory"
    Write-Host ""

    # Ask if the user wants to build the project now (or auto-yes)
    $BuildNow = if ($y) { "Y" } else { Read-Host "Do you want to build the project now? (Y/N)" }

    if ($BuildNow -eq "Y" -or $BuildNow -eq "y") {
        Write-Host ""
        Write-Host "Building project..." -ForegroundColor Cyan
        
        & cmake --build . --config Release
        
        if ($LASTEXITCODE -ne 0) {
            Handle-Error "Build failed with error code $LASTEXITCODE" $LASTEXITCODE
        }
        
        Write-Host ""
        Write-Host "===== Build Completed Successfully =====" -ForegroundColor Green
        Write-Host ""
        Write-Host "Executables can be found at: $BuildDir\bin\Release" -ForegroundColor Cyan
    }

    # Return to original directory
    Pop-Location
    Set-Location -Path $OriginalDir
}
catch {
    # Handle any unexpected errors
    Handle-Error "An error occurred: $_" 1
}