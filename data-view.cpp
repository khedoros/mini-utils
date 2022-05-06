#include<vector>
#include<cstdint>
#include<iostream>
#include<fstream>
#include<assert.h>
#include<SDL2/SDL.h>
using namespace std;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color;

void update_display(SDL_Renderer * rend, vector<uint8_t>& img, SDL_Palette * spal, int winw, int winh, int skipx, int skipy) {

    SDL_Surface * surf = SDL_CreateRGBSurface(0, winw, winh, 8 /*bit depth*/, 0,0,0,0 /*color masks*/);
    if(!surf) {
        cerr<<"Failed to create surface.\t"<<SDL_GetError()<<endl;
        return;
    }
    
    if(SDL_SetSurfacePalette(surf, spal) != 0) {
        cerr<<"Failed to set the surface's palette.\t"<<SDL_GetError()<<endl;
        return;
    }

    for(int pix = 0; pix < winw * winh && pix + skipx + skipy * winw < img.size(); pix++) {
        int x = pix % winw;
        int y = pix / winw;
        int pitch = surf->pitch;
        int addr = y * pitch + x;
        ((uint8_t *)(surf->pixels))[addr] = img[pix + skipx + skipy * winw];
    }

    SDL_Texture * tex = SDL_CreateTextureFromSurface(rend, surf);
    SDL_FreeSurface(surf);
    surf = nullptr;
    SDL_RenderClear(rend);
    SDL_RenderCopy(rend, tex, NULL, NULL);
    SDL_RenderPresent(rend);

}

int sol_size_1(vector<uint8_t>& img) {
    int retval = 0;
    for(int i=0;i<img.size();i++) {
        if(img[i]==0xff) {
            retval += img[i+1];
            i+=2;
        }
        else retval++;
    }
    return retval;
}

int sol_size_2(vector<uint8_t>& img) {
    int retval = 0;
    for(int i=0;i<img.size();i++) {
        if(img[i]==0xff) {
            retval += img[i+1] - 1;
            i+=2;
        }
        else retval++;
    }
    return retval;
}

void decode_img(vector<uint8_t>& orig, vector<uint8_t>& deco, int mode) {
    cout<<"Decoding image in mode "<<mode;
    if(mode == 0) {
        cout<<" (using original data)"<<endl;
        deco = orig;
    }
    if(mode == 2 || mode == 3) { //Solar Winds RLE encoding
        int diff;
        int decoded_size;
        if(mode == 1) {
            diff = 0;
            decoded_size = sol_size_1(orig);
        }
        else if(mode == 2) {
            diff = -1;
            decoded_size = sol_size_2(orig);
        }
        cout<<" (decoding "<<orig.size()<<"bytes to "<<decoded_size<<" bytes)"<<endl;
        deco.resize(decoded_size);
        int dec_index = 0;
        for(int i=0;i<orig.size();i++) {
            if(orig[i]==0xff) { // repeat entry
                if(dec_index + orig[i+1]+diff + 1< deco.size()) { //enough space left in vector
                    memset(&deco[dec_index], orig[i+2], orig[i+1]+diff);
                    dec_index += orig[i+1]+diff;
                }
                else { //not enough space left in vector
                    memset(&deco[dec_index], orig[i+2], deco.size() - dec_index - 1);
                    dec_index = deco.size() - 1;
                }
                i+=2;
            }
            else { //run byte
                deco[dec_index] = orig[i];
                dec_index++;
            }
        }
    }
    if(mode == 1) { // Keen 1-3 RLE encoding
        int decoded_size = orig[0] + orig[1] * 256 + orig[2] * (256*256) + orig[3] * (256 * 256 * 256);
        deco.resize(decoded_size);
        int dec_index = 0;
        for(int i=4; dec_index < decoded_size && i < orig.size(); i++) {
            int control = orig[i];
            if(control & 0x80 > 0) { //run record
                int length = (control & 0x7f) + 1;
                for(int j=0; j < length; j++) {
                    deco[dec_index + j] = orig[i + j + 1];
                }
                dec_index += length;
                i += length - 1; //it'll be incremented by the loop, anyhow
            }
            else { //repeat record
                int length = (control & 0x7f) + 3;
                memset(&deco[dec_index], orig[i+1], length);
                dec_index += length;
                i++; //skip the second byte
            }
        }
    }
    if(mode == 4) { // Keen 1-3 LZW encoding

    }
}

