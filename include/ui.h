#pragma once
#include "utils.h"
#include "thitracnghiem.h"
#include <vector>
#include <iostream>
#include <cstring>
#include <chrono>
#include <string>
#include <vector>
#include <windows.h>

struct UILayout {
    short CW, CH;
    short boxX, boxY, boxW, boxH;
};

// Menu
void menuChinh(TREE_MH &dsMH, DS_LOP &dsLop);

bool ValidateMonHoc(const MonHoc& mh, TREE_MH root);
bool validateSinhVien(const SinhVien &sv, std::string &msg);
bool validateCauHoi(const CauHoi &ch, std::string &msg);

void drawSidePanel(short x, short y, short w, short h,int soCau, int idx, const std::vector<char>& ans);
void drawQuestionMain(const UILayout& L,int idx, int soCau,const CauHoi& q,int curOpt,char chosen);

UILayout makeCenterBox(short boxW, short boxH);
void drawScreenBox(const char* title, const UILayout& L);
void beginScreenBox(const char* title, short boxW, short boxH, UILayout& L);
// Layout chung cho các màn hình chức năng
void boxPrint(const UILayout& L, short dx, short dy, const char* fmt, ...);

void drawOptionLineAt(short x, short y,const char* text,bool highlight,int width);
void clearRect(short x, short y, short w, short h);
void drawBox(short x, short y, short w, short h);
void gotoXY(short x, short y);
void hideCursor(bool hide);
void clearStdin();
void seedRandOnce();
void pauseScreen();
void drawAnswerLine(const char* prefix, const char* text, int y, bool highlight);
void drawAnswerLine(const char* prefix, const char* text, int y, bool highlight,bool daChon);
void setConsoleSize(short w, short h);
void printCenter(short y, const char* text, short width);
int readKey(); // đọc phím, phân biệt Arrow/Enter/Esc/Backspace
bool loginForm(char* user, int userMax, char* pass, int passMax, short boxX, short boxY);
void getConsoleWindowSize(short &cols, short &rows);
int  randomID();
struct CauHoi;
int chonDapAnBangMuiTen(const CauHoi &q, int yStart);
std::string fileCauHoiTheoMon(const char* mamh);
char docDapAnCoTimer(int tongGiay, std::chrono::steady_clock::time_point batDau);
void inDapAnSauKhiNop(char ma, const char* noiDung, char daChon, char daDung);
void hienThiBaiThiSauKhiNop(const CauHoi dsCau[], const char dsDaChon[], int soCau, int soDung);
void setColor(int attr);
