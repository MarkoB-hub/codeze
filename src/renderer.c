#include "renderer.h"
#include "fileio.h"
#include "shader.h"
#include "debug.h"

#include <glad/glad.h>

#define MAX_VERTICES 100000
#define VERTICES_PER_QUAD 6

static Vec4 gColors[TOK_TOTAL];

static void
error_callback(int code, const char* description) {

	printf("Error %i : %s \n", code, description);

}


GLFWwindow*
renderer_create_window() {

	Vec4 white = {0.8f, 0.8f, 0.8f, 1.0f};
	Vec4 green = {0.8f, 1.0f, 0.8f, 1.0f};
	Vec4 blue = {0.6f, 0.6f, 1.0f, 1.0f};
	Vec4 orange = {0.9f, 0.6f, 0.1f, 1.0f};
	Vec4 green2 = {0.2f, 0.8f, 0.5f, 0.8f};
	Vec4 redish = {0.7f, 0.3f, 0.1f, 0.8f};
	Vec4 purple = {0.5f, 0.2f, 0.6f, 0.8f};

	gColors[TOK_IDENTIFIER] = white;
	gColors[TOK_HASH] = white;
	gColors[TOK_NUMBER] = green;
	gColors[TOK_OPEN_PAREN] = orange;
	gColors[TOK_CLOSED_PAREN] = orange;
	gColors[TOK_OPEN_CURLY] = orange;
	gColors[TOK_CLOSED_CURLY] = orange;
	gColors[TOK_OPEN_SQUARE] = orange;
	gColors[TOK_CLOSED_SQUARE] = orange;
	gColors[TOK_KEYWORD] = orange;
	gColors[TOK_TYPE] = blue;
	gColors[TOK_STRING] = green2;
	gColors[TOK_SEMICOLON] = redish;
	gColors[TOK_COMMENT] = purple;

	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);
	
	if (!glfwInit()) {
		printf("Failed to initialize glfw\n");
		ASSERT(0);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1280, 768, "codeze", NULL, NULL);
	if (!window) {
		glfwTerminate();
		printf("Failed to create window");
		ASSERT(0);
	}

	glfwMakeContextCurrent(window);

	// Glad initialization
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initialize glad");
		ASSERT(0);
	}
	printf("Initialized glad\n");

	// Enable Vsync
	glfwSwapInterval(1);

	return window;

}

static void
texture_load(const char* path, u32* texID, u8 slot) {

	
	glActiveTexture(GL_TEXTURE0 + slot);

	i32 w, h, channels, internalFormat, dataFormat;
	u8* buffer = image_load_png(path, &w, &h, &channels);
	ASSERT(buffer != NULL);

	if (channels == 4) {
		internalFormat = GL_RGBA8;
		dataFormat = GL_RGBA;
	}
	else if (channels == 3) {
		internalFormat = GL_RGB8;
		dataFormat = GL_RGB;
	}


	glGenTextures(1, texID);
	glBindTexture(GL_TEXTURE_2D, *texID);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, dataFormat, GL_UNSIGNED_BYTE, buffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	image_free(buffer);
}

void
renderer_initialize(Renderer* ren, f32 width, f32 height) {

	ren->vertexArray = malloc(sizeof(Vertex) * MAX_VERTICES);
	ren->vertexArrayIndex = ren->vertexArray;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	File* vertexFile = file_open("src/vertex_shader.txt", "r");
	File* fragmentFile = file_open("src/fragment_shader.txt", "r");

	ren->program = shader_create(vertexFile->buffer, fragmentFile->buffer);
	
	glUseProgram(ren->program);

	glGenVertexArrays(1, &ren->VAO);
	glGenBuffers(1, &ren->VBO);

	glBindVertexArray(ren->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, ren->VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(f32) * 4));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(f32) * 4 * 2));

	ortho(ren->projection, 0.0f, width, height, 0.0f);

	texture_load("assets/green.png", &ren->texIDs[GREEN_TEXTURE_INDEX], GREEN_TEXTURE_INDEX);
	texture_load("assets/white.png", &ren->texIDs[WHITE_TEXTURE_INDEX], WHITE_TEXTURE_INDEX);
	texture_load("assets/ryuk.png", &ren->texIDs[RYUK_TEXTURE_INDEX], RYUK_TEXTURE_INDEX);

	String* str = str_create_c("uTextures");
	char index[] = "[ ]";
	i32 location;
	for (i32 i = 0; i < TEXTURE_SLOTS; ++i) {

		index[1] = '0' + i;
		str_concat(str, index);

		location = glGetUniformLocation(ren->program, str->data);
		ASSERT(location != -1);

		glUniform1i(location, i);
		str_delete_from_back(str, 3);
	}

	str_delete(str);


	file_close(vertexFile);
	file_close(fragmentFile);

}

