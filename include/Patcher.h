#ifndef PATCHER_H
#define PATCHER_H
#include <iostream>

enum class Result {
    OK, LibWrong, LibInvalid, VerInvalid, PfmInvalid, ApproxReq, FileError
};

class Patcher {
public:
    Result Patch(std::string filename, int ver, unsigned obj, int lib = 0, unsigned approx = 0, bool visual = false);
    static unsigned Approx(unsigned obj, bool type = 0);
    static int GetAutoVer(std::string filename);
};

/*
### PATCHER RETURN CODES ###
    1. OK - you are *probably* good
    2. Wrong library (e.g. armeabi-v7a for 1.41 or x86 for 1.00)
    3. Invalid library - nonexistent ID for library (any ID outside the range 0-2)
    4. Invalid version - pretty much the same as LibInvalid but for version IDs
    5. Invalid platform - upcoming feature...
    6. Requested approximation question - needed for the corresponding dialog
    7. File error - the file couldn't be opened...
*/

#endif // PATCHER_H
