# uc::apng::loader
**uc::apng::loader** is a header only C++11  APNG (Animated PNG) decoder.

## Example

### Load from APNG file

```cpp
auto loader = uc::apng::create_file_loader(filename);

std::cout << "\"" << filename << "\" " 
	<< "(" << loader.width() << "x" << loader.height() << "), " 
	<< loader.num_frames() << "frames, " 
	<< loader.num_plays() << " times to loop (0 indicates infinite looping).\n";
```

### Load APNG frames

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.image.width(), frame.image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, frame.image.data());
```

### Render Animation

```cpp
for (uint32_t i = 0; (loader.num_plays() == 0) || (i < loader.num_plays()); ++i) {

	for (auto&& frame : frames) {

		//
		// render frame
		//

		auto duration = std::chrono::microseconds(1000000) * frame.delay_num / frame.delay_den;
		std::this_thread::sleep_for(duration);
	}
}
```




## Requirements

* C++11 support compiler
* [stb_image.h](https://github.com/nothings/stb)

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
