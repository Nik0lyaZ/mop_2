#include "Patcher.h"
#define THUMB_NOP "x00\xBF"
#ifdef DEBUG
#include <wx/wx.h>
#endif // DEBUG
using namespace std;

#ifdef DEBUG
void Patcher::Test() {
    m_file.open("test.txt", ios::in | ios::out | ios::binary);
    Write(0, "\x0\x0\x0\x0");

    obj = 5000;
    Write(0, Cmp);

    m_file.close();
}
#endif // DEBUG

Patcher::Patcher() {}

Patcher::Patcher(string filename, unsigned obj, int ver, int lib, unsigned approx, bool visual) {
    Prepare(filename, obj, ver, lib);
    m_approx = approx;
    m_visual = visual;
}

void Patcher::Prepare(string filename, unsigned obj, int ver, int lib) {
    m_filename = filename;
    m_obj = obj;
    m_ver = ver;
    m_lib = lib;
}

// A simple function for writing bytes!
void Patcher::Write(int i, void f(char* a), int n) {
    if(!m_file) return;
    m_file.seekg(i, ios::beg);
    char buf[n];
    m_file.read(buf, n);
    if (f) f(buf);
    m_file.seekp(i, ios::beg);
    m_file.write(buf, n);
}

void Patcher::Write(int i, string s) {
    if(!m_file) return;
    m_file.seekp(i, ios::beg);
    for (auto x : s) {
        m_file.put(x);
    }
}

void Patcher::InitPows() {
    if (Pow.size() > 0) return;
    for (int i = 0; i < 31; i++) {
        Pow.push_back(1 << i);
    }
}

// Turns the object count into a DWORD
void Patcher::Dword(char* a) {
    int sum = 0, n = obj;
    for (int i = 0; i < 8; i++) {
        if (i % 2) {
            sum += n % 16 << 4;
            *a++ = sum;
        } else {
            sum = n % 16;
        }
        n >>= 4;
    }
}

// Replaces the value in MOVW instruction bytes
void Patcher::Mov(char* a) {
    obj = min(obj, 65535U);
    unsigned char reg = *(a + 3);
    if ((*a) >> 4 != 4) {
        if (ver > 0) {
            *(a + 4) = *(a + 2);
            *(a + 5) = *(a + 3);
        }
        reg = *(a + 1);
    }
    reg %= 0x10;
    *a++ = 0x40 + (obj >> 12);
    *a++ = obj % 4096 < 2048 ? 0xF2 : 0xF6;
    *a++ = obj % 256;
    *a = ((obj >> 8) % 8 << 4) + reg;
}

// Replaces the value in CMP.W instruction bytes
void Patcher::Cmp(char* a) {
    InitPows();
    unsigned char reg = *a;
    reg %= 0x10;
    *a++ = 0xB0 + reg;
    if (obj < 256) {
        *a++ = 0xF1;
        *a++ = obj;
        *a++ = 0x0F;
    } else {
        *a++ = obj < 0xFF0000 ? 0xF5 : 0xF1;
        int base = Pow[upper_bound(Pow.begin(), Pow.end(), obj) - Pow.begin() - 1];
        if (obj < INT_MAX) *a++ = obj / (base >> 7) - ((lower_bound(Pow.begin(), Pow.end(), base >> 9) - Pow.begin() + 1) % 2 << 7);
        else *a++ = 0x0;
        *a++ = 0x7F - ((upper_bound(Pow.begin(), Pow.end(), obj >> 8) - Pow.begin() - 1) >> 1 << 4) - ((obj >> 24) ? 0x80 : 0x00);
    }
}

bool Patcher::CheckApprox(unsigned obj) {
    if (obj > INT_MAX) return obj;
    if (obj < 256) return 1;
    InitPows();
    int base = Pow[upper_bound(Pow.begin(), Pow.end(), obj) - Pow.begin() - 1] >> 7;
    return !(obj % base);
}

unsigned Patcher::Approx(unsigned obj, bool type) {
    if (obj > INT_MAX) return obj;
    if (obj < 256) return obj + type;
    InitPows();
    int base = Pow[upper_bound(Pow.begin(), Pow.end(), obj) - Pow.begin() - 1] >> 7;
    return obj - obj % base + (type ? base : 0);
}

