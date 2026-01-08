#pragma once
#include <string>
#include <vector>
#include <windows.h>
using std::string;
using std::vector;

/*** Forward declarations khớp với project ***/
struct nodeMH;          // định nghĩa thật ở thitracnghiem.h
typedef nodeMH* TREE_MH;

struct DS_LOP;          // định nghĩa thật ở thitracnghiem.h
struct MonHoc;

namespace util {
    void ensureDir(const string& dir);
    vector<string> split(const string& s, char delim);
    string trim(string s);
    string upper(string s);
    long long randId();
}

// Một số util dùng bởi thitracnghiem.cpp
void clearConsole();
void clearStdin();
void seedRandOnce();
void pauseScreen();
void waitEsc(const char* msg = "\nNhan ESC de quay lai...");
void gotoXY(short x, short y);
void drawAnswerLine(const char* prefix, const char* text, int y, bool highlight);
void drawAnswerLine(const char* prefix, const char* text, int y, bool highlight,bool daChon);
void setConsoleSize(short w, short h);
void printCenter(short y, const char* text, short width);
int readKey(); // đọc phím, phân biệt Arrow/Enter/Esc/Backspace
void inputField(char* out, int maxLen, short x, short y, bool passwordMode, bool active, bool showPass);
bool loginForm(char* user, int userMax, char* pass, int passMax, short boxX, short boxY);
void getConsoleWindowSize(short &cols, short &rows);


int  randomID();
constexpr int KEY_ESC = 27;
struct CauHoi;
int chonDapAnBangMuiTen(const CauHoi &q, int yStart);
std::string fileCauHoiTheoMon(const char* mamh);
void docFileMonLop();
/*** Các hàm nạp dữ liệu từ .txt ***/
bool loadMonHoc (const char* path, TREE_MH &dsMH);
bool loadLop    (const char* path, DS_LOP &dsLop);
bool loadSinhVien(const char* path, DS_LOP &dsLop);
bool loadCauHoi (const char* path, TREE_MH &dsMH);
bool loadDiemThi(const char* path, DS_LOP &dsLop, TREE_MH &dsMH);
bool saveCauHoiTheoMon(const char* mamh, const MonHoc& mh);
bool saveSinhVien(const char* path, const DS_LOP& dsLop);
bool saveMonHoc(const char* path, TREE_MH dsMH);




