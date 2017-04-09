/**
uc::apng::loader
Copyright (c) 2017, Kentaro Ushiyama
This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#include"uc_apng_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <string>

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "Usage : " << argv[0] << " [APNG filename]" << std::endl;
		return 1;
	}
	try {
		auto loader = uc::apng::create_file_loader(argv[1]);

		std::cout << "\n\"" << argv[1] << "\" (" 
			<< loader.width() << "x" << loader.height() << "), " 
			<< loader.num_frames() << "frames, " 
			<< loader.num_plays() << " times to loop (0 indicates infinite looping).\n";

		while (loader.has_frame()) {
            auto frame = loader.next_frame();

			std::ostringstream os;
			os << "out" << std::setw(3) << std::setfill('0') << frame.index << ".png";
			auto outputfile = os.str();

            std::cout << " " << frame.index << " / " << loader.num_frames() << " : \"" << outputfile << "\" "
                << "(" << frame.image.width() << "x" << frame.image.height() << ")"
				<< " delay=" << frame.delay_num << "/" << frame.delay_den
				<< " " << (frame.is_default ? "default " : " ") << "\n";

			stbi_write_png(outputfile.c_str(), frame.image.width(), frame.image.height(), 4, frame.image.data(), frame.image.width() * 4);
		}
		std::cout << "all done\n";
	}catch(std::exception& ex) {
		std::cout << "failed : " << ex.what() << std::endl;
	} 
	return 0;
}