void
renderer_load_font(Renderer* ren, const char* fontFile, i32 fontSize) {

	ren->fontSize = fontSize;

	i32 success = FT_Init_FreeType(&ren->ftLib);
	ASSERT(success == 0);
	success = FT_New_Face(ren->ftLib, fontFile, 0, &ren->fontFace);
	ASSERT(success == 0);

	FT_Set_Pixel_Sizes(ren->fontFace, 0, ren->fontSize);

	i32 w = 0, h = 0;
	FT_GlyphSlot glyph = ren->fontFace->glyph;

	for (u8 i = 0; i < 128; i++) {

		i32 success = FT_Load_Char(ren->fontFace, i, FT_LOAD_RENDER);
		ASSERT(success == 0);
		w += glyph->bitmap.width;
		glyph->bitmap.rows > h ? h = glyph->bitmap.rows : h;
	}

	ren->bitmapW = w;
	ren->bitmapH = h;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glActiveTexture(GL_TEXTURE0 + FONT_TEXTURE_INDEX);

	glGenTextures(1, &ren->texIDs[FONT_TEXTURE_INDEX]);
	glBindTexture(GL_TEXTURE_2D, ren->texIDs[FONT_TEXTURE_INDEX]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);


	i32 texOffset = 0;

	for (u8 i = 0; i < 128; ++i) {

		i32 success = FT_Load_Char(ren->fontFace, i, FT_LOAD_RENDER);
		ASSERT(success == 0);

		glTexSubImage2D(GL_TEXTURE_2D, 0, texOffset, 0, 
			glyph->bitmap.width, glyph->bitmap.rows, 
			GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);

		ren->glyphs[i].advanceX = glyph->advance.x >> 6;
		ren->glyphs[i].advanceY = glyph->advance.y >> 6;

		ren->glyphs[i].width = glyph->bitmap.width;
		ren->glyphs[i].height = glyph->bitmap.rows;

		ren->glyphs[i].bearingX = glyph->bitmap_left;
		ren->glyphs[i].bearingY = glyph->bitmap_top;
	
		ren->glyphs[i].offsetX = (f32)texOffset / w;

		texOffset += glyph->bitmap.width;
	}

	// i32 location = glGetUniformLocation(ren->program, "uTextures[2]");
	// ASSERT(location != -1);

	// glUniform1i(location, FONT_TEXTURE_INDEX);

	// Hardcoded tab size
	ren->glyphs['\t'].advanceX = ren->glyphs[' '].advanceX * 4;

	FT_Done_Face(ren->fontFace);
	FT_Done_FreeType(ren->ftLib);

  
}

void
renderer_begin(Renderer* ren) {

	glClearColor(0.1f, 0.1f, 0.13, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	i32 projectionLocation = glGetUniformLocation(ren->program, "uProjection");
	ASSERT(projectionLocation != -1);

	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &ren->projection[0][0]);

}

void
render_quad(Renderer* ren, Vec2 position, Vec2 size, Vec4 color) {

		Vec4 quadVertices[] = {
				{position.x,          position.y,          0.0f, 0.0f},
				{position.x + size.x, position.y,          1.0f, 0.0f},
				{position.x + size.x, position.y + size.y, 1.0f, 1.0f},
				{position.x,          position.y,          0.0f, 0.0f},
				{position.x,          position.y + size.y, 0.0f, 1.0f},
				{position.x + size.x, position.y + size.y, 1.0f, 1.0f}
		};

		if (ren->vertexCount >= MAX_VERTICES) {
			renderer_end(ren);
		}

		for (int i = 0; i < VERTICES_PER_QUAD; ++i) {

			ren->vertexArrayIndex->color = color;
			ren->vertexArrayIndex->posData = quadVertices[i];
			ren->vertexArrayIndex->texIndex = NO_TEXTURE;
			ren->vertexArrayIndex++;
		}

		ren->vertexCount += VERTICES_PER_QUAD;

}

void
render_textured_quad(Renderer* ren, Vec2 position, Vec2 size, Vec4 color, u32 texID) {

		Vec4 quadVertices[] = {
				{position.x,          position.y,          0.0f, 0.0f},
				{position.x + size.x, position.y,          1.0f, 0.0f},
				{position.x + size.x, position.y + size.y, 1.0f, 1.0f},
				{position.x,          position.y,          0.0f, 0.0f},
				{position.x,          position.y + size.y, 0.0f, 1.0f},
				{position.x + size.x, position.y + size.y, 1.0f, 1.0f}
		};

		if (ren->vertexCount >= MAX_VERTICES) {
			renderer_end(ren);
		}

		for (int k = 0; k < VERTICES_PER_QUAD; ++k) {

			ren->vertexArrayIndex->color = color;
			ren->vertexArrayIndex->posData = quadVertices[k];
			ren->vertexArrayIndex->texIndex = texID;
			ren->vertexArrayIndex++;
		}

		ren->vertexCount += VERTICES_PER_QUAD;
	
}

