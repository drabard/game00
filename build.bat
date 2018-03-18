clang-cl -Zi -O0 ^
-D_CRT_SECURE_NO_WARNINGS ^
-Wall -Werror -Wno-unknown-pragmas -Wno-macro-redefined -Wno-unused-parameter ^
-ferror-limit=1 ^
-Iext/SDL2/include ^
/SUBSYSTEM:CONSOLE ^
src/main.c ^
src/math.c ^
src/game.c ^
src/file.c ^
src/resources.c ^
src/memory.c ^
src/gpu.c ^
src/graphics.c ^
src/camera.c ^
src/scene.c ^
src/string_id.c ^
src/resources_storage.c ^
src/GL/gl3w.c ^
-o main ^
/link /LIBPATH:ext/SDL2/lib/x64 ^
SDL2main.lib ^
user32.lib ^
shell32.lib ^
opengl32.lib ^
gdi32.lib ^
SDL2.lib