int Patcher::GetAutoVer(string filename) {
    ifstream file(filename, ios::ate | ios::binary);
    if (!file) return -2;
    int filesize = file.tellg();
    // This byte is checked when multiple vers share the file size
    // The first 64 bytes are used by file headers, so they are the same
    file.seekg(64, ios::beg); char b = file.tellg();
    file.close();
    switch(filesize) {
        case 5551628: return 0;
        case 4026224: return 1;
        case 4093996: return 2;
        case 4154008: return 3;
        case 4174700: return 4;
        case 4257648: return 5;
        case 4261752: return 6;
        case 4261760: return 7;
        case 4407700: return 8;
        case 4549112: return 9;
        case 4549160: return 10;
        case 4337016: return b == 0xE0 ? 11 : 12;
        case 4279688: return b == 0x50 ? 111 : 112;
        case 6263508: return 211;
        case 6267604: return 212;
        case 4709752: return 13;
        case 4648328: return 113;
        case 6685396: return 213;
        case 5041040: return b == 0x58 ? 14 : 15;
        case 4950928: return b == 0x64 ? 114 : 115;
        case 7728580: return b == 0x70 ? 214 : 215;
        case 5620308: return b == 0x98 ? 16 : 17;
        case 5497432: return b == 0x6C ? 116 : b == 0x34 ? 117 : 118;
        case 8765380: return b == 0x00 ? 216 : b == 0xC8 ? 217 : 218;
        case 5624404: return 18;
    }
    return -1;
}