void
render_text(Renderer* ren, String* text, Vec2 position, Vec4 color) {

	static float xpos, ypos, w, h, offsetX, texX, texY, advanceX, advanceY;
	advanceY = position.y;
	advanceX = position.x;

	for (sizet i = 0; i < text->length; ++i) {

		if (str_at(text, i) == '\n') {

			advanceY += ren->fontSize;
			advanceX = 0.0f;
			continue;
		}
		else if (str_at(text, i) == '\t') {

			advanceX += ren->glyphs[str_at(text, i)].advanceX;
			continue;
		}

		xpos = advanceX + ren->glyphs[str_at(text, i)].bearingX;
		// this is stupid, idk how else to make it work
		ypos = advanceY - ren->glyphs[str_at(text, i)].bearingY + ren->fontSize;
		w = ren->glyphs[str_at(text, i)].width;
		h = ren->glyphs[str_at(text, i)].height;
		offsetX = ren->glyphs[str_at(text, i)].offsetX;
		texX = w / ren->bitmapW;
		texY = h / ren->bitmapH;

		Vec4 quadVertices[] = {
			{xpos,     ypos,     offsetX,        0.0f},
			{xpos + w, ypos,     offsetX + texX, 0.0f},
			{xpos + w, ypos + h, offsetX + texX, texY},
			{xpos,     ypos,     offsetX,        0.0f},
			{xpos,     ypos + h, offsetX,        texY},
			{xpos + w, ypos + h, offsetX + texX, texY}
		};

		if (ren->vertexCount >= MAX_VERTICES) {

			renderer_end(ren);
		}

		for (int j = 0; j < VERTICES_PER_QUAD; ++j) {

			ren->vertexArrayIndex->color = color;
			ren->vertexArrayIndex->posData = quadVertices[j];
			ren->vertexArrayIndex->texIndex = FONT_TEXTURE_INDEX;
			ren->vertexArrayIndex++;
		}

		ren->vertexCount += VERTICES_PER_QUAD;
		advanceX += ren->glyphs[str_at(text, i)].advanceX;
	}
}

void
render_text_syntax(Renderer* ren, String* text, Vec2 position, Token* tokens) {

	static float xpos, ypos, w, h, offsetX, texX, texY, advanceX, advanceY;
	advanceY = position.y;
	advanceX = position.x;

	i32 tokIndex = 0;
	Vec4 color = gColors[0];

	for (sizet i = 0; i < text->length; ++i) {

		if (str_at(text, i) == '\n') {

			advanceY += ren->fontSize;
			advanceX = 0.0f;
			continue;
		}
		else if (str_at(text, i) == '\t') {

			advanceX += ren->glyphs[str_at(text, i)].advanceX;
			continue;
		}

		xpos = advanceX + ren->glyphs[str_at(text, i)].bearingX;
		// this is stupid, idk how else to make it work
		ypos = advanceY - ren->glyphs[str_at(text, i)].bearingY + ren->fontSize;
		w = ren->glyphs[str_at(text, i)].width;
		h = ren->glyphs[str_at(text, i)].height;
		offsetX = ren->glyphs[str_at(text, i)].offsetX;
		texX = w / ren->bitmapW;
		texY = h / ren->bitmapH;

		Vec4 quadVertices[] = {
			{xpos,     ypos,     offsetX,        0.0f},
			{xpos + w, ypos,     offsetX + texX, 0.0f},
			{xpos + w, ypos + h, offsetX + texX, texY},
			{xpos,     ypos,     offsetX,        0.0f},
			{xpos,     ypos + h, offsetX,        texY},
			{xpos + w, ypos + h, offsetX + texX, texY}
		};

		if (ren->vertexCount >= MAX_VERTICES) {

			renderer_end(ren);
		}

		static i32 len = 0;

		if (i == tokens[tokIndex].pos) {

			color = gColors[tokens[tokIndex].type];
			len = tokens[tokIndex].length;
			tokIndex++;
		}

		if (len <= 0) {
			color = gColors[0];
		}

		for (int j = 0; j < VERTICES_PER_QUAD; ++j) {

			ren->vertexArrayIndex->color = color;
			ren->vertexArrayIndex->posData = quadVertices[j];
			ren->vertexArrayIndex->texIndex = FONT_TEXTURE_INDEX;
			ren->vertexArrayIndex++;
		}

		len--;

		ren->vertexCount += VERTICES_PER_QUAD;
		advanceX += ren->glyphs[str_at(text, i)].advanceX;
	}
}

void
renderer_end(Renderer* ren) {

	static i32 count, dataSize;
	count = (ren->vertexArrayIndex - ren->vertexArray);
	dataSize = (ren->vertexArrayIndex - ren->vertexArray) * sizeof(Vertex);

	glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, ren->vertexArray);

	glDrawArrays(GL_TRIANGLES, 0, ren->vertexCount);

	ren->vertexCount = 0;
	ren->vertexArrayIndex = ren->vertexArray;

}
