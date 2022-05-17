#include <string>
#include <fstream>
#include <ctime>
#include <iostream>

#define KEY_SIZE 32
#define BUFF_SIZE 1024

unsigned int holdrand = 0;

unsigned int firstSeed =    1646082000;
unsigned int lastSeed =     1648760400;
unsigned int correct =      1647941775;

void Srand (unsigned int seed) {
    holdrand = seed;
    //printf("seed=%d\n", seed);
}

int Rand () {
    return(((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}

char* genKey() {
    int i;
    static char key[KEY_SIZE + 1];
    const char charset[] = "abcdefghijklmnopqrstuvwxyz"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "123456789";

    for(i = 0; i < KEY_SIZE; i++) {
        key[i] = charset[Rand() % (sizeof(charset) - 1)];
    }
    key[KEY_SIZE] = '\0';

    return key;

}

void crypt_buffer(char *buffer, size_t size, char *key) {
    size_t i;
    int j;

    j = 0;
    for(i = 0; i < size; i++) {
        if(j >= KEY_SIZE)
            j = 0;
        buffer[i] ^= key[j];
        j++;
    }
}

std::string crypt_file(std::fstream& fin) {
    char* buffer = new char[BUFF_SIZE];
    char *key;
    size_t size;
    std::string res;

    key = genKey();

    do {
        fin.read(buffer, BUFF_SIZE);
        if (fin.gcount() >= BUFF_SIZE) {
            size = BUFF_SIZE;
        } else {
            size = fin.gcount();
        }
        //size = fread(buffer, 1, BUFF_SIZE, in);
        crypt_buffer(buffer, size, key);
        res += buffer;
        //fwrite(buffer, 1, size, out);

    }while(size == BUFF_SIZE);

    return res;
}

std::istream& safeGetline(std::istream& is, std::string& t)
{
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for(;;) {
        int c = sb->sbumpc();
        switch (c) {
            case '\n':
                return is;
            case '\r':
                if(sb->sgetc() == '\n')
                    sb->sbumpc();
                return is;
            case std::streambuf::traits_type::eof():
                // Also handle the case when the last line has no line ending
                if(t.empty())
                    is.setstate(std::ios::eofbit);
                return is;
            default:
                t += (char)c;
        }
    }
}

int main() {
    std::string fileName = "secret.bz2.crypt";
    std::string in;
    std::string inputBuffer;// = new char[1024];
    int n = 0;

    std::fstream fin(fileName, std::ios::in | std::ios::out | std::ios::binary);
    /*while(!safeGetline(fin, in).eof()) {
        ++n;
        inputBuffer += in;
    }
    inputBuffer += "\n";*/

    //fin.read(inputBuffer, 167);
    while (n <= 6) {
        n++;
        fin >> in;
        inputBuffer += in;
    }
    /*fin >> inputBuffer;
    fin >> inputBuffer;
    fin >> inputBuffer;*/
    /*while(inputBuffer.size() < 168) {
        inputBuffer += fin.rdbuf()->sbumpc();
    }*/

    fin.close();
    std::fstream fout("secret.bz2", std::ios::in | std::ios::out | std::ios::binary);
    fout << inputBuffer;
    fout.close();

    std::string result;

    std::cout << "Searching from " << lastSeed - firstSeed << " seeds" << std::endl;
    for (unsigned int i = correct; i < lastSeed; i++) {
        Srand(i);

        fin.open(fileName);
        result = crypt_file(fin);
        fin.close();

        /*fout.open("secret.bz2");
        fout << result;
        fout.close();*/


        if ((lastSeed - firstSeed) % 100000 == 0) {
            //searched += 100000;
            std::cout << "#";
        }

        if (i % 100000 == 0) {
            //searched += 100000;
            std::cout << "#";
        }

        if ("BZh" == result.substr(0, 3)) {
            std::cout << "Found B using seed " << i << std::endl;
            fout.open("secret.bz2");
            fout << result;
            fout.close();
        }


    }

    fin.close();
    fout.close();

    return 0;
}