Result Patcher::Patch() {
    m_file.open(m_filename, ios::in | ios::out | ios::binary);
    if (!m_file) return Result::FileError;

    obj = m_visual ? m_approx : m_obj;
    ver = m_ver, lib = m_lib;

    switch(lib) {
        /*
            Counter   - EditorPauseLayer::init
            Popup     - EditorUI::showMaxError
            Create    - EditorUI::onCreate
            Duplicate - EditorUI::onDuplicate
            100kB     - cocos2d::CCString::initWithFormatAndValist
        */

        default: return Result::LibInvalid;

        // armeabi
        case 0: {

            // 1.00 - 1.22 (1.00 is compiled differently)
            if (ver < 8) {
                static const int TABLE[4][8] = {
                    {0x1C4DAC, 0x14464C, 0x148844, 0x14B454, 0x14D7E0, 0x14F56C, 0x14F884, 0x14FADC},
                    {0x1C4348, 0x147048, 0x14B240, 0x14DE84, 0x150210, 0x151FB0, 0x1522C8, 0x152520},
                    {0x298B11, 0x1830BF, 0x18A38F, 0x18F027, 0x19341F, 0x195467, 0x195787, 0x1959EF},
                    {0x298B27, 0x1830D5, 0x18A3A5, 0x18F03D, 0x193435, 0x19547D, 0x19579D, 0x195A05}
                };
                Write(TABLE[0][ver], Mov, ver == 0 ? 4 : 6); // Popup
                --obj;
                Write(TABLE[1][ver], Dword); // Create
                Write(TABLE[2][ver], "\x03"); // 100kB (1)
                Write(TABLE[3][ver], "\x03"); // 100kB (2)
            }

            // 1.3 - 1.41: Popup uses DWORD
            else if (ver < 11) {
                static const int TABLE[4][3] = {
                    {0x1544B0, 0x15861C, 0x15864C},
                    {0x156BD8, 0x15BE78, 0x15BEA8},
                    {0x1A27D7, 0x1A9BDF, 0x1A9C0F},
                    {0x1A27ED, 0x1A9BF5, 0x1A9C25}
                };
                Write(TABLE[0][ver - 8], Dword); // Popup
                --obj;
                Write(TABLE[1][ver - 8], Dword); // Create
                Write(TABLE[2][ver - 8], "\x03"); // 100kB (1)
                Write(TABLE[3][ver - 8], "\x03"); // 100kB (2)
            }

            // 1.50 - 1.51: Multiple libraries, fixed level uploading
            else if (ver < 13) {
                static const int TABLE[3][2] = {
                    {0x1482B8, 0x148478},
                    {0x149E74, 0x14A034},
                    {0x14897C, 0x148B3C}
                };
                Write(TABLE[0][ver - 11], Dword); // Popup
                --obj;
                Write(TABLE[1][ver - 11], Dword); // Create
                Write(TABLE[2][ver - 11], Dword); // Duplicate
            }

            // 1.6: Popup uses MOV, added object counter
            else if (ver < 14) {
                static const int TABLE[4][1] = {
                    {0x168E3A},
                    {0x150CA4},
                    {0x1529E4},
                    {0x1513A4}
                };
                Write(TABLE[0][ver - 13], Mov); // Counter
                Write(TABLE[1][ver - 13], Mov); // Popup
                --obj;
                Write(TABLE[2][ver - 13], Dword); // Create
                Write(TABLE[3][ver - 13], Dword); // Duplicate
            }

            // 1.70 - ?: everything uses DWORD
            else if (ver < 19) {
                static const int TABLE[4][5] = {
                    {0x1BA5A4, 0x1BA768, 0x1FB6CC, 0x1FB6CC, 0x1FBAA8},
                    {0x19DF54, 0x19E058, 0x1DA9C4, 0x1DA9BC, 0x1DADA0},
                    {0x19FD80, 0x19FF1C, 0x1E2184, 0x1E2180, 0x1E255C},
                    {0x19E888, 0x19E9BC, 0x1E2BB0, 0x1E2BAC, 0x1E2F7C}
                };
                Write(TABLE[0][ver - 14], Dword); // Counter
                Write(TABLE[1][ver - 14], Dword); // Popup
                --obj;
                Write(TABLE[2][ver - 14], Dword); // Create
                Write(TABLE[3][ver - 14], Dword); // Duplicate
            }

            else return Result::VerInvalid;
            break;
        }

        // armeabi-v7a
        case 1: {

            // Pre-1.5 versions don't have armeabi-v7a lib
            if (ver < 11) return Result::LibWrong;

            // 1.50 - 1.51
            else if (ver < 13) {
                static const int TABLE[5][2] = {
                    {0x145EEE, 0x1460AA},
                    {0x147B1A, 0x147CD6},
                    {0x14659E, 0x14675A},
                    {0x147B12, 0x147CCE},
                    {0x146598, 0x146754}
                };
                Write(TABLE[0][ver - 11], Mov); // Popup
                Write(TABLE[1][ver - 11], obj == 0 ? THUMB_NOP : "\x02\xDD"); // Zero object toggle (Create)
                Write(TABLE[2][ver - 11], obj == 0 ? THUMB_NOP : "\xEA\xDD"); // (Duplicate)
                if (obj) --obj;
                Write(TABLE[3][ver - 11], Mov); // Create
                Write(TABLE[4][ver - 11], Mov); // Duplicate
            }

            // 1.6 amreabi-v7a uses CMP.W instruction
            else if (ver < 14) {
                static const int TABLE[6][1] = {
                    {0x166130},
                    {0x14E2C2},
                    {0x150050},
                    {0x14E982},
                    {0x15004A},
                    {0x14E97E}
                };
                if (!CheckApprox(obj)) {if (!m_approx) {m_file.close(); return Result::ApproxReq;}}
                else obj = m_visual ? m_approx : m_obj;
//                wxLogMessage("visual=%d, resultobj=%d", m_visual, obj);
                Write(TABLE[0][ver - 13], Mov); // Counter
                Write(TABLE[1][ver - 13], Mov); // Popup
                Write(TABLE[2][ver - 13], obj == 0 ? THUMB_NOP : "\x02\xDD"); // Zero object toggle (Create)
                Write(TABLE[3][ver - 13], obj == 0 ? THUMB_NOP : "\xEB\xDB"); // (Duplicate)
//                wxLogMessage("%d and %d, sir", m_approx, obj);
                obj = m_approx; // Using approximation
                if(obj <= 256) {
                    if (obj) --obj;
                    Write(0x150050, "\x02\xDB\xFE\xF7\x35\xF9\x03\xE0\xD4\xF8\xC4\x11\xFF");
                    Write(0x15005E, "\xA8\xFE");
                } else {
                    Write(0x150050, "\x04\xDA\xD4\xF8\xC4\x11\xFF\xF7\xAB\xFE\x01\xE0\xFE");
                    Write(0x15005E, "\x30\xF9");
                }
                Write(TABLE[4][ver - 13], Cmp); // Create
                Write(TABLE[5][ver - 13], Cmp); // Duplicate
            }

            // 1.70 - ?
            else if (ver < 19) {
                static const int TABLE[6][5] = {
                    {0x1B683A, 0x1B6A0A, 0x1F6462, 0x1F645A, 0x1F67DE},
                    {0x19A996, 0x19AAA6, 0x1D6282, 0x1D6276, 0x1D65EA},
                    {0x19C80E, 0x19C9B2, 0x1DDABC, 0x1DDAB4, 0x1DDE28},
                    {0x19B2A8, 0x19B3F4, 0x1DE48C, 0x1DE484, 0x1DE7F8},
                    {0x19C806, 0x19C9AA, 0x1DDAB6, 0x1DDAAE, 0x1DDE22},
                    {0x19B2A2, 0x19B3EE, 0x1DE486, 0x1DE47E, 0x1DE7F2}
                };
                Write(TABLE[0][ver - 14], Mov); // Counter
                Write(TABLE[1][ver - 14], Mov); // Popup
                string THUMB_BLE;
                switch(ver) {
                    case 14:
                    case 15: THUMB_BLE = "\x02\xDD"; break;
                    case 16:
                    case 17:
                    case 18: THUMB_BLE = "\x03\xDD"; break;
                }
                Write(TABLE[2][ver - 14], (obj == 0 ? THUMB_NOP : THUMB_BLE)); // Zero object toggle (Create)
                switch(ver) {
                    case 14: THUMB_BLE = "\xE8\xDD"; break;
                    case 15: THUMB_BLE = "\xCA\xDD"; break;
                    case 16:
                    case 17:
                    case 18: THUMB_BLE = "\xE5\xDD"; break;
                }
                Write(TABLE[3][ver - 14], (obj == 0 ? THUMB_NOP : THUMB_BLE)); // (Duplicate)
                if (obj) --obj;
                Write(TABLE[4][ver - 14], Mov); // Create
                Write(TABLE[5][ver - 14], Mov); // Duplicate
            }

            else return Result::VerInvalid;
            break;
        }

        // x86
        case 2: {

            // Pre-1.5 versions don't have x86 lib
            if (ver < 11) return Result::LibWrong;

            // 1.50 - 1.51
            else if (ver < 13) {
                static const int TABLE[3][2] = {
                    {0x19966A, 0x19987A},
                    {0x19D3B6, 0x19D5C6},
                    {0x19A4A7, 0x19A6B7}
                };
                Write(TABLE[0][ver - 11], Dword); // Popup
                --obj;
                Write(TABLE[1][ver - 11], Dword); // Create
                Write(TABLE[2][ver - 11], Dword); // Duplicate
            }

            // 1.6
            else if (ver < 14) {
                static const int TABLE[4][1] = {
                    {0x1DA8B7},
                    {0x1A516A},
                    {0x1A9106},
                    {0x1A5FB7}
                };
                Write(TABLE[0][ver - 13], Dword); // Counter
                Write(TABLE[1][ver - 13], Dword); // Popup
                --obj;
                Write(TABLE[2][ver - 13], Dword); // Create
                Write(TABLE[3][ver - 13], Dword); // Duplicate
            }

            // 1.70 - ?
            else if (ver < 19) {
                static const int TABLE[4][5] = {
                    {0x217776, 0x217986, 0x263AC6, 0x263A16, 0x263EA6},
                    {0x1DA88A, 0x1DA8EA, 0x22C7EE, 0x21E98A, 0x21EDFA},
                    {0x1DE526, 0x1DE6A6, 0x22C7EE, 0x22C73E, 0x22CBAE},
                    {0x1DBC97, 0x1DBCFB, 0x22DC17, 0x22DB67, 0x22DFD7}
                };
                Write(TABLE[0][ver - 14], Dword); // Counter
                Write(TABLE[1][ver - 14], Dword); // Popup
                --obj;
                Write(TABLE[2][ver - 14], Dword); // Create
                Write(TABLE[3][ver - 14], Dword); // Duplicate
            }

            else return Result::VerInvalid;
            break;
        }
    }

    m_file.close();
    return Result::OK;
}
