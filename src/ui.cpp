#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "thitracnghiem.h"
#include "ui.h"
#include "utils.h"
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <fstream>
#include <limits>
#include <iomanip>
#include <cstring>    
#include <cstdio>   
#include <cstdlib>   
#include <ctime>     
#include <chrono>
#include <thread>

using namespace std;


void inDapAnSauKhiNop(char ma, const char* noiDung, char daChon, char daDung)
{
    cout << "  ";
    cout << ma << ". " << noiDung;

    if (daChon == ma) {
        if (daChon == daDung)
            cout << "  (Dung)";
        else
            cout << "  (Sai)";
    }
    else if (daDung == ma) {
        cout << "  (Dap an dung)";
    }

    cout << "\n";
}

void hienThiBaiThiSauKhiNop(const CauHoi dsCau[], const char dsDaChon[], int soCau, int soDung) {
    system("cls");

    gotoXY(90, 2);
    cout << "DIEM: " << soDung << "/" << soCau;

    int y = 4;
    for (int i = 0; i < soCau; i++) {

        gotoXY(2, y++);
        cout << "Cau " << (i+1) << " (ID " << dsCau[i].ID << "):";

        gotoXY(2, y++);
        cout << dsCau[i].NoiDung;

        gotoXY(4, y++);
        inDapAnSauKhiNop('A', dsCau[i].A, dsDaChon[i], dsCau[i].DapAn);

        gotoXY(4, y++);
        inDapAnSauKhiNop('B', dsCau[i].B, dsDaChon[i], dsCau[i].DapAn);

        gotoXY(4, y++);
        inDapAnSauKhiNop('C', dsCau[i].C, dsDaChon[i], dsCau[i].DapAn);

        gotoXY(4, y++);
        inDapAnSauKhiNop('D', dsCau[i].D, dsDaChon[i], dsCau[i].DapAn);

        y++;
    }

    gotoXY(2, 28);
    cout << "Nhan ESC de quay lai menu...";
}

void clearStdin() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void seedRandOnce() {
    static bool seeded = false;
    if (!seeded) {
        std::srand((unsigned)std::time(nullptr));
        seeded = true;
    }
}

int randomID() {
    seedRandOnce();
    return 100000 + std::rand() % 900000; // 6 chữ số
}

void pauseScreen() {
    std::cin.clear();
    std::cin.ignore();
    std::cout << "\nNhan ESC đe quay lai...";
    std::cin.get();
}


void waitEsc(const char* msg) {
    std::cout << msg << std::flush;
    while (true) {
        int k =_getch();        // đọc phím ngay, không cần Enter
        if (k == 27)  //    27 = ESC
            break;
    }
}
void gotoXY(short x, short y) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(h, pos);
}

// simple cross-platform console color helper
// Windows: use SetConsoleTextAttribute; POSIX: emit basic ANSI sequences
void setColor(int attr) {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h != INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(h, static_cast<WORD>(attr));
    }
#else
    // Minimal mapping: support default (7) and white background black text (240).
    if (attr == 7) {
        std::cout << "\x1b[0m"; // reset
    } else if (attr == 240) {
        std::cout << "\x1b[30;47m"; // black on white
    } else {
        // Try a basic decode of Windows attr: fg = low 4 bits, bg = high 4 bits.
        int fg = attr & 0x0F;
        int bg = (attr >> 4) & 0x0F;
        // Map 0..7 to ANSI colors (0=black,1=blue,2=green,3=cyan,4=red,5=magenta,6=yellow,7=white)
        const int fg_map[8] = {30,34,32,36,31,35,33,37};
        int ansi_fg = 39; // default
        int ansi_bg = 49; // default
        if (fg < 8) ansi_fg = fg_map[fg];
        if (bg < 8) ansi_bg = 40 + bg;
        std::cout << "\x1b[" << ansi_fg << ";" << ansi_bg << "m";
    }
#endif
}


// in 1 đáp án, có/không highlight
// ===== CHỌN ĐÁP ÁN BẰNG MŨI TÊN (KHÔNG NHÁY MÀN HÌNH) =====

void drawAnswerLine(const char* prefix, const char* text, int y, bool highlight) {
    // gọi bản mới, nhưng mặc định daChon = false
    drawAnswerLine(prefix, text, y, highlight, false);
}

