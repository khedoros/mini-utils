#include<cstdint>
#include<cassert>
#include<iostream>
#include<fstream>
#include<string>
#include<byteswap.h>
#include<vector>

//Get lowest-order bit. If all have been consumed, pull in the next.
#define GET_BIT do { bit = bitstr_value & 1; \
    bitstr_len--; \
    bitstr_value >>= 1; \
    if (bitstr_len == 0) { \
        bitstr_value = READ_LE_UINT16(compressed); \
        bitstr_len = 16; \
        compressed += 2; \
    } \
} while (0)

#define READ_LE_UINT16(target) ((uint16_t(*target) & 0xff) | ((uint16_t(*(target + 1))&0xff)<<8)) & 0xffff
#define MKTAG(a0,a1,a2,a3) ((uint32_t)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

uint32_t readUint32LE(std::ifstream& in) {
	uint32_t buffer;
	in.read(reinterpret_cast<char *>(&buffer), 4);
	return buffer;
}

uint32_t readUint32BE(std::ifstream& in) {
	return bswap_32(readUint32LE(in));
}

bool decompress_codec3(const char *compressed, char *result, int maxBytes) {
    // consume 2 bytes from compressed stream as a string of 16 bits (inits the bit-processing stream)
    int bitstr_value = READ_LE_UINT16(compressed);
    int bitstr_len = 16;
    compressed += 2;
    bool bit; // this will be 0 or 1 after a GET_BIT

    int byteIndex = 0;
    for (;;) {
        GET_BIT;
        if (bit == 1) { // if bit==1, copy a byte from compressed data to result
            if (byteIndex >= maxBytes) {
				std::fprintf(stderr,"Buffer overflow when decoding image: decompress_codec3 walked past the input buffer!");
                return false;
            } else {
                *result++ = *compressed++;
            }
            ++byteIndex;
        } else {
            GET_BIT;
            int copy_len, copy_offset;
            if (bit == 0) { // 2 bits were 00, copy-length is 3,4,5, or 6, depending on next 2 bits. 
                GET_BIT;
                copy_len = 2 * bit;
                GET_BIT;
                copy_len += bit + 3;
                copy_offset = *(const uint8_t *)(compressed++) - 0x100;
            } else {
                copy_offset = (*(const uint8_t *)(compressed) | (*(const uint8_t *)(compressed + 1) & 0xf0) << 4) - 0x1000;
                copy_len = (*(const uint8_t *)(compressed + 1) & 0xf) + 3;
                compressed += 2;
                if (copy_len == 3) {
                    copy_len = *(const uint8_t *)(compressed++) + 1;
                    if (copy_len == 1)
                        return true;
                }
            }
            while (copy_len > 0) {
                if (byteIndex >= maxBytes) {
					std::fprintf(stderr,"Buffer overflow when decoding image: decompress_codec3 walked past the input buffer!");
                    return false;
                } else {
                    assert(byteIndex + copy_offset >= 0);
                    assert(byteIndex + copy_offset < maxBytes);
                    *result = result[copy_offset];
                    result++;
                }
                ++byteIndex;
                copy_len--;
            }
        }
    }
    return true;
}

struct metadata_t {
	std::string _fname;
	uint32_t _numImages;
	uint32_t _x;
	uint32_t _y;
	uint32_t _format;
	uint32_t _bpp;
	uint32_t _width;
	uint32_t _height;
	uint32_t _colorFormat;
	uint32_t _hasTransparency;
	std::vector<std::vector<uint8_t>> _data;
	bool loadGrimBm(std::ifstream&);
} metadata;

bool metadata_t::loadGrimBm(std::ifstream& data) {
    uint32_t tag2 = readUint32BE(data);
    if (tag2 != (MKTAG('F','\0','\0','\0')))
        return false;

    int codec = readUint32LE(data);
    readUint32LE(data);               //_paletteIncluded
    _numImages = readUint32LE(data);
    _x = readUint32LE(data);
    _y = readUint32LE(data);
    readUint32LE(data);               //_transparentColor
    _format = readUint32LE(data);
    _bpp = readUint32LE(data);
//  uint32_t redBits = readUint32LE(data);
//  uint32_t greenBits = readUint32LE(data);
//  uint32_t blueBits = readUint32LE(data);
//  uint32_t redShift = readUint32LE(data);
//  uint32_t greenShift = readUint32LE(data);
//  uint32_t blueShift = readUint32LE(data);

    // Hardcode the format, since the values saved in the files are garbage for some, like "ha_0_elvos.zbm".
    //Graphics::PixelFormat pixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);

    data.seekg(128);
    _width = readUint32LE(data);
    _height = readUint32LE(data);
    //_colorFormat = BM_RGB565;
    _hasTransparency = false;

    _data.resize(_numImages);// = new Graphics::PixelBuffer[_numImages];
    data.seekg(0x80);
    for (int i = 0; i < _numImages; i++) {
        data.seekg(8, std::ios::cur);
		_data[i].resize(_width*_height*2);
        //_data[i].create(pixelFormat, _width * _height, DisposeAfterUse::YES);
        if (codec == 0) {
            uint32_t dsize = _bpp / 8 * _width * _height;
            data.read(reinterpret_cast<char *>(_data[i].data()), dsize);
        } else if (codec == 3) {
            int compressed_len = readUint32LE(data);
            char *compressed = new char[compressed_len];
            data.read(compressed, compressed_len);
            bool success = decompress_codec3(compressed, (char *)_data[i].data(), _bpp / 8 * _width * _height);
            delete[] compressed;
            if (!success)
                std::fprintf(stderr,".. when loading image %s.", _fname.c_str());
        } else
            std::cerr<<"Unknown image codec in BitmapData ctor!\n";

#ifdef SCUMM_BIG_ENDIAN
        if (_format == 1) {
            uint16 *d = (uint16 *)_data[i].data();
            for (int j = 0; j < _width * _height; ++j) {
                d[j] = SWAP_BYTES_16(d[j]);
            }
        }
#endif
    }

    // Initially, no GPU-side textures created. the createBitmap
    // function will allocate some if necessary (and successful)
    //_numTex = 0;
    //_texIds = nullptr;

    //g_driver->createBitmap(this);
    return true;
}

bool load(std::string& filename) {
	std::ifstream data(filename);
	if(!data.is_open()) {
		std::cerr<<"Couldn't open file at "<<filename<<"\n";
	}
	metadata._fname = filename;
    uint32_t tag = readUint32BE(data);
    switch(tag) {
        case(MKTAG('B','M',' ',' ')):               //Grim bitmap
            metadata.loadGrimBm(data);
            break;
		default:
			std::cerr<<"Saw tag "<<std::hex<<tag<<" instead of expected "<<MKTAG('B','M',' ',' ')<<"\n";
			return false;
	}
	return true;
}

int main(int argc, char* argv[]) {
	if(argc==2) {
		std::string filename = argv[1];
		if(load(filename)) {
			std::cout<<"There are "<<metadata._numImages<<" "<<metadata._width<<"x"<<metadata._height<<" images to output\n";
			for(int i=0;i<metadata._numImages;i++) {
				std::ofstream out(filename + "-"+std::to_string(i+1)+".out");
				out.write(reinterpret_cast<char *>(metadata._data[i].data()), metadata._data[i].size());
				out.close();
			}
		}
		else {
			std::cerr<<"Some problem occurred.\n";
			return 1;
		}
	}
	else {
		std::cerr<<"Provide a filename.\n";
		return 1;
	}
	return 0;
}
