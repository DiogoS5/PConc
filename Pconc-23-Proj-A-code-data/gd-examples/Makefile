all: apply_contrast apply_sepia apply_smooth apply_texture

apply_contrast: apply_contrast.c image-lib.c image-lib.h
	gcc apply_contrast.c image-lib.c image-lib.h -g -o apply_contrast -lgd

apply_sepia: apply_sepia.c image-lib.c image-lib.h
	gcc apply_sepia.c image-lib.c image-lib.h -g -o apply_sepia -lgd

apply_smooth: apply_smooth.c image-lib.c image-lib.h
	gcc apply_smooth.c image-lib.c image-lib.h -g -o apply_smooth -lgd

apply_texture: apply_texture.c image-lib.c image-lib.h
	gcc apply_texture.c image-lib.c image-lib.h -g -o apply_texture -lgd