int main(int argc, char *argv[]) {
	int pal_arg_idx = 1;
	int img_arg_idx = 2;
	if(argc == 2) {
		img_arg_idx = 1;
		pal_arg_idx = -1;
	}
	else if(argc != 3) {
        cerr<<"Provide a palette and a data file."<<endl;
        return 1;
    }
    
    assert(sizeof(color) == 3);
    vector<color> pal(256);
	size_t pal_size = 768;

	if(pal_arg_idx != -1) {
	    ifstream pal_file(argv[pal_arg_idx]);
		if(!pal_file.is_open()) {
			cerr<<"Couldn't open the palette file. Check paths."<<endl;
			return 1;
		}

		pal_file.seekg(0,ios::end);
		pal_size = pal_file.tellg();
		pal_file.seekg(0);

		if(pal_size != 256 * 3) {
			cerr<<"The palette needs to be a 768-byte file."<<endl;
			cerr<<"Palsize: "<<pal_size<<endl;
			return 1;
		}

		pal_file.read(reinterpret_cast<char *>(&pal[0]), pal_size);
		pal_file.close();
	}
	else {
		for(int i=0;i<pal.size();i++) {
			pal[i].r = i;
			pal[i].g = i;
			pal[i].b = i;
		}
	}

    SDL_Palette *spal = SDL_AllocPalette(256);
    if(!spal) {
        cerr<<"Couldn't allocate palette."<<endl;
        return 1;
    }

    for(int i=0;i<256;i++) {
        int index = i % (pal_size / 3);
        cout<<"Index: "<<i<<"\t"<<int(pal[index].r)<<"\t"<<int(pal[index].g)<<"\t"<<int(pal[index].b)<<endl;
        spal->colors[i].r = pal[index].r;
        spal->colors[i].g = pal[index].g;
        spal->colors[i].b = pal[index].b;
        spal->colors[i].a = 255;
    }


    ifstream img_file(argv[img_arg_idx]);

    if(!img_file.is_open()) {
        cerr<<"Couldn't open the image file. Check paths."<<endl;
        return 1;
    }

    size_t img_size = 0;
    img_file.seekg(0,ios::end);
    img_size = img_file.tellg();
    img_file.seekg(0);

    vector<uint8_t> img(img_size);
    vector<uint8_t> decoded_img;
    img_file.read(reinterpret_cast<char *>(&img[0]), img_size);
    decode_img(img, decoded_img, 0);
    img_file.close();


    int sdl_status = SDL_Init(SDL_INIT_VIDEO);
    if(sdl_status != 0) {
        cerr<<"Got return value "<<sdl_status<<" while init'ing SDL. Aborting."<<endl;
        cerr<<"Message: "<<SDL_GetError()<<endl;
        return 1;
    }

    bool cont = true;
    bool changed = true;

    int winh = 512;
    int winw = 256;
    int old_winh = 512;
    int old_winw = 256;
    int skipx = 0;
    int skipy = 0;

    SDL_Window * win = SDL_CreateWindow("image viewer", 
                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           winw, winh,
                           SDL_WINDOW_SHOWN);
    if(!win) {
        cerr<<"Failed to create window!\t"<<SDL_GetError()<<endl;
        return 1;
    }

    SDL_Renderer *rend = SDL_CreateRenderer(win, -1,
                                            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!rend) {
        cerr<<"Failed to create renderer!\t"<<SDL_GetError()<<endl;
        return 1;
    }

    int ticks = 0;

    int decode_mode = 0; // 0 = no decode, 1 = Solar Winds RLE decode #1, 2 = Solar Winds RLE decode #2
    while(cont) {
        //Read inputs
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_WINDOWEVENT) {
                cout<<"Window event"<<endl;
            }
            else if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                //cout<<"Key event (scancode: "<<event.key.keysym.scancode<<", keycode: "<<event.key.keysym.sym<<", name: "<<SDL_GetKeyName(event.key.keysym.sym)<<": ";
                if(event.key.state == SDL_PRESSED) {
                    //cout<<"Keydown event"<<endl;
                    switch(event.key.keysym.scancode) {
                        case SDL_SCANCODE_Q:
                            cont = false;
                            break;
                        case SDL_SCANCODE_A:
                            if(winw > 8) {
                                winw--;
                                changed = true;
                            }
                            break;
                        case SDL_SCANCODE_D:
                            if(winw < 2048) {
                                winw++;
                                changed = true;
                            }
                            break;
                        case SDL_SCANCODE_W:
                            if(winh > 8) {
                                winh--;
                                changed = true;
                            }
                            break;
                        case SDL_SCANCODE_S:
                            if(winh < 2048) {
                                winh++;
                                changed = true;
                            }
                            break;
                        case SDL_SCANCODE_F:
                            decode_mode = (decode_mode + 1) % 4;
                            decode_img(img, decoded_img, decode_mode);
                            changed = true;
                            break;
                        case SDL_SCANCODE_O: {
                            ofstream out((string(argv[img_arg_idx])+".exp").c_str());
                            if(out.is_open()) {
                                out.write(reinterpret_cast<char *>(&decoded_img[0]), decoded_img.size());
                                if(out.tellp() != decoded_img.size()) {
                                    cout<<"Might not have written; "<<out.tellp()<<" is the file position, tried to write "<<decoded_img.size()<<" bytes."<<endl;
                                }
                                out.flush();
                                out.close();
                                cout<<"Dumped "<<decoded_img.size()<<" bytes of data to output file "<<argv[img_arg_idx]<<".exp"<<endl;
                            }
                            }
                            break;
                        case SDL_SCANCODE_J:
                            if(skipx > 0) {
                                skipx--;
								cout<<"Top-left is offset "<<std::hex<<"0x"<<skipx+skipy*winw<<"\n";
                                changed = true;
                            }
                            break;
                        case SDL_SCANCODE_L:
                            if(skipx + skipy * winw < decoded_img.size()) {
                                skipx++;
								cout<<"Top-left is offset "<<std::hex<<"0x"<<skipx+skipy*winw<<"\n";
                                changed = true;
                            }
                            break;
                        case SDL_SCANCODE_I:
                            if(skipy > 0) {
                                skipy--;
								cout<<"Top-left is offset "<<std::hex<<"0x"<<skipx+skipy*winw<<"\n";
                                changed = true;
                            }
                            break;
                        case SDL_SCANCODE_K:
                            if(skipx + skipy * winw < decoded_img.size()) {
                                skipy++;
								cout<<"Top-left is offset "<<std::hex<<"0x"<<skipx+skipy*winw<<"\n";
                                changed = true;
                            }
                            break;
                    }
                }
                else if(event.key.state == SDL_RELEASED) {
                    //cout<<"Keyup event"<<endl;
                }
            }
        }

        if(changed) {
            //Apply changes
            if(winw != old_winw || winh != old_winh) {
                SDL_SetWindowSize(win, winw, winh);
                old_winw = winw;
                old_winh = winh;
                cout<<winw<<" x "<<winh<<endl;
            }
            update_display(rend, decoded_img, spal, winw, winh, skipx, skipy);
            changed = false;
        }
        SDL_Delay(10);
        ticks++;
    }
    SDL_DestroyWindow(win);
    win = NULL;
    SDL_Quit();
    return 0;
}
