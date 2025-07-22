@echo off
setlocal enabledelayedexpansion

set GLSLC= C:\VulkanSDK\1.4.304.1\Bin\glslc.exe

set SHADER_DIR=shaders
set OUTPUT_DIR=shaders\compiledShaders

echo Compiling all shaders in %SHADER_DIR% ...

REM Loop through all shader files recursively
for /r "%SHADER_DIR%" %%f in (*.vert *.frag *.comp *.geom *.tesc *.tese) do (
    set FILENAME=%%~nf
    set EXT=%%~xf
    set OUTPUT=%OUTPUT_DIR%\!FILENAME!!EXT!.spv

    echo Compiling %%f -> !OUTPUT!
    %GLSLC% %%f -o !OUTPUT!

    if errorlevel 1 (
        echo [ERROR] Failed to compile %%f
        exit /b 1
    )
)

echo All shaders compiled successfully!
exit /b 0