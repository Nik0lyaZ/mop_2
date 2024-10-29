#ifndef PATCHER_H
#define PATCHER_H
#include <iostream>

class Patcher
{
    public:
        int Patch(std::string filename, int ver, unsigned obj, int lib = 0, unsigned approx = 0, bool visual = false);
        static unsigned Approx(unsigned obj, bool type = 0);
        static int GetAutoVer(std::string filename);
};

/* ### PATCHER ERROR CODES ###
    0 - invalid library for specified version
    1 - success
    2 - approximation problem
    3 - file could not be openned
    4 - invalid library
    5 - invalid version
*/

#endif // PATCHER_H
