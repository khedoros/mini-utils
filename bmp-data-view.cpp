#include<vector>
#include<cstdint>
#include<iostream>
#include<fstream>
#include<assert.h>
#include<SDL2/SDL.h>
using namespace std;

void update_display(SDL_Renderer * rend, vector<uint8_t>& img, int winw, int winh, int skipx, int skipy) {
    uint32_t rmask = 0x000000ff;
    uint32_t gmask = 0x0000ff00;
    uint32_t bmask = 0x00ff0000;
    uint32_t amask = 0xff000000;

    SDL_Surface * surf = SDL_CreateRGBSurface(0, winw, winh, 32 /*bit depth*/, 0,0,0,0 /*color masks*/);
    if(!surf) {
        cerr<<"Failed to create surface.\t"<<SDL_GetError()<<endl;
        return;
    }
    printf("Pitch: %d\n", surf->pitch);
    //               imgp < max_winp                
    for(int pix = 0; pix < winw * winh * 3 && pix + skipx + 3 * winw * skipy < img.size(); pix+=3) {
        int x = (pix/3) % winw;
        int y = (pix/3) / winw;
        int pitch = surf->pitch;
        int addr = (y * pitch) + (4*x);
        ((uint8_t *)(surf->pixels))[addr+2] = img[pix + skipx + (3*skipy*winw)+2 ];
        ((uint8_t *)(surf->pixels))[addr+1] = img[pix + skipx + (3*skipy*winw)+1 ];
        ((uint8_t *)(surf->pixels))[addr+0] = img[pix + skipx + (3*skipy*winw)+0 ];
        ((uint8_t *)(surf->pixels))[addr+3] = 0xff;
    }

    SDL_Texture * tex = SDL_CreateTextureFromSurface(rend, surf);
    SDL_FreeSurface(surf);
    surf = nullptr;
    SDL_RenderClear(rend);
    SDL_RenderCopy(rend, tex, NULL, NULL);
    SDL_RenderPresent(rend);

}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        cerr<<"Provide a data file."<<endl;
        return 1;
    }
        
    ifstream img_file(argv[1]);

    if(!img_file.is_open()) {
        cerr<<"Couldn't open the file. Check paths."<<endl;
        return 1;
    }

    size_t img_size = 0;
    img_file.seekg(0,ios::end);
    img_size = img_file.tellg();
    img_file.seekg(0);

    vector<uint8_t> decoded_img(img_size);
    img_file.read(reinterpret_cast<char *>(&decoded_img[0]), img_size);

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
                            if(winw < 4096) {
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
                            if(winh < 4096) {
                                winh++;
                                changed = true;
                            }
                            break;
                        case SDL_SCANCODE_J:
                            if(skipx > 0) {
                                skipx--;
                                changed = true;
                            }
                            break;
                        case SDL_SCANCODE_L:
                            if(skipx + skipy * winw < decoded_img.size()) {
                                skipx++;
                                changed = true;
                            }
                            break;
                        case SDL_SCANCODE_I:
                            if(skipy > 0) {
                                skipy--;
                                changed = true;
                            }
                            break;
                        case SDL_SCANCODE_K:
                            if(skipx + skipy * winw < decoded_img.size()) {
                                skipy++;
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
            update_display(rend, decoded_img, winw, winh, skipx, skipy);
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