void drawAnswerLine(const char* prefix, const char* text, int y, bool highlight,bool daChon) {
    gotoXY(4, short(y));
    if (highlight) {
        setColor(240);} // nền trắng chữ đen
    else{
        setColor(7);
    }

    char buf[256];
    if (daChon) {
        printf("  [%s] %s    ", prefix, text);
    }
    else{
        printf("  %s %s  ", prefix, text);
    }

    printf(" %60", buf);  
    setColor(7);
}

void getConsoleWindowSize(short &cols, short &rows) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) { cols = 120; rows = 30; return; }

    if (!GetConsoleScreenBufferInfo(h, &csbi)) { cols = 120; rows = 30; return; }

    cols = (short)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    rows = (short)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
#else
    cols = 120;
    rows = 30;
#endif
}


void hideCursor(bool hide) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 20;
    info.bVisible = hide ? FALSE : TRUE;
    SetConsoleCursorInfo(h, &info);
}

void setConsoleSize(short w, short h) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD size;
    size.X=w;
    size.Y=h;
    SetConsoleScreenBufferSize(hOut, size);

    SMALL_RECT rect;
    rect.Left = 0;
    rect.Top = 0;
    rect.Right = w-1;
    rect.Bottom = h - 1;
    SetConsoleWindowInfo(hOut, TRUE, &rect);
}


UILayout makeCenterBox(short boxW, short boxH) {
    UILayout L{};
    getConsoleWindowSize(L.CW, L.CH);

    L.boxW = boxW;
    L.boxH = boxH;
    L.boxX = (L.CW - L.boxW) / 2;
    L.boxY = (L.CH - L.boxH) / 2;

    return L;
}

void beginScreenBox(const char* title, short boxW, short boxH, UILayout& L) {
    L = makeCenterBox(boxW, boxH);
    clearConsole();
    setColor(11);
    printCenter(L.boxY - 2, title, L.CW);
    setColor(7);
    drawBox(L.boxX, L.boxY, L.boxW, L.boxH);
}

void drawScreenBox(const char* title, const UILayout& L) {
    clearConsole();
    setColor(11);
    printCenter(L.boxY - 2, title, L.CW);
    setColor(7);
    drawBox(L.boxX, L.boxY, L.boxW, L.boxH);
}

void drawOptionLineAt(short x, short y, const char* text, bool highlight, int width) {
    gotoXY(x, y);

    if (highlight) {
        setColor(240);
        printf(" > %-*.*s", width - 3, width - 3, text); // -3 vì có " > "
        setColor(7);
    } else {
        printf("   %-*.*s", width - 3, width - 3, text);
    }
}

void boxPrint(const UILayout& L, short dx, short dy, const char* fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    gotoXY(L.boxX + dx, L.boxY + dy);
    std::cout << buf;
}


void clearInsideBox(short x, short y, short w, short h) {
    // x,y,w,h là kích thước cái box; phần trong là (x+1..x+w-2)
    for (short row = 1; row < h - 1; row = row + 1) {
        gotoXY((short)(x + 1), (short)(y + row));
        for (short col = 1; col < w - 1; col = col + 1) std::cout << ' ';
    }
}

void drawBox(short x, short y, short w, short h) {
    char tl = 218, tr = 191, bl = 192, br = 217;
    char hz = 196, vt = 179;

    gotoXY(x, y); std::cout << tl;
    for (int i = 0; i < w - 2; i++) std::cout << hz;
    std::cout << tr;

    for (int i = 1; i < h - 1; i++) {
        gotoXY(x, y + i); std::cout << vt;
        gotoXY(x + w - 1, y + i); std::cout << vt;
    }

    gotoXY(x, y + h - 1); std::cout << bl;
    for (int i = 0; i < w - 2; i++) std::cout << hz;
    std::cout << br;
}

void printCenter(short y, const char* text, short width) {
    short x = (width - (short)strlen(text)) / 2;
    if (x < 0) x = 0;
    gotoXY(x, y);
    std::cout << text;
}

void clearRect(short x, short y, short w, short h) {
    for (short row = 0; row < h; row = row + 1) {
        gotoXY(x, (short)(y + row));
        for (short col = 0; col < w; col = col + 1) {
            std::cout << ' ';
        }
    }
}


int readKey() {
    int c = _getch();
    if (c == 0 || c == 224) {
        int c2 = _getch();
        return 1000 + c2; // Arrow keys...
    }
    return c;
}
