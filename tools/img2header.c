#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdio.h>

const char* header_begin =
"// This image is packed to header using generator in ./tools/ directory!\n"
"// Copyright (C) 2022 UtoECat. All rights reserved.\n"
"// GNU GPL License. No any warrianty!\n"
"\n\n#pragma once\n";

const char* data_type  = "static const unsigned char";
const char* value_type = "static const int ";

int main (int argc, char** argv) {
	if (argc < 4) {
		perror("Usage : img2header [file] [array name] [output]");
		exit(-1);
	};

	FILE* dst;
	dst = fopen(argv[3], "w");
	if (!dst) {
		perror("Can't open output file!");
		exit(-2);
	}

	int x = 0, y = 0, ch = 0;
	unsigned char* img = stbi_load(argv[1], &x, &y, &ch, 0);
	if (!img) {
		perror("Can't open image!");
		exit(-3);
	}

	// write header
	fprintf(dst, header_begin);
	fprintf(dst, "%s %s_width = %i;\n", value_type, argv[2], x);
	fprintf(dst, "%s %s_height = %i;\n", value_type, argv[2], y);
	fprintf(dst, "%s %s_channels = %i;\n", value_type, argv[2], ch);

	// write image data
	fprintf(dst, "\n%s %s_data[] = {\n", data_type, argv[2]);

	size_t size = x * y * ch;

	for (size_t i = 0; i < size; i++) {
		fprintf(dst, "%i", img[i]);
		if (i != size - 1) {
			fprintf(dst, ", ");
			if (i % 13 == 12) fprintf(dst, "\n");
		}
	}

	fprintf(dst, "};\n");
	fflush(dst);
	fclose(dst);
	return 0;
};
