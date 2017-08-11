#include<stdint.h>
#include<string>
#include<iostream>
#include<vector>
#include<time.h>
#include<stdlib.h>
#include<SDL2/SDL.h>
#include<utility>
#include<memory>
using namespace std;

int main(int argc, char *argv[]) {
    float framerate = 30.0;
    if(argc<3) {
        cerr<<"Give me a width and a height [and optionally, a framerate in fps."<<endl;
        return 1;
    }
    else if(argc == 4) {
        framerate = stoi(argv[3]);
        if(framerate < 1) framerate = 1.0;
        else if(framerate > 200) framerate = 200.0;
    }
    uint32_t w = stoi(argv[1]);
    uint32_t h = stoi(argv[2]);
    bool showcounts = false;
    if(w < 1 || w > 2000 || h < 1 || h > 2000) {
        cerr<<"The width and height must be in the range [1, 2000]."<<endl;
        return 2;
    }
    int run_count = 0;
    restartit:
    cout<<"Running iteration number "<<++run_count<<endl;
    srand(time(0));
    cout<<"Rand: "<<rand()<<endl;

    shared_ptr<vector<uint32_t>> fbuf(new vector<uint32_t>(w*h, 0)); //Front buffer
    shared_ptr<vector<uint32_t>> bbuf(new vector<uint32_t>(w*h, 0)); //Back buffer
    vector<bool> test(w*h,false); //Was the given pixel already marked as "changed"
    vector<uint32_t> changes(w*h,-1); //indices that changed this generation
    vector<uint32_t> old_changes(w*h,-1); //indices that changed last generation
    uint32_t change_count = 0;
    uint32_t old_change_count = 0;
    uint32_t rep_count = 0;
    bool informed_of_end = false;
    uint32_t last_seen_gen = 0;
    
    for(int i=0;i<w*h;++i) {
        (*fbuf)[i] = (*bbuf)[i] = (rand() % 2) * (UINT32_MAX);
    }

    //Short-circuit if we're starting a second iteration
    if(run_count == 1 && SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS)) {
        cerr<<"SDL Init returned non-zero; aborting.";
        return 3;
    }
    SDL_Window * win;
    SDL_Renderer * rend;
    SDL_Texture * tex;
    if(run_count == 1) {
        win = SDL_CreateWindow("Conway's Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN|SDL_WINDOW_INPUT_FOCUS);
        rend = SDL_CreateRenderer(win, -1, 0);
        tex = SDL_CreateTexture(rend,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING, w, h);
        if(!win || !rend || !tex) {
            cerr<<"Window, renderer, or texture couldn't be created: "<<SDL_GetError()<<endl;
            return 3;
        }
        SDL_SetRenderDrawColor(rend, 0,0,0,255);
    }
    SDL_RenderClear(rend);
    SDL_RenderPresent(rend);
    bool quitting_time = false;
    uint32_t gen = 0;
    uint32_t frame = 0;
    uint32_t cur_time = SDL_GetTicks();
    uint32_t start_time = SDL_GetTicks();
    int32_t count_hist[60]; //Should allow for checking cycles based on lengths of 2, 3, 4, or 5, and any of their composite multiples
    uint32_t reason = 0;

    for(int i=0;i<w*h;++i) {
        uint32_t row = i/w;
        uint32_t col = i%w;
        uint32_t left = ((col == 0)?w-1:col-1);
        uint32_t right = ((col == w-1)?0:col+1);
        uint32_t top = ((row == 0)?h-1:row-1) * w;
        uint32_t bottom = ((row == h-1)?0:row+1) * w;
        row *= w;

        uint64_t count = (*fbuf)[left+top]; 
        count +=         (*fbuf)[col+top]; 
        count +=         (*fbuf)[right+top];
        count +=         (*fbuf)[left+row];
        count +=         (*fbuf)[right+row];
        count +=         (*fbuf)[left+bottom];
        count +=         (*fbuf)[col+bottom];
        count +=         (*fbuf)[right+bottom]; 

        //cout<<count<<endl<<count/UINT32_MAX<<endl;
        count/=(UINT32_MAX);
        bool status = ((*fbuf)[i] > 0);
        bool change = false;
        if((status && (count < 2 || count > 3))) {//cell will die
            (*bbuf)[i] = 0;
            change = true;
        }
        else if(!status && count == 3) { //cell will come alive
            (*bbuf)[i] = UINT32_MAX;
            change = true;
        }

        if(change) {
            uint32_t neighbors[] = {left+top,    col+top,    right+top,
                                    left+row,    col+row,    right+row,
                                    left+bottom, col+bottom, right+bottom};
            for(uint32_t i = 0; i < 9; ++i) {
                if(!test[neighbors[i]]) {
                    test[neighbors[i]]=true;
                    changes[change_count++] = neighbors[i];
                }
            }
        }
    }

    while(!quitting_time && !informed_of_end) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_KEYDOWN || event.type == SDL_QUIT) {
                if(event.type == SDL_KEYDOWN && event.key.keysym.sym == ' ') {
                    showcounts = !showcounts;
                }
                else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == 'r') {
                    //SDL_Quit();
                    cout<<endl<<endl<<"Restarting a new simulation..."<<endl;
                    goto restartit;
                }
                else if(event.type == SDL_KEYDOWN && (event.key.keysym.sym == 1073742048 || event.key.keysym.sym == 1073742050 || event.key.keysym.sym == 1073742052 || event.key.keysym.sym == 1073742054)) {
                }
                else {
                    quitting_time = true;
                    cout<<"Exiting due to key press (keysym "<<int(event.key.keysym.sym)<<")"<<endl;
                }
            }
        }
        //framerate = 20.0;
        //If at least 1/framerate of a second has passed, render the image
        cur_time = SDL_GetTicks();
        if((1000.0 * float(frame)) / framerate + float(start_time) <= float(cur_time) ) {
            last_seen_gen = gen;
            ++frame;
            if(frame%10 == 0)
                cout<<"Frame: "<<frame<<" Generation: "<<gen<<endl;
            SDL_UpdateTexture(tex, NULL, &((*fbuf)[0]), w * sizeof(uint32_t));
            SDL_RenderClear(rend);
            SDL_RenderCopy(rend, tex, NULL, NULL);
            SDL_RenderPresent(rend);
        }

        //update the map (loop through changelist and process for cells that *actually* changed)
        
        swap(changes, old_changes);
        if(showcounts == true) {
            cout<<rep_count<<" "<<old_change_count<<">>"<<change_count<<endl;
        }
        if(change_count == old_change_count || count_hist[gen % 60] == (int32_t(change_count) - int32_t(old_change_count))) {
            if(change_count == old_change_count) reason = 1;
            else reason = 2;
            rep_count++;
            if(rep_count > 500 && !informed_of_end) {
                cout<<"Simulation appears stable around generation "<<gen-500<<"."<<endl;
                informed_of_end = true;
            }
        }
        else {
            rep_count = 0;
            reason = 0;
        }

        count_hist[gen % 60] = int32_t(change_count) - int32_t(old_change_count);

        old_change_count = change_count;
        change_count = 0;
        fill(test.begin(), test.end(), false);
        copy(fbuf->begin(), fbuf->end(), bbuf->begin());
        for(int i=0;i<old_change_count;++i) {
            uint32_t index = old_changes[i];
            uint32_t row = index/w;
            uint32_t col = index%w;
            uint32_t left = ((col == 0)?w-1:col-1);
            uint32_t right = ((col == w-1)?0:col+1);
            uint32_t top = ((row == 0)?h-1:row-1) * w;
            uint32_t bottom = ((row == h-1)?0:row+1) * w;
            row *= w;

            uint64_t count = (*fbuf)[left+top]; 
            count +=         (*fbuf)[col+top]; 
            count +=         (*fbuf)[right+top];
            count +=         (*fbuf)[left+row];
            count +=         (*fbuf)[right+row];
            count +=         (*fbuf)[left+bottom];
            count +=         (*fbuf)[col+bottom];
            count +=         (*fbuf)[right+bottom]; 

            //cout<<count<<endl<<count/UINT32_MAX<<endl;
            count/=(UINT32_MAX);
            bool status = ((*fbuf)[index] > 0);
            bool change = false;
            if((status && (count < 2 || count > 3))) {//cell will die
                (*bbuf)[index] = 0;
                change = true;
            }
            else if(!status && count == 3) { //cell will come alive
                (*bbuf)[index] = UINT32_MAX;
                change = true;
            }

            if(change) {
                uint32_t neighbors[] = {left+top,    col+top,    right+top,
                                        left+row,    col+row,    right+row,
                                        left+bottom, col+bottom, right+bottom};
                for(uint32_t j = 0; j < 9; ++j) {
                    if(!test[neighbors[j]]) {
                        test[neighbors[j]] = true;
                        changes[change_count++] = neighbors[j];
                    }
                }
            }
        }

        gen++;
        swap(fbuf,bbuf);

    }
    cout<<"Calculated "<<gen<<" generations in "<<frame<<" frames"<<endl;
    cur_time = SDL_GetTicks();
    float duration = float(cur_time - start_time) / 1000.0;
    cout<<float(gen)/duration<<" gen/s, "<<float(frame)/duration<<" fps (requested "<<framerate<<" fps)"<<endl;
    if(!quitting_time) {
        cout<<"Press any key to exit"<<endl;
        if(reason == 1) { //If it just hit the end of the thing, restart it
            goto restartit;
        }
    }
    while(!quitting_time) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_KEYDOWN || event.type == SDL_QUIT) {
                if(event.type == SDL_KEYDOWN && event.key.keysym.sym == ' ') {
                    showcounts = !showcounts;
                }
                else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == 'r') {
                    cout<<endl<<endl<<"Restarting a new simulation..."<<endl;
                    goto restartit;
                }
                else if(event.type == SDL_KEYDOWN && (event.key.keysym.sym == 1073742048 || event.key.keysym.sym == 1073742050 || event.key.keysym.sym == 1073742052 || event.key.keysym.sym == 1073742054)) {
                }
                else {
                    quitting_time = true;
                    cout<<"Exiting due to key press (keysym "<<int(event.key.keysym.sym)<<")"<<endl;
                }
            }
        }
        SDL_Delay(500);
        swap(fbuf,bbuf);
        SDL_UpdateTexture(tex, NULL, &((*fbuf)[0]), w * sizeof(uint32_t));
        SDL_RenderClear(rend);
        SDL_RenderCopy(rend, tex, NULL, NULL);
        SDL_RenderPresent(rend);
    }
    SDL_Quit();
    return 0;
}
