#include<iostream>
#include<fstream>
using namespace std;

int main(int argc, char *argv[]) {
    if(argc != 3) {
        cout<<"Provide 2 filenames to compare."<<endl;
        return 1;
    }

    ifstream in1(argv[1]);
    if(!in1.is_open()) {
        cout<<"Couldn't open file \""<<argv[1]<<"\"."<<endl;
        return 2;
    }

    int size1 = 0;
    in1.seekg(0,ios::end);
    size1 = in1.tellg();
    in1.seekg(0,ios::beg);

    ifstream in2(argv[2]);
    if(!in2.is_open()) {
        cout<<"Couldn't open file \""<<argv[2]<<"\"."<<endl;
        return 2;
    }

    int size2 = 0;
    in2.seekg(0,ios::end);
    size2 = in2.tellg();
    in2.seekg(0,ios::beg);

    char buf1[512] = {0};
    char buf2[512] = {0};

    bool samesize = (size1 == size2);
    int mismatch = 0;
    int chunks = min(size1,size2)/512;
    int rem = min(size1,size2)%512;

    for(int i=0; i<chunks;i++) {
        in1.read(buf1,512);
        in2.read(buf2,512);
        for(int j=0;j<512;j++) {
            if(buf1[j] != buf2[j]) {
                cout<<"Mismatch at "<<i*512+j<<endl;
                mismatch++;
            }
        }
    }

    in1.read(buf1,rem);
    in2.read(buf2,rem);
    for(int i=0;i<min(size1,size2)%512;i++) {
        if(buf1[i] != buf2[i]) {
            cout<<"Mismatch at "<<chunks*512+i<<endl;
            mismatch++;
        }
    }

    if(samesize && mismatch==0) {
        cout<<"Files are identical."<<endl;
    }
    else if(mismatch==0) {
        cout<<"Files are identical, up to the size of the smaller file."<<endl;
    }
    else {
        cout<<mismatch<<" bytes were different."<<endl;
    }

    return 0;
}
