#include<iostream>
#include<fstream>
#include<vector>
#include<cstdint>
#include<string>
#include<cstring>
using namespace std;

int exp_size(vector<uint8_t>& in) {
    int retval = 0;
    for(int i=0;i<in.size();i++) {
        if(in[i] == 0xff) {
            retval += in[i+1];
            i+=2;
        }
        else {
            retval++;
        }
    }
    return retval;
}

void expand(vector<uint8_t>& in, vector<uint8_t>& out) {
    int out_ptr = 0;
    for(int i=0;i<in.size();i++) {
        if(in[i] == 0xff) {
            memset(&out[out_ptr], in[i+2], in[i+1]);
            out_ptr+=in[i+1];
            if(in[i+1] == 0) {
                cerr<<"Found a 0-length \"run\" command"<<endl;
            }
            i+=2;
        }
        else {
            out[out_ptr] = in[i];
            out_ptr++;
        }
    }
}

int main(int argc, char **argv) {
    if(argc != 2) {
        cerr<<"Provide a single filename."<<endl;
        return 1;
    }

    ifstream in(argv[1]);
    if(!in.is_open()) {
        cerr<<"Problem opening the file."<<endl;
        return 1;
    }

    ofstream out((string(argv[1])+".exp").c_str());

    in.seekg(0,ios::end);
    int in_size = in.tellg();
    in.seekg(0);

    vector<uint8_t> inv(in_size);
    in.read(reinterpret_cast<char *>(&inv[0]), in_size);
    in.close();
    
    vector<uint8_t> outv(exp_size(inv));
    expand(inv, outv);

    out.write(reinterpret_cast<char *>(&outv[0]), outv.size());
    out.close();

    return 0;
}
