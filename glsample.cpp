/**
uc::apng::loader
Copyright (c) 2017, Kentaro Ushiyama
This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#define STB_IMAGE_IMPLEMENTATION
#include"uc_apng_loader.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <array>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#if defined(_MSC_VER)
#include <GL/glew.h>
#include <GL/wglew.h>
#endif
#include <GLFW/glfw3.h>

namespace
{
	const std::string VERTEX_SHADER = R"(
			attribute vec4 a_position;
			attribute vec2 a_texCoord;
			varying vec2 v_texCoord;
			void main()
			{
				gl_Position = a_position;
				v_texCoord = a_texCoord;
			})";

	const std::string FRAGMENT_SHADER = R"(
			#ifdef GL_ES
			precision mediump float;
			#endif
			varying vec2 v_texCoord;
			uniform sampler2D s_texture;
			void main()
			{
                gl_FragColor = texture2D( s_texture, v_texCoord );
			})";

	void loadShader(GLuint prg, GLenum shaderType, const std::string& shader)
	{
		GLuint sh = glCreateShader(shaderType);
		const GLchar* vsrcs[1] = { shader.c_str() };
		const GLint vlen[1] = { static_cast<GLint>(shader.size()) };
		glShaderSource(sh, 1, vsrcs, vlen);
		glCompileShader(sh);
		GLint compiled;
		GLint loglen;
		glGetShaderiv(sh, GL_COMPILE_STATUS, &compiled);
		glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &loglen);

		std::vector<char> msg(loglen);
		glGetShaderInfoLog(sh, loglen, nullptr, msg.data());
		glAttachShader(prg, sh);
		glDeleteShader(sh);
		if (compiled == GL_FALSE) throw std::runtime_error(msg.data());
	}
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "Usage : " << argv[0] << " [APNG filename]" << std::endl;
		return 1;
	}
	if (!glfwInit()) {
		std::cerr << "Error : glfwInit() failed." << std::endl;
		return -1;
	}
	try {
		uint32_t remainPlay{};
		std::vector<uc::apng::frame> frames;
		{
			const std::string filename = argv[1];
			auto loader = uc::apng::create_file_loader(filename);
			remainPlay = loader.num_plays();

			std::cout << "\n\"" << filename << "\" ("
				<< loader.width() << "x" << loader.height() << "), "
				<< loader.num_frames() << "frames, "
				<< loader.num_plays() << " times to loop (0 indicates infinite looping).\n";

			while (loader.has_frame()) {
				frames.push_back(loader.next_frame());
			}
			if (frames.empty()) throw std::runtime_error("apng has no frame.");
		}
		auto frame = frames.begin();

		auto window = glfwCreateWindow(frame->image.width(), frame->image.height(), "apng player", nullptr, nullptr);
		if (!window) throw std::runtime_error("glfwCreateWindow() failed.");

		glfwMakeContextCurrent(window);
#ifdef __glew_h__
		glewExperimental = TRUE;
		GLenum err = glewInit();
		if (err != GLEW_OK) throw std::runtime_error("glewInit() failed.");
#endif

		auto prg = glCreateProgram();
		loadShader(prg, GL_VERTEX_SHADER, VERTEX_SHADER);
		loadShader(prg, GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
		glLinkProgram(prg);
		GLint linked;
		GLint loglen = 0;
		glGetProgramiv(prg, GL_LINK_STATUS, &linked);
		glGetProgramiv(prg, GL_INFO_LOG_LENGTH, &loglen);
		std::vector<char> msg(loglen);
		glGetProgramInfoLog(prg, loglen, nullptr, msg.data());
		if (linked == GL_FALSE) throw std::runtime_error(msg.data());
		glUseProgram(prg);

		std::array<GLuint, 2> vbo;
		glGenBuffers(static_cast<GLsizei>(vbo.size()), vbo.data());

		GLushort indices[] = { 0, 1, 2, 2, 1, 3 };
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		GLfloat vertices[] = {
			-1.0f, 1.0f, 0.0f, 	0.0f, 0.0f,
			1.0f, 1.0f, 0.0f,   1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,	0.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 	1.0f, 1.0f
		};
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		auto positionLoc = glGetAttribLocation(prg, "a_position");
		auto texCoordLoc = glGetAttribLocation(prg, "a_texCoord");
		glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
		glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(positionLoc);
		glEnableVertexAttribArray(texCoordLoc);

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        using namespace std::chrono;
        auto nextTime = high_resolution_clock::now();
        nextTime += microseconds(1000000) * frame->delay_num / frame->delay_den;

		while (!glfwWindowShouldClose(window)) {
			glClear(GL_COLOR_BUFFER_BIT);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame->image.width(), frame->image.height(), 
				0, GL_RGBA, GL_UNSIGNED_BYTE, frame->image.data());
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
			glfwSwapBuffers(window);
			glfwPollEvents();

			++frame;
			if (frame == frames.end()) {
				if (remainPlay > 0 && --remainPlay == 0) {
					break;
				}
				frame = frames.begin();
			}
			std::this_thread::sleep_until(nextTime);
			nextTime += microseconds(1000000) * frame->delay_num / frame->delay_den;
		}

		glDisableVertexAttribArray(texCoordLoc);
		glDisableVertexAttribArray(positionLoc);
		glDeleteTextures(1, &tex);
		glDeleteBuffers(static_cast<GLsizei>(vbo.size()), vbo.data());
		glDeleteProgram(prg);
		glfwDestroyWindow(window);

		std::cout << "quit.\n";
	} catch (std::exception& ex) {
		std::cerr << "Error : " << ex.what() << std::endl;
	} 
	glfwTerminate();
	return 0;
}
