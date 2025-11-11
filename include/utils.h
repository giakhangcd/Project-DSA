#pragma once
#include "khai_bao.h"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;
using std::string;
using std::vector;

using namespace std;

/*** Forward declarations khớp với project ***/
struct nodeMH;          // định nghĩa thật ở thitracnghiem.h
typedef nodeMH* TREE_MH;

struct DS_LOP;          // định nghĩa thật ở thitracnghiem.h

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

int  randomID();
constexpr int KEY_ESC = 27;

/*** Các hàm thao tác với dữ liệu từ .txt ***/
void loaddulieu_sv_lop(const string &folder_name, DS_LOP &ds);
void ghidulieu_sv_lop(const string &folder_name, DS_LOP &ds);
void loaddulieumonhoc();
void luumotmonhoc(const MonHoc& mh, const fs::path &folder_mh);
void ghidulieumonhoc(TREE_MH first, const string& folder_name);
