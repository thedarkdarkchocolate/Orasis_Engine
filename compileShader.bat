C:\VulkanSDK\1.4.304.1\Bin\glslc.exe shaders\shader.vert -o shaders\compiledShaders\shader.vert.spv
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe shaders\shader.frag -o shaders\compiledShaders\shader.frag.spv

if %ERRORLEVEL% NEQ 0 (
    echo Shader compilation failed!
    exit /b 1
)