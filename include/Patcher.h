#ifndef PATCHER_H
#define PATCHER_H
#include <algorithm>
#include <fstream>
#include <iostream>

enum class Result {
    OK, LibWrong, LibInvalid, VerInvalid, PfmInvalid, ApproxReq, FileError
};

/*
### PATCHER RETURN CODES ###
    1. OK - you are *probably* good
    2. Wrong library (e.g. armeabi-v7a for 1.41 or x86 for 1.00)
    3. Invalid library - nonexistent ID for library (any ID outside the range 0-2)
    4. Invalid version - pretty much the same as LibInvalid but for version IDs
    5. Invalid platform - upcoming feature...
    6. Approximation question request (for the corresponding dialog)
    7. File error - the file couldn't be opened
*/

class Patcher {
public:
    // Constructors
    Patcher();
    Patcher(std::string filename, unsigned obj, int ver, int lib, unsigned approx = 0);

    // Functions
    Result Patch();
    void Prepare(std::string filename, unsigned obj, int ver, int lib);
    unsigned Approx(unsigned obj, bool type = 0);
    static int GetAutoVer(std::string filename);

    #ifdef DEBUG
    void Test(); // listen and do what I said...
    #endif // DEBUG

    // Fields
    std::fstream m_file;
    std::string m_filename;
    unsigned m_obj = 16384;
    int m_ver = -2; // because -1 is used by autover (might change that)
    int m_lib = -1;
    unsigned m_approx = 0;

protected:
    // Functions
    void Write(int i, void f(char* a), int n = 4);
    void Write(int i, std::string s);
    static void InitPows();
    static void Dword(char* a);
    static void Mov(char* a);
    static void Cmp(char* a);
    bool CheckApprox(unsigned n);

    // Fields
    static inline std::vector<unsigned> Pow;
    static inline unsigned obj;
    static inline int ver;
    static inline int lib;
};

#endif // PATCHER_H
