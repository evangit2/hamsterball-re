# Generate GLEW .a from the DLL using dlltool
# Actually, just use the static lib with MinGW
# The .lib files MSVC produces won't work with MinGW. Let's use gendef + dlltool.
# Or just include a minimal GL extension loader inline.
echo "Will use inline GL loader instead of GLEW for Windows build"
