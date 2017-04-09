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
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>

#define TEST_ASSERT(pred) if (!(pred)) throw std::runtime_error(std::string(__func__).append(" : ").append(#pred).append(" : line ").append(std::to_string(__LINE__)))

inline std::vector<char> read_all(const char* filepath)
{
	std::vector<char> data;
	std::ifstream is(filepath, std::ios::in | std::ios::binary);
	if (is.is_open()) {
		auto pos = is.seekg(0, std::ios::end).tellg();
		data.resize(static_cast<size_t>(pos));
		is.seekg(0, std::ios::beg).read(data.data(), pos);
	}
	return data;
}

template <typename Loader> void test(Loader& loader)
{
	TEST_ASSERT(loader.width() == 100);
	TEST_ASSERT(loader.height() == 100);
	TEST_ASSERT(loader.num_frames() == 20);
	TEST_ASSERT(loader.num_plays() == 0);

	std::vector<uc::apng::frame> frames;
	frames.reserve(loader.num_frames());
	while (loader.has_frame()) {
		frames.push_back(loader.next_frame());
	}

	for (size_t i = 0; i < frames.size(); i++) {
		auto outputfile = "test_data/answer" + std::to_string(i) + ".png";
		int w, h, d;
		uc::apng::stbi_ptr answerImage(stbi_load(outputfile.c_str(), &w, &h, &d, STBI_rgb_alpha));
		std::cout << "  frame " << i << " / " << frames.size() << " : " 
			<< "(" << frames[i].image.width() << "x" << frames[i].image.height() << ")"
			<< " delay=" << frames[i].delay_num << "/" << frames[i].delay_den
			<< " " << (frames[i].is_default ? "default " : " ")
			<< " / expected : \"" << outputfile << "\" (" << w << "x" << h << "x" << d <<") ";
		TEST_ASSERT(answerImage);
		TEST_ASSERT(frames[i].is_default == (i == 0));
		TEST_ASSERT(frames[i].index == i);
		TEST_ASSERT(frames[i].image.width() == w);
		TEST_ASSERT(frames[i].image.height() == h);
		TEST_ASSERT(frames[i].delay_num == 75);
		TEST_ASSERT(frames[i].delay_den == 1000);
		TEST_ASSERT(std::equal(frames[i].image.begin(), frames[i].image.end(), answerImage.get()));
		std::cout << "\t: [OK]\n";
	}
}

int main(int argc, char** argv)
{
	try {
		{
			auto loader = uc::apng::create_file_loader("test_data/Animated_PNG_example_bouncing_beach_ball.apng");
			test(loader);
		}
		{
			auto mem_data = read_all("test_data/Animated_PNG_example_bouncing_beach_ball.apng");
			auto loader = uc::apng::create_memory_loader(mem_data.data(), mem_data.size());
			test(loader);
		}
		{
			auto loader = uc::apng::create_file_loader("test_data/Animated_PNG_example_bouncing_beach_ball0.png");
			std::cout << "normal png read (" 
				<< loader.width() << "x" << loader.height() << "), " 
				<< loader.num_frames() << "frames, " 
				<< loader.num_plays() << " times to loop.\n";

			TEST_ASSERT(loader.width() == 100);
			TEST_ASSERT(loader.height() == 100);
			TEST_ASSERT(loader.num_frames() == 1);
			TEST_ASSERT(loader.num_plays() == 0);
			TEST_ASSERT(loader.has_frame());
			auto frame = loader.next_frame();
			TEST_ASSERT(!loader.has_frame());

			int w, h, d;
			uc::apng::stbi_ptr answerImage(stbi_load("test_data/answer0.png", &w, &h, &d, STBI_rgb_alpha));
			TEST_ASSERT(answerImage);
			TEST_ASSERT(frame.is_default);
			TEST_ASSERT(frame.index == 0);
			TEST_ASSERT(frame.image.width() == w);
			TEST_ASSERT(frame.image.height() == h);
			TEST_ASSERT(frame.delay_num == 0);
			TEST_ASSERT(frame.delay_den == 0);
			TEST_ASSERT(std::equal(frame.image.begin(), frame.image.end(), answerImage.get()));
		}
		std::cout << "all test succeeded.\n";
	}catch(std::exception& ex) {
		std::cout << "test failed : " << ex.what() << std::endl;
	} 
	return 0;
}
