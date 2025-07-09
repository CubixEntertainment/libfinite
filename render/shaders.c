#include "../include/render/render-core.h"

char *finite_render_get_shader_code(const char *fileName, uint32_t *pShaderSize) {
	if (pShaderSize == NULL){
		return NULL;
	}
	FILE *fp = NULL;
	fp = fopen(fileName, "rb+");

	if (fp == NULL){
		return NULL;
	}

	fseek(fp, 0l, SEEK_END);

	*pShaderSize = (uint32_t)ftell(fp);

	rewind(fp);

	char *shaderCode = (char *)malloc((*pShaderSize) * sizeof(char));
	fread(shaderCode, 1, *pShaderSize, fp);

	fclose(fp);
	return shaderCode;
}