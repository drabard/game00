clang-3.9 -g -Wall -Werror -std=c11 -I../src -fno-exceptions -ferror-limit=1 \
      src/main.c \
      src/math.c \
      src/game.c \
      src/file.c \
      src/resources.c \
      src/memory.c \
      src/gpu.c \
      src/graphics.c \
      src/camera.c \
      src/scene.c \
      src/string_id.c \
      src/resources_storage.c \
      src/GL/gl3w.c \
      -o main -lSDL2 -lGL -ldl -lm
