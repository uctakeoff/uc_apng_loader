# uc::apng::loader
**uc::apng::loader** is a header only C++11  APNG (Animated PNG) decoder.

## Requirements

* C++11 support compiler
* [stb_image.h](https://github.com/nothings/stb)

## Example

### source code

```cpp
// apng2pngs.cpp

#define STB_IMAGE_IMPLEMENTATION
#include "uc_apng_loader.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage : " << argv[0] << " [APNG filename]" << std::endl;
        return 1;
    }
    try {
        auto loader = uc::apng::create_file_loader(argv[1]);

        while (loader.has_frame()) {

            auto frame = loader.next_frame();

            std::ostringstream filename;
            filename << "out" << std::setw(3) << std::setfill('0') << frame.index << ".png";

            stbi_write_png(filename.str().c_str(), frame.image.width(), frame.image.height(), 
	        4, frame.image.data(), frame.image.width() * 4);
        }
    } catch (std::exception& ex) {
        std::cout << "failed : " << ex.what() << std::endl;
    } 
    return 0;
}
```
### compile

```bash
$ g++ -std=c++11 apng2pngs.cpp
```

## Usage

### Load APNG data

```cpp
// from file
auto loader = uc::apng::create_file_loader("filename.apng");

// from memory (std::string stringdata)
auto loader = uc::apng::create_memory_loader(stringData);

// from memory (const char* buf, size_t buflen)
auto loader = uc::apng::create_memory_loader(buf, buflen);

// member
std::cout << "(" << loader.width() << "x" << loader.height() << "), " 
	<< loader.num_frames() << "frames, " 
	<< loader.num_plays() << " times to loop (0 indicates infinite looping).\n";
```

### Get APNG frames

```cpp
std::vector<uc::apng::frame> frames;
while (loader.has_frame()) {
	frames.push_back(loader.next_frame());
}
```

### Write to PNG file (using `stb_image_write.h`)

```cpp
for (auto&& frame : frames) {

	auto filename = "out" + std::to_string(frame.index) + ".png";

	stbi_write_png(filename.c_str(), frame.image.width(), frame.image.height(), 4,
		 frame.image.data(), frame.image.width() * 4);
}
```

### Load to OpenGL Texture

```cpp
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.image.width(), frame.image.height(), 
		0, GL_RGBA, GL_UNSIGNED_BYTE, frame.image.data());
```


## Sample Code

### Build & Execute

```bash
$ g++ -std=c++11 apng2pngs.cpp
$ ./a.out test_data/Animated_PNG_example_bouncing_beach_ball.apng
```

## Test

### Build & Execute

```bash
$ g++ -std=c++11 test.cpp
$ ./a.out
```

## License

MIT License

## References

* [APNG Specification](https://wiki.mozilla.org/APNG_Specification#.60fcTL.60:_The_Frame_Control_Chunk)
