# Harfang C++ binding to Emscripten JS
Example to show how to build a small binding from Harfang 3D engine (https://github.com/scorpheus/harfang3d/tree/main) to JS
Download the project https://github.com/ejulien/FABGen to compile Harfang3D.

# Demo
https://scorpheus.github.io/harfang_c_binding_js_emscripten/build_emsdk/bin/harfang_c_binding_js_emscripten.html

# Compile
To compile see "compile_with_emscripten_note.txt" on windows WSL (works on linux too)

# Scene
To create your own scene, follow the instruction on Harfang3D with the api GLES.  
Example build asset on windows:
* Download assetc for your platform: https://dev.harfang3d.com/releases/
* `assetc.exe -api GLES -quiet -t assetc\\toolchains\\host-windows-x64-target-windows-x64 project project_compiled -platform linux`
