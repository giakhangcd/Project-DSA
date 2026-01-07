#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <conio.h>
#include "utils.h"
#include "ui.h"
#include <fstream>
#include "thitracnghiem.h"
#include <iostream>
#include <limits>
#include <iomanip>
#include <cstring>    
#include <cstdio>   
#include <cstdlib>   
#include <ctime>     
#include <chrono>
#include <thread>
#include <sstream>
#include <vector>
#include <cctype>

using namespace std;

static UILayout gExamLayout;
static bool gExamLayoutReady = false;
static bool gDaVeKhungThi = false;  
static UILayout gLayoutThi;          
static void capNhatDongHoBenPhai(const UILayout& L, int conLai);


static void veFooterGrid(const UILayout& L, int soCau, FooterGrid& G) {
    if(soCau >10)
        G.cols = 10;
        else{
            G.cols = (short)soCau;
        }
    G.cellW = 8;
    G.x     = (short)(L.boxX + 10);
    G.y     = (short)(L.boxY + L.boxH +2);

    short W = (short)(G.cols * G.cellW + 2);
    short H = 4;

    drawBox(G.x, G.y, W, H);
    clearRect(G.x+1, G.y+1, W-2, H-2);

    // line ngang
    gotoXY((short)(G.x + 1), (short)(G.y + 2));
    for (int i = 0; i < W - 2; i = i + 1) std::cout << (char)196;

    // line dọc
    for (int c = 1; c < G.cols; c = c + 1) {
        short vx = (short)(G.x + 1 + c * G.cellW);
        gotoXY(vx, (short)(G.y + 1)); std::cout << (char)179;
        gotoXY(vx, (short)(G.y + 2)); std::cout << (char)197;
        gotoXY(vx, (short)(G.y + 3)); std::cout << (char)179;
    }

    // số câu
    for (int i = 0; i < G.cols; i = i + 1) {
        short cellLeft = (short)(G.x + 2 + i * G.cellW);
        gotoXY((short)(cellLeft+3), (short)(G.y + 1));
        std::cout << (i + 1);
    }
}

static void capNhatFooterCell(const FooterGrid& G, int idx, char daChon, bool isCursor)
{
    if (idx < 0) return;
    if (idx >= G.cols) return;

    // ===== clear vùng trong ô (tránh dính ký tự cũ) =====
    short cellLeft = (short)(G.x + 1 + idx * G.cellW);

    // clear hàng trên
    gotoXY((short)(cellLeft + 1), (short)(G.y + 1));
    std::cout << "     ";

    // clear hàng dưới
    gotoXY((short)(cellLeft + 1), (short)(G.y + 3));
    std::cout << "     ";

    // ===== hàng trên: dấu '>' =====
    gotoXY((short)(cellLeft + 2), (short)(G.y + 1));
    std::cout << (idx + 1);

    // ===== hàng dưới: đáp án =====
    gotoXY((short)(cellLeft + 2), (short)(G.y + 3));
    if (daChon == 0)
        std::cout << ' ';
    else
        std::cout << daChon;
}


static FooterGrid gFooter;
static bool gFooterInited = false;



// ====== TAO NODE ======


PTRDIEM taoNodeDiem(const DiemThi& x) {
    PTRDIEM p = new nodeDiem;
    p->dt = x;
    p->next = NULL;
    return p;
}

void themCuoiDiem(DS_DiemThi& ds, const DiemThi& x) {
    PTRDIEM n = taoNodeDiem(x);
    if (ds.pHead == NULL) {
        ds.pHead = n;
        ds.pLast = n;
    } else {
        ds.pLast->next = n;
        ds.pLast = n;
    }
    ds.SLDiem = ds.SLDiem + 1;
}


int demLanThiTrongFile(const char* masv, const char* mamh) {
    std::ifstream f("data/baithi_chitiet.txt");
    if (!f.is_open()) return 0;

    std::string line;
    std::string sMasv = masv;
    std::string sMamh = mamh;

    int maxLan = 0;
    

    while (std::getline(f, line)) {
        if (line.empty()) continue;

        // Format mới sẽ là: MASV|MAMH|LAN|ID|DADUNG|DASV
        // Nhưng để tránh crash khi file còn dữ liệu cũ, ta parse mềm:
        std::stringstream ss(line);
        std::string fMasv, fMamh, fLan;

        if (!std::getline(ss, fMasv, '|')) continue;
        if (!std::getline(ss, fMamh, '|')) continue;
        if (!std::getline(ss, fLan,  '|')) continue;

        if (fMasv != sMasv || fMamh != sMamh) continue;

        // nếu fLan không phải số (dữ liệu cũ) thì bỏ qua
        bool ok = true;
        for (char c : fLan) if (!isdigit((unsigned char)c)) { ok = false; break; }
        if (!ok) continue;

        int lan = std::atoi(fLan.c_str());
        if (lan > maxLan) maxLan = lan;
    }
    return maxLan;
}



// ======= THI =======
bool svDaThiMon(const SinhVien& sv, const char* mamh){
    // 1) check trong RAM (nhanh)
    for (PTRDIEM d = sv.dsDiem.pHead; d; d = d->next) {
        if (strcmp(d->dt.MAMH, mamh) == 0) return true;
    }

    // 2) fallback: check trong file baithi_chitiet.txt
    std::ifstream f("data/baithi_chitiet.txt");
    if (!f.is_open()) return false;

    std::string line;
    std::string sMasv = sv.MASV;
    std::string sMamh = mamh;

    while (std::getline(f, line)) {
        if (line.empty()) continue;

        size_t p1 = line.find('|');
        if (p1 == std::string::npos) continue;
        size_t p2 = line.find('|', p1 + 1);
        if (p2 == std::string::npos) continue;

        std::string fileMasv = line.substr(0, p1);
        std::string fileMamh = line.substr(p1 + 1, p2 - p1 - 1);

        if (fileMasv == sMasv && fileMamh == sMamh) return true;
    }
    return false;
}


bool luuDiem(SinhVien &sv, const char* mamh, float diem){
    

    DiemThi x;
    strcpy(x.MAMH, mamh);
    x.diem = diem;

    themCuoiDiem(sv.dsDiem, x);
    return true;
}


static void hoanVi(nodeCH** a, nodeCH** b){
    nodeCH* t = *a; *a = *b; *b = t;
}
// ===== Helpers chống nháy =====
static HANDLE HOUT = GetStdHandle(STD_OUTPUT_HANDLE);

// Forward declaration


void setColor(WORD attr){
    SetConsoleTextAttribute(HOUT, attr);
}

static void capNhatDongHoBenPhai(const UILayout& L, int conLai) {
    int phut = conLai / 60;
    int giay = conLai % 60;

    char buf[64];
    std::snprintf(buf, sizeof(buf), "Thoi gian con lai: %02d:%02d", phut, giay);

    int innerW = L.boxW - 2;   // vùng trong khung (không tính viền)
    int yLine  = 1;            // đúng dòng header anh đang in

    int xRight = innerW - (int)std::strlen(buf);
    if (xRight < 1) {
        xRight = 1;
    }

    // Tọa độ tuyệt đối trên console
    short absX = (short)(L.boxX + xRight);
    short absY = (short)(L.boxY + yLine);

    // In đè + xoá phần dư để khỏi dính ký tự cũ
    gotoXY(absX, absY);
    std::cout << buf;

    // Xóa phần còn lại bên phải (nếu trước đó dài hơn)
    int printed = (int)std::strlen(buf);
    int remain  = innerW - xRight - printed;
    if (remain < 0) {
        remain = 0;
    }

    for (int i = 0; i < remain; i = i + 1) {
        std::cout << ' ';
    }

    std::cout.flush();
}

// In 1 dòng option ở vị trí y; nếu highlight thì tô màu & có mũi tên
void drawOptionLine(const char* text, int y, bool highlight){
    gotoXY(2, (short)y);
    if (highlight){
        setColor(240);         // nền sáng, chữ tối (anti-flicker, dễ nhìn)
        printf(" > %-60s", text);
        setColor(7);
    } else {
        printf("   %-60s", text);
    }
}

int menuArrowGV() {
    static int current = 0;
    static const char* options[] = {
        "1. Nhap lop",
        "2. In danh sach lop",
        "3. Nhap SV vao lop",
        "4. Nhap/Cap nhat mon hoc",
        "5. Them cau hoi cho mon",
        "6. Sua cau hoi theo ID",
        "7. Xoa cau hoi theo ID",
        "8. In cau hoi mon",
        "9. Bao cao tinh trang thi theo lop (theo mon)",
        "10. Nhap lich thi ",
        "0. Dang xuat"
    };
    const int total = 11;

    hideCursor(true);
    setColor(7);
    UILayout L;
    beginScreenBox("=== MENU GIANG VIEN ===", 90, 18, L);

    short baseX = L.boxX + 3;
    short baseY = L.boxY + 3;
    int   lineW = L.boxW - 6;


    // In menu lần đầu
    for (int i = 0; i < total; i++) {
        drawOptionLineAt(baseX, baseY +i,options[i], (i == current),lineW);
    }

    while (true) {
        int key = _getch();
        if (key == 0 || key == 224) {
            key = _getch();
        }

        if (key == 72) { // Mũi tên Lên
            drawOptionLineAt(baseX,baseY+current, options[current],false,lineW);
            current = current - 1;
            if (current < 0) {
                current = total - 1;
            }
            drawOptionLineAt(baseX, baseY+current, options[current], true,lineW);
        }
        else if (key == 80) { // Mũi tên Xuống
            drawOptionLineAt(baseX, baseY + current,options[current],false,lineW);
            current = current + 1;
            if (current >= total) {
                current = 0;
            }
            drawOptionLineAt(baseX, baseY+current ,options[current], true,lineW);
        }
        else if (key == 13) { // Enter
            hideCursor(false);
            setColor(7);
            if (current == total - 1) return 0;
            return current + 1;
        }
            else if(key == KEY_ESC){
                hideCursor(false);
                setColor(7);
                clearConsole();
                return -1;
            }
        }
    }

    int menuArrowSV(const SinhVien* sv) {
    static int current = 0;
    static const char* options[] = {
        "1. Thi trac nghiem",
        "0. Dang xuat"
    };
    const int total = 2;

    hideCursor(true);
    setColor(7);
    clearConsole();

    gotoXY(0, 0);
    cout << "=== MENU SINH VIEN (" << sv->MASV << ") ===\n\n";

    int baseY = 2;

    // In lần đầu
    for (int i = 0; i < total; i = i + 1) {
        drawOptionLine(options[i], baseY + i, (i == current));
    }

    while (true) {
        int key = _getch();
        if (key == 0 || key == 224) {
            int k2 = _getch();
            if (k2 == 72) { // LEN
                drawOptionLine(options[current], baseY + current, false);
                current = current - 1;
                if (current < 0) {
                    current = total - 1;
                }
                drawOptionLine(options[current], baseY + current, true);
            } else if (k2 == 80) { // XUONG
                drawOptionLine(options[current], baseY + current, false);
                current = current + 1;
                if (current >= total) {
                    current = 0;
                }
                drawOptionLine(options[current], baseY + current, true);
            }
        }
         else if (key == 13) { // ENTER
            hideCursor(false);
            setColor(7);
            // current = 0 -> thoát, current = 1 -> thi
            if (current == 0) {
                return 1;      // Dang xuat
            } else {
                return 0;      // Thi trac nghiem
            }
        } else if (key == KEY_ESC) {
            hideCursor(false);
            setColor(7);
            return 0;          // ESC cũng coi như đăng xuất
        }
    }
}


bool readLineEsc(const char* prompt, char* out, size_t maxLen) {
    cout << prompt;
    size_t len = 0;
    while(true){
        int c = _getch();
        if(c == KEY_ESC){
            cout << "\n(Huy bo - ESC)\n";
            return false;
        }
        if(c == '\r'){ // Enter
            cout << "\n";
            out[len] = '\0';
            return true;
        }
        if(c == '\b'){ // Backspace
            if(len > 0){ len--; cout << "\b \b"; }
            continue;
        }
        // In ký tự thường (tránh control)
        if(c >= 32 && c < 127){
            if(len + 1 < maxLen){
                out[len++] = (char)c;
                cout << (char)c;
            }
        }
    }
}

bool readIntEsc(const char* prompt, int& out){
    char buf[32];
    if(!readLineEsc(prompt, buf, sizeof(buf))) return false;
    out = atoi(buf);
    return true;
}




char docDapAnCoTimer(int tongGiay, std::chrono::steady_clock::time_point batDau) {
    int conLaiCu = -1;

    while (true) {
        auto now   = std::chrono::steady_clock::now();
        auto daQua = std::chrono::duration_cast<std::chrono::seconds>(now - batDau).count();
        int conLai = tongGiay - (int)daQua;
        if (conLai < 0) conLai = 0;

        if (conLai == 0) {
            gotoXY(0, 2);
            std::cout << "\nDa het thoi gian lam bai.\n";
            return '?';
        }

        if (conLai != conLaiCu) {
            conLaiCu = conLai;

            if (gExamLayoutReady) {
                capNhatDongHoBenPhai(gExamLayout, conLai);
            }

            int m = conLai / 60;
            int s = conLai % 60;

            gotoXY(59, 0);
            if (m < 10) std::cout << "0";
            std::cout << m << ":";
            if (s < 10) std::cout << "0";
            std::cout << s << "   ";
        }

        if (_kbhit()) {
            int c = _getch();
            if (c == KEY_ESC) return KEY_ESC;

            c = (int)std::toupper((unsigned char)c);
            if (c >= 'A' && c <= 'D') {
                std::cout << (char)c << "\n";
                return (char)c;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}


bool xacNhanNopBai(
    int tongGiay,
    std::chrono::steady_clock::time_point batDau
){
    int conLaiCu = -1;
    int choice   = 0; // 0 = YES, 1 = NO

    const short dialogX = 50;
    const short dialogY = 15;

    gotoXY(dialogX, dialogY);
    std::cout << "Ban da tra loi xong tat ca cau hoi.\n";
    gotoXY(dialogX, dialogY + 1);
    std::cout << "Ban co chac chan muon nop bai khong?\n";

    short optY = dialogY + 3;

    auto drawOptions = [&](int ch){
        gotoXY(dialogX, optY);
        if (ch == 0) {
            std::cout << "[YES]    no   ";
        } else {
            std::cout << " yes    [NO]  ";
        }
        std::cout.flush();
    };

    drawOptions(choice);

    gotoXY(dialogX, optY + 2);
    std::cout << "Su dung phim TRAI/PHAI hoac A/D de chon, Enter de xac nhan.";
    gotoXY(dialogX, optY + 3);
    std::cout << "- ESC: huy bai thi, khong tinh diem";

    while (true) {
        auto now   = std::chrono::steady_clock::now();
        auto daQua = std::chrono::duration_cast<std::chrono::seconds>(now - batDau).count();
        int conLai = tongGiay - (int)daQua;
        if (conLai < 0) {
            conLai = 0;
        }

        if (conLai == 0) {
            // Hết giờ trong lúc confirm -> tự động nộp
            clearRect(dialogX, dialogY, 80, 7);
            return true;
        }

        if (conLai != conLaiCu) {
            conLaiCu = conLai;
            int m = conLai / 60;
            int s = conLai % 60;

            gotoXY(59, 0);
            if (m < 10) std::cout << "0";
            std::cout << m << ":";
            if (s < 10) std::cout << "0";
            std::cout << s << "   ";
        }

        if (_kbhit()) {
            int c = _getch();

            if (c == KEY_ESC) {
                // KHÔNG nộp -> xoá popup rồi quay lại màn thi
                clearRect(dialogX, dialogY, 80, 7);
                return false;
            }

            if (c == 0 || c == 224) {
                int c2 = _getch();
                if (c2 == 75 || c2 == 77) { // LEFT / RIGHT
                    if (choice == 0) {
                        choice = 1;
                    } else {
                        choice = 0;
                    }
                    drawOptions(choice);
                }
            } else {
                if (c == 'a' || c == 'A' || c == 'd' || c == 'D') {
                    if (choice == 0) {
                        choice = 1;
                    } else {
                        choice = 0;
                    }
                    drawOptions(choice);
                } else if (c == 13) { // Enter
                    if (choice == 0) {
                        // YES
                        clearRect(dialogX, dialogY, 80, 7);
                        return true;
                    } else {
                        // NO
                        clearRect(dialogX, dialogY, 80, 7);
                        return false;
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}


// Vẽ 1 câu hỏi trong màn hình thi (dùng cho UI mũi tên)
static void veMotCauHoiThi(
    const CauHoi &ch,
    int index,          // chỉ số câu 0..soCau-1
    char daChon,        // 'A'..'D' hoặc 0 nếu chưa chọn
    bool highlight      // true nếu con trỏ đang đứng ở câu này
){
     
    if (highlight) {
        setColor(240);
    }
    cout << "Cau " << (index + 1) << " (ID " << ch.ID << ")\n" << ch.NoiDung << "\n";
    if(highlight){
        setColor(7);
    }

    char label[4] = { 'A', 'B', 'C', 'D' };
    const char* nd[4] = { ch.A, ch.B, ch.C, ch.D };

    for (int i = 0; i < 4; ++i) {
        char L = label[i];

        cout << "   ";

        // Hiển thị đáp án đang CHỌN của câu đó
        if (daChon == L) {
            cout << "[" << L << "] ";
        } else {
            cout << " " << L << ". ";
        }

        cout << nd[i] << "\n";
    }

    cout << "\n";
}

// Vẽ cố định label "Thoi gian: X phut  Con lai: "
static void veDongThoiGian(int soPhut) {
    gotoXY(0, 2);
    cout << "Thoi gian: " << soPhut << " phut  Con lai: ";
}

// Chỉ cập nhật phần mm:ss, không đụng tới cả dòng
static void capNhatDongHoThi(int conLai) {
    int m = conLai / 60;
    int s = conLai % 60;

    // Sau chữ "Con lai: " ta nhảy ra 1 cột cố định, ví dụ x = 32
    gotoXY(32, 2);

    if (m < 10) cout << "0";
    cout << m << ":";

    if (s < 10) cout << "0";
    cout << s << "   ";
}

static void drawAnswerBox(
    short x, short y,
    short boxW, short boxH,
    const char* label,
    const char* text,
    bool highlight,
    bool chosen
) {
    // In label (bên trái)
    gotoXY(x, y);
    if (highlight) setColor(240);
    else setColor(7);
    std::cout << label;
    setColor(7);

    // Vẽ box nội dung (bên phải label)
    short bx = (short)(x + (short)strlen(label) + 2);
    short by = y;
    drawBox(bx, by, boxW, boxH);

    // In text trong box (1 dòng)
    gotoXY((short)(bx + 2), (short)(by + 1));

    if (chosen) {
        // nếu muốn đánh dấu câu đã chọn rõ hơn thì mở màu này
        // setColor(11);
    }

    int maxLen = boxW - 4;
    for (int i = 0; text[i] != '\0' && i < maxLen; i++) std::cout << text[i];

    setColor(7);
}

static void veMotDapAn(
    const UILayout& L,
    const CauHoi& ch,
    int idxAns,     // 0..3
    bool highlight,
    bool chosen
){
    short startX = (short)(L.boxX + 2);
    short startY = (short)(L.boxY + 8);
    short boxW   = (short)(L.boxW - 20);
    short boxH   = 3;

    const char* labels[4] = {
        "DAP AN A:", "DAP AN B:", "DAP AN C:", "DAP AN D:"
    };
    const char* texts[4] = {
        ch.A, ch.B, ch.C, ch.D
    };

    drawAnswerBox(
        startX,
        (short)(startY + idxAns * (boxH + 1)),
        boxW,
        boxH,
        labels[idxAns],
        texts[idxAns],
        highlight,
        chosen
    );
}

static void veCauHienTai(
    const UILayout& L,
    const CauHoi dsCau[],
    const char dsDaChon[],
    int soCau,
    int cursor,
    int curAns
){
    // Nếu đang ở dòng NOP BAI thì tuỳ em:
    // ở đây ta vẫn vẽ nội dung "NOP BAI" mà không vẽ full màn
    short contentX = (short)(L.boxX + 2);
    short contentY = (short)(L.boxY + 5);
    short contentW = (short)(L.boxW - 2);
    short contentH = (short)(L.boxH - 5);
    clearRect(contentX, contentY, contentW, contentH);

    boxPrint(L, 3, 3, "Huong dan:  Left/Right doi cau | UP/DOWN chon A/B/C/D | ENTER chon | ESC huy");

    if (cursor == soCau) {
        boxPrint(L, 3, 6, "[ NOP BAI ]");
        return;
    }

    int i = cursor;
    boxPrint(L, 3, 5, "Cau %d/%d (ID %d):", i + 1, soCau, dsCau[i].ID);
    boxPrint(L, 3, 6, "%s", dsCau[i].NoiDung);

    // vẽ 4 đáp án
    char chosen = dsDaChon[i];

    for (int a = 0; a < 4; ++a) {
        bool hl = (a == curAns);
        bool ch = (chosen == "ABCD"[a]);
        veMotDapAn(L, dsCau[i], a, hl, ch);
    }
}

static void veNoiDungCau(
    const UILayout& L,
    const CauHoi& ch,
    char daChon,
    int curAns,
    int idx,
    int soCau
){
    // Xóa vùng nội dung câu
    short contentX = (short)(L.boxX + 2);
    short contentY = (short)(L.boxY + 5);
    short contentW = (short)(L.boxW - 2);
    short contentH = (short)(L.boxH - 5);
    clearRect(contentX, contentY, contentW, contentH);

    boxPrint(L, 3, 5, "Cau %d/%d (ID %d):", idx + 1, soCau, ch.ID);
    boxPrint(L, 3, 6, "%s", ch.NoiDung);

    for (int a = 0; a < 4; a++) {
        bool hl = (a == curAns);
        bool chon = (daChon == "ABCD"[a]);
        veMotDapAn(L, ch, a, hl, chon);
    }
}

static void veNoiDungCau_KhongClear(
    const UILayout& L,
    const CauHoi& ch,
    char daChon,
    int curAns,
    int idx,
    int soCau
){
    // ----- Tiêu đề câu (ghi đè)
    gotoXY(L.boxX + 3, L.boxY + 5);
    printf("Cau %02d/%02d (ID %d)%-40s",
           idx + 1, soCau, ch.ID, " ");

    // ----- Nội dung câu (ghi đè 1–2 dòng)
    gotoXY(L.boxX + 3, L.boxY + 6);
    printf("%-90s", ch.NoiDung);

    gotoXY(L.boxX + 3, L.boxY + 7);
    printf("%-90s", " ");

    // ----- 4 đáp án
    for (int a = 0; a < 4; a++) {
        bool hl   = (a == curAns);
        bool chon = (daChon == "ABCD"[a]);
        veMotDapAn(L, ch, a, hl, chon);
    }
}


// Vẽ toàn bộ màn hình thi (tất cả câu + dòng NOP BAI + thời gian)
static void veManHinhThi(
    const CauHoi dsCau[],
    const char dsDaChon[],
    int soCau,
    int cursor, // 0..soCau (soCau = dòng NOP BAI)
    int curAns,
    int conLai,
    int tongGiay,
    int soPhut)
{
    // 1) Layout khung chính ở giữa
    static UILayout gLayoutThi;

    if (!gDaVeKhungThi) {
    beginScreenBox("DE THI TRAC NGHIEM", 100, 25, gLayoutThi);

    gExamLayout = gLayoutThi;
    gExamLayoutReady = true;

    veFooterGrid(gLayoutThi, soCau, gFooter);
    for (int i = 0; i < gFooter.cols; i++) {
        capNhatFooterCell(gFooter, i, dsDaChon[i], (i == cursor));
    }

    capNhatDongHoBenPhai(gExamLayout, conLai);

    gDaVeKhungThi = true;
    gFooterInited = true;
}


    // Xoá vùng nội dung câu hỏi + đáp án bên trong khung chính
    short contentX = (short)(gLayoutThi.boxX + 2);
    short contentY = (short)(gLayoutThi.boxY + 5);
    short contentW = (short)(gLayoutThi.boxW - 2);
    short contentH = (short)(gLayoutThi.boxH - 5 );
    clearRect(contentX, contentY, contentW, contentH);


    short sideW = 20;
    short sideH = gLayoutThi.boxH;
    short sideX = gLayoutThi.boxX + gLayoutThi.boxW + 2;
    short sideY = gLayoutThi.boxY;

    short CW, CH;
    getConsoleWindowSize(CW, CH);
    if (sideX + sideW >= CW) {
        sideX = gLayoutThi.boxX - sideW - 2; // nếu không đủ chỗ thì đặt bên trái
    }
    gotoXY(sideX + 2, sideY + 1);
     


    // 2) Panel phụ bên phải: chỉ hiện "Cau i: A/B/."
    
    // 3) Vẽ header trong khung chính

    boxPrint(gLayoutThi, 3, 3, "Huong dan:  Left/Right do    i cau | UP/DOWN chon A/B/C/D | ENTER chon | ESC huy");

    // 4) Nếu đang ở dòng NOP BAI
    if (cursor == soCau) {
        boxPrint(gLayoutThi, 3, 6, "[ NOP BAI ]");
    } else {
        // 5) Vẽ CHỈ 1 câu hiện tại
        int i = cursor;
        boxPrint(gLayoutThi, 3, 5, "Cau %d/%d (ID %d):", i + 1, soCau, dsCau[i].ID);
        boxPrint(gLayoutThi, 3, 6, "%s", dsCau[i].NoiDung);

        // vẽ 4 đáp án (anh có thể tận dụng drawAnswerLine)
        int baseY = gLayoutThi.boxY + 9;

        bool daA = (dsDaChon[i] == 'A');
        bool daB = (dsDaChon[i] == 'B');
        bool daC = (dsDaChon[i] == 'C');
        bool daD = (dsDaChon[i] == 'D');

        // curOpt phải lấy từ biến điều khiển bên ngoài (đáp án đang trỏ)
        // tạm thời nếu anh chưa có curOpt thì cứ highlight = false hết
        // vị trí theo khung
        short startX = (short)(gLayoutThi.boxX + 2);
        short startY = (short)(gLayoutThi.boxY + 8);

// boxW là độ rộng phần chữ trong ô (không tính label)
        short boxW = (short)(gLayoutThi.boxW - 20);
        short boxH = 3;

// highlight theo đáp án đang trỏ (nếu anh chưa có curOpt thì để false hết như hiện tại)
        bool hlA = false, hlB = false, hlC = false, hlD = false;
        
        if (curAns == 0) hlA = true;
        else if (curAns == 1) hlB = true;
        else if (curAns == 2) hlC = true;
        else if (curAns == 3) hlD = true;

// mỗi ô cách nhau 1 dòng (boxH=3 => y tăng 4)
        drawAnswerBox(startX, (short)(startY + 0 * (boxH + 1)), boxW, boxH, "DAP AN A:", dsCau[i].A, hlA, daA);
        drawAnswerBox(startX, (short)(startY + 1 * (boxH + 1)), boxW, boxH, "DAP AN B:", dsCau[i].B, hlB, daB);
        drawAnswerBox(startX, (short)(startY + 2 * (boxH + 1)), boxW, boxH, "DAP AN C:", dsCau[i].C, hlC, daC);
        drawAnswerBox(startX, (short)(startY + 3 * (boxH + 1)), boxW, boxH, "DAP AN D:", dsCau[i].D, hlD, daD);

    }
}

// Xoá toàn bộ dòng bai thi cu cua 1 SV + mon trong file baithi_chitiet.txt
void xoaBaiThiCuCuaSV(const char* masv, const char* mamh) {
    std::ifstream in("data/baithi_chitiet.txt");
    if (!in.is_open()) {
        return; // chưa có file thì thôi
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string fMasv, fMamh;

        if (!std::getline(ss, fMasv, '|')) continue;
        if (!std::getline(ss, fMamh, '|')) continue;

        // Nếu trùng MASV + MAMH thì bỏ, không lưu lại
        if (fMasv == masv && fMamh == mamh) {
            continue;
        }

        lines.push_back(line);
    }

    in.close();

    std::ofstream out("data/baithi_chitiet.txt", std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return;
    }

    for (const std::string &l : lines) {
        out << l << '\n';
    }

    out.close();
}


// Lưu chi tiết bài thi của một sinh viên vào file (mỗi dòng 1 câu)
void luuBaiThiChiTiet(
    const char* masv,
    const char* mamh,
    int lanThi,
    const CauHoi dsCau[],   // mảng câu hỏi đã thi
    const char dsDaChon[],  // mảng đáp án sinh viên chọn
    int soCau
){  
    // Nên để trong thư mục data cho gọn
    std::ofstream f("data/baithi_chitiet.txt", std::ios::app);
    if (!f.is_open()) {
        std::cout << "Khong mo duoc file data/baithi_chitiet.txt\n";
        return;
    }

    if(lanThi > 1){
        f << '\n';
    }

    for (int i = 0; i < soCau; i = i + 1) {
        char daSv = dsDaChon[i];
        if (daSv == 0) {
            daSv = '-'; // chưa trả lời
        }

        f << masv                << '|'
          << mamh                << '|'
          << lanThi              << '|'
          << dsCau[i].ID         << '|'
          << dsCau[i].DapAn      << '|'
          << daSv                << '\n';
    }

    f.close();
}

static void xoaFooterGrid(const FooterGrid& G) {
    // footer cao ~ 3 dòng (dòng số + dòng đáp án + viền)
    // rộng = cols * cellW (+ dư 2 cho viền)
    clearRect(G.x, G.y, (short)(G.cols * G.cellW + 2), 3);
}


   float thiTracNghiem(MonHoc &mh, int soCau, int soPhut, const char* masv, const char* mamh,int lanThi){
       clearConsole();
       int tongGiay = soPhut * 60;
       auto batDau = std::chrono::steady_clock::now();

       int n = demCauHoi(mh.dsCH.pHead);
       if (n == 0)
       {
           cout << "Mon chua co cau hoi.\n";
           return -1.0f;
       }
    if (soCau > n) {
        soCau = n;
    }

    // Gom nodeCH* vao mang
    nodeCH** arrNode = new nodeCH*[n];
    nodeCH* p = mh.dsCH.pHead;
    int idx = 0;
    while (p != NULL) {
        arrNode[idx] = p;
        idx = idx + 1;
        p = p->next;
    }

    // Fisher–Yates shuffle
    seedRandOnce();
    for (int k = n - 1; k > 0; k--) {
        int j = std::rand() % (k + 1);
        hoanVi(&arrNode[k], &arrNode[j]);
    }

    // Copy soCau dau tien vao mang CauHoi de lam de thi
    CauHoi* dsCauThi = new CauHoi[soCau];
    for (int i = 0; i < soCau; ++i) {
        dsCauThi[i] = arrNode[i]->ch;
    }

    // Mang luu dap an da chon
    char* dsDaChon = new char[soCau];
    for (int i = 0; i < soCau; ++i) {
        dsDaChon[i] = 0; // 0 = chua chon
    }

    int cursor    = 0;   // 0..soCau (soCau = dong [NOP BAI])
    int conLaiCu  = -1;
    int curAns = 0; 
    bool ketThuc  = false;
    bool daNop    = false;  // true khi da xac nhan nop bai
    bool huyBai   = false;  // true khi ESC
    gDaVeKhungThi = false;
    gFooterInited = false;
    gExamLayoutReady = false;

    // xoaFooterGrid(gFooter);

    int conLaiInit = tongGiay;
    conLaiCu       = conLaiInit;
    veManHinhThi(dsCauThi, dsDaChon, soCau, cursor,curAns, conLaiInit, tongGiay, soPhut);

    while (!ketThuc) {
        auto now   = std::chrono::steady_clock::now();
        auto daQua = std::chrono::duration_cast<std::chrono::seconds>(now - batDau).count();
        int conLai = tongGiay - (int)daQua;
        if (conLai < 0) {
            conLai = 0;
        }

        // Het gio -> tu dong nop
        if (conLai == 0) {
            ketThuc = true;
            daNop   = true;
            break;
        }

        // Neu doi giay -> cap nhat dong ho
        if (conLai != conLaiCu) {
            conLaiCu = conLai;
            //capNhatDongHoThi(conLai);
            if(gExamLayoutReady){
                capNhatDongHoBenPhai(gExamLayout, conLai);
            }
        }

        // Neu co phim nhap vao
        if (_kbhit()) {
            int c = _getch();

            // ESC: huy bai thi
            if (c == KEY_ESC) {
                huyBai  = true;
                ketThuc = true;
                break;
            }

            // Phim mo rong (mui ten)
           if (c == 0 || c == 224) {
    int c2 = _getch();

        if (c2 == 72) { // UP
            if (cursor == soCau) continue;
            int old = curAns;
            curAns = (curAns + 3) % 4;
            veMotDapAn(gExamLayout, dsCauThi[cursor],old, false, dsDaChon[cursor] == "ABCD"[old]);
            veMotDapAn(gExamLayout, dsCauThi[cursor],curAns, true, dsDaChon[cursor] == "ABCD"[curAns]);
}
    else if (c2 == 80) { // DOWN
        if (cursor == soCau) continue;
        int old = curAns;
        curAns = (curAns + 1) % 4;
        veMotDapAn(gExamLayout, dsCauThi[cursor],old, false, dsDaChon[cursor] == "ABCD"[old]);
        veMotDapAn(gExamLayout, dsCauThi[cursor],curAns, true, dsDaChon[cursor] == "ABCD"[curAns]);
}

    else if (c2 == 75) { // LEFT
    int oldCursor = cursor;

    // 1) đổi cursor
    cursor--;
    if (cursor < 0) cursor = soCau; // vòng về NOP BAI

    // 2) set curAns theo đáp án đã chọn của câu mới
    if (cursor < soCau) {
        if (dsDaChon[cursor] == 'A') curAns = 0;
        else if (dsDaChon[cursor] == 'B') curAns = 1;
        else if (dsDaChon[cursor] == 'C') curAns = 2;
        else if (dsDaChon[cursor] == 'D') curAns = 3;
        else curAns = 0;
    } else {
        curAns = 0; // đang ở NOP BAI
    }

    // 3) cập nhật footer highlight
    if (oldCursor < soCau) capNhatFooterCell(gFooter, oldCursor, dsDaChon[oldCursor], false);
    if (cursor   < soCau) capNhatFooterCell(gFooter, cursor,   dsDaChon[cursor],   true);

    // 4) quan trọng nhất: vẽ lại nội dung theo cursor mới
    veCauHienTai(gExamLayout, dsCauThi, dsDaChon, soCau, cursor, curAns);
}



    else if (c2 == 77) { // RIGHT
    int oldCursor = cursor;

    // 1) đổi cursor
    if (cursor == soCau - 1) cursor = soCau;      // từ câu cuối -> NOP BAI
    else if (cursor < soCau - 1) cursor++;        // câu bình thường -> câu tiếp
    else if (cursor == soCau) cursor = 0;         // từ NOP BAI -> về câu 1 (tuỳ bạn)

    // 2) set curAns theo đáp án đã chọn của câu mới
    if (cursor < soCau) {
        if (dsDaChon[cursor] == 'A') curAns = 0;
        else if (dsDaChon[cursor] == 'B') curAns = 1;
        else if (dsDaChon[cursor] == 'C') curAns = 2;
        else if (dsDaChon[cursor] == 'D') curAns = 3;
        else curAns = 0;
    } else {
        curAns = 0; // đang ở NOP BAI
    }

    // 3) cập nhật footer highlight
    if (oldCursor < soCau) capNhatFooterCell(gFooter, oldCursor, dsDaChon[oldCursor], false);
    if (cursor   < soCau) capNhatFooterCell(gFooter, cursor,   dsDaChon[cursor],   true);

    // 4) quan trọng nhất: vẽ lại nội dung theo cursor mới
    veCauHienTai(gExamLayout, dsCauThi, dsDaChon, soCau, cursor, curAns);
}

}

else if (c == 13) { // ENTER
    if (cursor < soCau) {
        dsDaChon[cursor] = "ABCD"[curAns];
        capNhatFooterCell(gFooter, cursor, dsDaChon[cursor], true);
        for (int a = 0; a < 4;a++){
            bool hl = (a == curAns);
            bool ch = (dsDaChon[cursor] == "ABCD"[a]);
        }
    } else { // cursor == soCau  => NOP BAI
        bool ok = xacNhanNopBai(tongGiay, batDau);
        if (ok) {
            xoaFooterGrid(gFooter);
            daNop = true;
            ketThuc = true;
        } else {
    // quay lại câu cuối
    cursor = soCau - 1;

    if (dsDaChon[cursor] == 'A') curAns = 0;
    else if (dsDaChon[cursor] == 'B') curAns = 1;
    else if (dsDaChon[cursor] == 'C') curAns = 2;
    else if (dsDaChon[cursor] == 'D') curAns = 3;
    else curAns = 0;

    capNhatFooterCell(gFooter, cursor, dsDaChon[cursor], true);

    veNoiDungCau_KhongClear(
        gExamLayout,
        dsCauThi[cursor],
        dsDaChon[cursor],
        curAns,
        cursor,
        soCau
    );
}

    }
}

    }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    } // <- ĐÓNG while (!ketThuc)

    // ================= XỬ LÝ KẾT THÚC =================
    if (!daNop) {
        // ESC hoac khong nop bai
        delete[] arrNode;
        delete[] dsCauThi;
        delete[] dsDaChon;

        hideCursor(false);
        if (huyBai) {
            waitEsc("\nBan da huy bai thi. Nhan ESC de quay lai menu...");
        } else {
            waitEsc("\nBai thi khong duoc nop. Nhan ESC de quay lai menu...");
        }
        clearConsole();
        return -1.0f;
    }

    // =============== TÍNH ĐIỂM ===============
    int dung = 0;
    for (int i = 0; i < soCau; ++i)
    {
        if (dsDaChon[i] == dsCauThi[i].DapAn)
        {
            dung = dung + 1;
        }
}

float diem;
if (soCau == 0) {
    diem = 0.0f;
} else {
    diem = dung * 10.0f / soCau;
}
mh.trangThaiThi = 0;

// =============== LƯU CHI TIẾT BÀI THI ===============
luuBaiThiChiTiet(masv, mamh, lanThi, dsCauThi, dsDaChon, soCau);

// (Nếu muốn lưu file tổng hợp điểm theo ngày thì có thể gọi thêm
// luuKetQuaThi(*sv, mamh, (int)std::round(diem), soPhut); ở ngoài menuSV)

// =============== GIAO DIỆN CHO SINH VIÊN ===============
clearConsole();
hideCursor(false);

std::cout << "Ban da nop bai mon " << mamh << ".\n";
std::cout << "So cau dung: " << dung << "/" << soCau << "\n";

std::cout << std::fixed << std::setprecision(2);
std::cout << "Diem: " << diem << "\n";

waitEsc("\nNhan ESC de quay lai menu...");

// Dọn dẹp bộ nhớ
delete[] arrNode;
delete[] dsCauThi;
delete[] dsDaChon;

clearConsole();
return diem;
   }

void inDSSVTrongLop(const Lop* lop) {
    clearConsole();

    if (lop == NULL) {
        cout << "Khong xac dinh duoc lop.\n";
        waitEsc("\nNhan ESC de quay lai...");
        clearConsole();
        return;
    }

    cout << "=== DANH SACH SINH VIEN LOP "
         << lop->MALOP << " - " << lop->TENLOP << " ===\n\n";

    PTRSV p = lop->dsSV.pHead;
    int stt = 1;

    if (p == NULL) {
        cout << "Lop khong co sinh vien nao.\n";
    } else {
        cout << left << setw(5) << "STT"
             << setw(16) << "MASV"
             << setw(20) << "HO"
             << setw(12) << "TEN"
             << setw(8)  << "PHAI"
             << "\n";

        while (p != NULL) {
            cout << left << setw(5)  << stt
                 << setw(16) << p->sv.MASV
                 << setw(20) << p->sv.HO
                 << setw(12) << p->sv.TEN
                 << setw(8)  << p->sv.PHAI
                 << "\n";
            stt = stt + 1;
            p   = p->next;
        }
    }

    waitEsc("\nNhan ESC de quay lai danh sach lop...");
    clearConsole();
}

Lop* chonLopBangMuiTen(const DS_LOP &ds) {
    if (ds.n == 0) {
        clearConsole();
        std::cout << "Khong co lop nao trong he thong.\n";
        waitEsc("\nNhan ESC de quay lai...");
        clearConsole();
        return NULL;
    }

    hideCursor(true);
    setColor(7);
    clearConsole();   // ✅ CHỈ GỌI 1 LẦN

    UILayout L;
    beginScreenBox("=== CHON LOP ===", 80, 20, L);

    boxPrint(L, 3, 2, "Dung MUI TEN LEN/XUONG de chon lop");
    boxPrint(L, 3, 3, "ENTER de xac nhan, ESC de huy");

    int current = 0;
    int total   = ds.n;

    short baseX = L.boxX + 3;
    short baseY = L.boxY + 5;
    int   lineW = L.boxW - 6;

    // ===== IN DANH SÁCH LẦN ĐẦU =====
    for (int i = 0; i < total; i++) {
        char buf[80];
        std::snprintf(buf, sizeof(buf), "%s - %s",
                      ds.nodes[i]->MALOP,
                      ds.nodes[i]->TENLOP);
        drawOptionLineAt(baseX, baseY + i, buf, (i == current), lineW);
    }

    // ===== VÒNG LẶP PHÍM – KHÔNG NHÁY =====
    while (true) {
        int c = _getch();

        if (c == 0 || c == 224) {
            int k = _getch();

            if (k == 72) { // ↑ UP
                // xoá highlight cũ
                char bufOld[80];
                std::snprintf(bufOld, sizeof(bufOld), "%s - %s",
                              ds.nodes[current]->MALOP,
                              ds.nodes[current]->TENLOP);
                drawOptionLineAt(baseX, baseY + current, bufOld, false, lineW);

                current--;
                if (current < 0) current = total - 1;

                // tô highlight mới
                char bufNew[80];
                std::snprintf(bufNew, sizeof(bufNew), "%s - %s",
                              ds.nodes[current]->MALOP,
                              ds.nodes[current]->TENLOP);
                drawOptionLineAt(baseX, baseY + current, bufNew, true, lineW);
            }
            else if (k == 80) { // ↓ DOWN
                char bufOld[80];
                std::snprintf(bufOld, sizeof(bufOld), "%s - %s",
                              ds.nodes[current]->MALOP,
                              ds.nodes[current]->TENLOP);
                drawOptionLineAt(baseX, baseY + current, bufOld, false, lineW);

                current++;
                if (current >= total) current = 0;

                char bufNew[80];
                std::snprintf(bufNew, sizeof(bufNew), "%s - %s",
                              ds.nodes[current]->MALOP,
                              ds.nodes[current]->TENLOP);
                drawOptionLineAt(baseX, baseY + current, bufNew, true, lineW);
            }
        }
        else if (c == 13) { // ENTER
            hideCursor(false);
            setColor(7);
            clearConsole();
            return ds.nodes[current];   // ✅ LỚP ĐƯỢC CHỌN
        }
        else if (c == KEY_ESC) {
            hideCursor(false);
            setColor(7);
            clearConsole();
            return NULL;               // ❌ HỦY
        }
    }
}


void xemDSLopBangMuiTen(const DS_LOP &ds) {
    clearConsole();

    if (ds.n == 0) {
        cout << "Khong co lop nao trong he thong.\n";
        waitEsc("\nNhan ESC de quay lai menu...");
        clearConsole();
        return;
    }

    hideCursor(true);
    setColor(7);

    gotoXY(0, 0);
    cout << "=== DANH SACH LOP (" << ds.n << ") ===\n\n";
    cout << "Dung MUI TEN LEN/XUONG de chon lop,\n";
    cout << "ENTER de xem sinh vien, ESC de quay lai menu.\n\n";

    int baseY   = 4;
    int current = 0;

    // in lần đầu
    for (int i = 0; i < ds.n; i = i + 1) {
        char buf[80];
        std::snprintf(buf, sizeof(buf), "%s - %s",
                      ds.nodes[i]->MALOP, ds.nodes[i]->TENLOP);
        drawOptionLine(buf, baseY + i, (i == current));
    }

    while (true) {
        int key = _getch();

        if (key == 0 || key == 224) {
            int k2 = _getch();
            if (k2 == 72) { // LEN
                char bufOld[80];
                std::snprintf(bufOld, sizeof(bufOld), "%s - %s",
                              ds.nodes[current]->MALOP, ds.nodes[current]->TENLOP);
                drawOptionLine(bufOld, baseY + current, false);

                current = current - 1;
                if (current < 0) {
                    current = ds.n - 1;
                }

                char bufNew[80];
                std::snprintf(bufNew, sizeof(bufNew), "%s - %s",
                              ds.nodes[current]->MALOP, ds.nodes[current]->TENLOP);
                drawOptionLine(bufNew, baseY + current, true);
            } else if (k2 == 80) { // XUONG
                char bufOld[80];
                std::snprintf(bufOld, sizeof(bufOld), "%s - %s",
                              ds.nodes[current]->MALOP, ds.nodes[current]->TENLOP);
                drawOptionLine(bufOld, baseY + current, false);

                current = current + 1;
                if (current >= ds.n) {
                    current = 0;
                }

                char bufNew[80];
                std::snprintf(bufNew, sizeof(bufNew), "%s - %s",
                              ds.nodes[current]->MALOP, ds.nodes[current]->TENLOP);
                drawOptionLine(bufNew, baseY + current, true);
            }
        } else if (key == 13) { // ENTER -> xem SV trong lop
            hideCursor(false);
            setColor(7);

            Lop* lopChon = ds.nodes[current];
            inDSSVTrongLop(lopChon);

            // quay lại màn chọn lớp
            hideCursor(true);
            setColor(7);
            clearConsole();

            gotoXY(0, 0);
            cout << "=== DANH SACH LOP (" << ds.n << ") ===\n\n";
            cout << "Dung MUI TEN LEN/XUONG de chon lop,\n";
            cout << "ENTER de xem sinh vien, ESC de quay lai menu.\n\n";

            for (int i = 0; i < ds.n; i = i + 1) {
                char buf[80];
                std::snprintf(buf, sizeof(buf), "%s - %s",
                              ds.nodes[i]->MALOP, ds.nodes[i]->TENLOP);
                drawOptionLine(buf, baseY + i, (i == current));
            }
        } else if (key == KEY_ESC) {
            hideCursor(false);
            setColor(7);
            clearConsole();
            return;
        }
    }
}


// ======= BÁO CÁO =======
void inDSLop(const DS_LOP &ds){
    cout << "\n=== DANH SACH LOP (" << ds.n << ") ===\n";
    for(int i=0;i<ds.n;i++){
        cout << i+1 << ". " << ds.nodes[i]->MALOP << " - " << ds.nodes[i]->TENLOP << "\n";
    }
}

struct RowBaoCaoSV {
    SinhVien* sv;
    const char* malop;
    short y;        // dòng trên console
};

void xemBaiThiCuaSV(TREE_MH dsMH, const char* masv, const char* mamh);



void inTinhTrangThiTheoLop(const DS_LOP &ds,TREE_MH dsMH, const char* mamh){
    std::vector<RowBaoCaoSV> rows;

    auto veLaiManHinh = [&](){
        clearConsole();
        rows.clear();

        std::cout << "=== TINH TRANG THI MON " << mamh << " THEO LOP ===\n";
        std::cout << "Dung MUI TEN LEN/XUONG de chon sinh vien,\n";
        std::cout << "ENTER de xem bai thi, ESC de quay lai menu.\n\n";

        short y = 4;

        for(int i = 0; i < ds.n; i = i + 1){
            Lop* lop = ds.nodes[i];
            if(!lopDuocThiMon(mamh, lop->MALOP)){
                continue;
            }

            gotoXY(0, y);
            std::cout << "[Lop] " << lop->MALOP << " - " << lop->TENLOP << "\n";
            y = y + 1;

            gotoXY(0, y);
            std::cout << std::left << std::setw(16) << "MASV"
                      << std::setw(20) << "HO"
                      << std::setw(12) << "TEN"
                      << "Trang thai";
            y = y + 1;

            PTRSV p = lop->dsSV.pHead;
            while(p != NULL){
                gotoXY(0, y);
                std::cout << std::left
                          << std::setw(16) << p->sv.MASV
                          << std::setw(20) << p->sv.HO
                          << std::setw(12) << p->sv.TEN;
                if(svDaThiMon(p->sv, mamh)){
                    std::cout << "Da thi";
                }else{
                    std::cout << "Chua thi";
                }
                std::cout << "        ";

                RowBaoCaoSV row;
                row.sv    = &p->sv;
                row.malop = lop->MALOP;
                row.y     = y;
                rows.push_back(row);

                y = y + 1;
                p = p->next;
            }

            y = y + 1; // dòng trống giữa các lớp
        }

        if(rows.empty()){
            gotoXY(0, y);
            std::cout << "Khong co sinh vien nao trong cac lop duoc thi mon nay.\n";
        }
    };

    veLaiManHinh();

    if(rows.empty()){
        waitEsc("\nNhan ESC de quay lai menu...");
        clearConsole();
        return;
    }

    int index = 0;

    auto toMauDong = [&](int idx, bool highlight){
        RowBaoCaoSV &row = rows[idx];
        gotoXY(0, row.y);

        if(highlight){
            setColor(240);      // bôi trắng
        }else{
            setColor(7);        // màu thường
        }

        std::cout << std::left
                  << std::setw(16) << row.sv->MASV
                  << std::setw(20) << row.sv->HO
                  << std::setw(12) << row.sv->TEN;
        if(svDaThiMon(*(row.sv), mamh)){
            std::cout << "Da thi";
        }else{
            std::cout << "Chua thi";
        }
        std::cout << "        ";
        setColor(7);
    };

    // tô màu dòng đầu tiên
    toMauDong(index, true);

    while(true){
        int c = _getch();

        if(c == KEY_ESC){
            clearConsole();
            return;
        }

        if(c == 0 || c == 224){
            int c2 = _getch();
            if(c2 == 72){ // lên
                toMauDong(index, false);
                index = index - 1;
                if(index < 0){
                    index = (int)rows.size() - 1;
                }
                toMauDong(index, true);
            }else if(c2 == 80){ // xuống
                toMauDong(index, false);
                index = index + 1;
                if(index >= (int)rows.size()){
                    index = 0;
                }
                toMauDong(index, true);
            }
        }else if(c == 13){ // ENTER
            xemBaiThiCuaSV(dsMH ,rows[index].sv->MASV, mamh);

            // sau khi xem xong quay lại danh sách
            veLaiManHinh();
            if(rows.empty()){
                waitEsc("\nNhan ESC de quay lai menu...");
                clearConsole();
                return;
            }
            if(index >= (int)rows.size()){
                index = (int)rows.size() - 1;
            }
            toMauDong(index, true);
        }
    }
}

static const CauHoi* timCauHoiTheoID(const MonHoc &mh, int id) {
    PTRCH p = mh.dsCH.pHead;
    while (p != NULL) {
        if (p->ch.ID == id) {
            return &p->ch;
        }
        p = p->next;
    }
    return NULL;
}


void xemBaiThiCuaSV(TREE_MH dsMH, const char* masv, const char* mamh){
    std::ifstream f("data/baithi_chitiet.txt");
    if(!f.is_open()){
        clearConsole();
        std::cout << "Khong mo duoc file data/baithi_chitiet.txt\n";
        waitEsc("\nNhan ESC de quay lai...");
        clearConsole();
        return;
    }

    std::string sMasv(masv);
    std::string sMamh(mamh);

    struct Record {
        int lan;
        int  id;
        char daDung;
        char daSv;
    };

    std::vector<Record> recs;

    std::string line;
    while(std::getline(f, line)){
        if(line.empty()){
            continue;
        }
        std::stringstream ss(line);
        std::string fileMasv, fileMamh, lanStr, idStr, daDungStr, daSvStr;

        if(!std::getline(ss, fileMasv, '|')) continue;
        if(!std::getline(ss, fileMamh, '|')) continue;
        if(!std::getline(ss, lanStr,  '|')) continue;
        if(!std::getline(ss, idStr,     '|')) continue;
        if(!std::getline(ss, daDungStr, '|')) continue;
        if(!std::getline(ss, daSvStr,   '|')) continue;

        if(fileMasv != sMasv){
            continue;
        }
        if(fileMamh != sMamh){
            continue;
        }

        Record r;
        r.lan = std::atoi(lanStr.c_str());
        r.id = std::atoi(idStr.c_str());
        r.daDung = daDungStr.empty() ? '-' : daDungStr[0];
        r.daSv   = daSvStr.empty()   ? '-' : daSvStr[0];

        recs.push_back(r);
    }

    int maxLan = 0;
for (auto &r : recs)
    if (r.lan > maxLan)
        maxLan = r.lan;

std::vector<Record> filtered;
for (auto &r : recs)
    if (r.lan == maxLan)
        filtered.push_back(r);

recs = filtered;

    f.close();
    clearConsole();

    if(recs.empty()){
        std::cout << "Khong tim thay du lieu bai thi cho SV "
                  << masv << " mon " << mamh << ".\n";
        waitEsc("\nNhan ESC de quay lai...");
        clearConsole();
        return;
    }

    // Tìm mon hoc trong cay de lay noi dung cau hoi
    nodeMH* node = timMH(dsMH, mamh);
    MonHoc* mh = (node != NULL) ? &node->mh : NULL;

    int soCau  = (int)recs.size();
    int soDung = 0;
    for(size_t i = 0; i < recs.size(); i = i + 1){
        if(recs[i].daSv == recs[i].daDung && recs[i].daSv != '-'){
            soDung = soDung + 1;
        }
    }

    float diem = 0.0f;
    if(soCau > 0){
        diem = soDung * 10.0f / soCau;
    }

    std::cout << "Bai thi chi tiet\n";
    std::cout << "SV: " << masv << "  | Mon: " << mamh << "\n\n";

    for(size_t i = 0; i < recs.size(); i = i + 1){
        Record &r = recs[i];

        std::cout << "Cau ID " << r.id
                  << " | Dap an dung: " << r.daDung
                  << " | SV chon: ";
        if(r.daSv == '-'){
            std::cout << "(khong chon)";
        }else{
            std::cout << r.daSv;
        }
        std::cout << " | Ket qua: ";
        if(r.daSv == '-'){
            std::cout << "Khong lam";
        }else{
            if(r.daSv == r.daDung){
                std::cout << "Dung";
            }else{
                std::cout << "Sai";
            }
        }
        std::cout << "\n";

        if(mh != NULL){
            const CauHoi* q = timCauHoiTheoID(*mh, r.id);
            if(q != NULL){
                std::cout << "  " << q->NoiDung << "\n";
                inDapAnSauKhiNop('A', q->A, r.daSv, r.daDung);
                inDapAnSauKhiNop('B', q->B, r.daSv, r.daDung);
                inDapAnSauKhiNop('C', q->C, r.daSv, r.daDung);
                inDapAnSauKhiNop('D', q->D, r.daSv, r.daDung);
                std::cout << "\n";
            }else{
                std::cout << "  (Khong tim thay noi dung cau hoi trong he thong!)\n\n";
            }
        }else{
            std::cout << "  (Khong tim thay mon hoc trong cay de thi!)\n\n";
        }
    }

    std::cout << "\nSo cau dung: " << soDung << "/" << soCau << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Diem (tinh lai): " << diem << "\n";

    waitEsc("\nNhan ESC de quay lai danh sach sinh vien...");
    clearConsole();
}



void docCauHoiChoMotMon(MonHoc &mh) {
    char filename[128];
    std::snprintf(filename, sizeof(filename), "data/cauhoi_%s.txt", mh.MAMH);

    std::ifstream f(filename);
    if (!f.is_open()) {
        // Mon chua co file cau hoi rieng
        return;
    }

    char line[1024];

    while (f.getline(line, sizeof(line))) {
        if (line[0] == '\0') {
            continue;
        }

        char* token = std::strtok(line, "|");
        if (token == NULL) {
            continue;
        }

        // token 1: MAMH trong file (KTMT, CTDL,...)
        // co the check trung mh.MAMH neu muon
        // if (std::strcmp(token, mh.MAMH) != 0) continue;

        // token 2: ID
        token = std::strtok(NULL, "|");
        if (token == NULL) continue;

        CauHoi ch{};
        ch.ID = std::atoi(token);

        // token 3: Noi dung
        token = std::strtok(NULL, "|");
        if (token == NULL) continue;
        std::strncpy(ch.NoiDung, token, sizeof(ch.NoiDung) - 1);

        // token 4: A
        token = std::strtok(NULL, "|");
        if (token == NULL) continue;
        std::strncpy(ch.A, token, sizeof(ch.A) - 1);

        // token 5: B
        token = std::strtok(NULL, "|");
        if (token == NULL) continue;
        std::strncpy(ch.B, token, sizeof(ch.B) - 1);

        // token 6: C
        token = std::strtok(NULL, "|");
        if (token == NULL) continue;
        std::strncpy(ch.C, token, sizeof(ch.C) - 1);

        // token 7: D
        token = std::strtok(NULL, "|");
        if (token == NULL) continue;
        std::strncpy(ch.D, token, sizeof(ch.D) - 1);

        // token 8: Dap an
        token = std::strtok(NULL, "|");
        if (token == NULL) continue;
        ch.DapAn = (char)std::toupper((unsigned char)token[0]);

        themCauHoi(mh, ch);
    }

    f.close();
}


bool appendCauHoiToFile(const char* mamh, const CauHoi& q) {
    std::string path = fileCauHoiTheoMon(mamh);

    // mở để vừa đọc vừa ghi, con trỏ ở cuối
    std::fstream f(path, std::ios::in | std::ios::out | std::ios::binary);
    if (!f.is_open()) {
        // nếu file chưa tồn tại -> tạo mới
        std::ofstream create(path, std::ios::out | std::ios::binary);
        if (!create.is_open()) return false;
        create.close();
        f.open(path, std::ios::in | std::ios::out | std::ios::binary);
        if (!f.is_open()) return false;
    }

    // nếu file không rỗng thì check ký tự cuối có '\n' chưa
    f.seekg(0, std::ios::end);
    std::streampos sz = f.tellg();
    if (sz > 0) {
        f.seekg(-1, std::ios::end);
        char last = 0;
        f.get(last);
        if (last != '\n') {
            f.clear();
            f.seekp(0, std::ios::end);
            f.put('\n');
        }
    }

    // ghi record + newline
    f.clear();
    f.seekp(0, std::ios::end);
    f << mamh << '|'
      << q.ID << '|'
      << q.NoiDung << '|'
      << q.A << '|'
      << q.B << '|'
      << q.C << '|'
      << q.D << '|'
      << q.DapAn
      << '\n';

    f.close();
    return true;
}

// ================= LOAD CAU HOI THEO MON =================

void loadCauHoiTheoMon(MonHoc &mh) {
    giaiPhongDanhSachCauHoi(mh);
    docCauHoiChoMotMon(mh);
}

bool xacNhanThemSinhVienPopup() {
    int choice = 0; // 0 = YES, 1 = NO

    short x = 0;
    short y = 20; // vẽ ở dưới form

    gotoXY(x, y);
    std::cout << "Ban da nhap xong thong tin sinh vien.\n";
    gotoXY(x, y + 1);
    std::cout << "Ban co chac chan muon THEM sinh vien nay khong?\n";

    auto drawOptions = [&](int ch) {
        gotoXY(x, y + 3);
        if (ch == 0) {
            std::cout << "[YES]    no   ";
        } else {
            std::cout << " yes    [NO]  ";
        }
        std::cout.flush();
    };

    drawOptions(choice);

    gotoXY(x, y + 5);
    std::cout << "Dung phim TRAI/PHAI hoac A/D de doi lua chon, Enter de xac nhan.";
    gotoXY(x, y + 6);
    std::cout << "- ESC: huy thao tac, quay lai form.";

    while (true) {
        int c = _getch();

        if (c == KEY_ESC) {
            return false; // huy
        }

        if (c == 0 || c == 224) {
            int c2 = _getch();
            if (c2 == 75 || c2 == 77) { // LEFT / RIGHT
                if (choice == 0) {
                    choice = 1;
                } else {
                    choice = 0;
                }
                drawOptions(choice);
            }
        } else {
            if (c == 'a' || c == 'A' || c == 'd' || c == 'D') {
                if (choice == 0) {
                    choice = 1;
                } else {
                    choice = 0;
                }
                drawOptions(choice);
            } else if (c == 13) { // Enter
                if (choice == 0) {
                    return true;   // YES
                } else {
                    return false;  // NO
                }
            }
        }
    }
}


bool formSuaCauHoi(CauHoi &ch) {
    int field = 0;
    const int MAX_FIELD = 6; // 0..5 field, 6 = XAC NHAN

    while (true) {
        clearConsole();
        cout << "===== SUA CAU HOI =====\n\n";

        auto drawLine = [&](int idx, const char* label, const char* value) {
            if (field == idx) setColor(240);
            else setColor(7);

            cout << label << ": " << value << "\n";
            setColor(7);
        };

        drawLine(0, "Noi dung", ch.NoiDung);
        drawLine(1, "Dap an A", ch.A);
        drawLine(2, "Dap an B", ch.B);
        drawLine(3, "Dap an C", ch.C);
        drawLine(4, "Dap an D", ch.D);

        char da[2] = { ch.DapAn, '\0' };
        drawLine(5, "Dap an dung", da);

        if (field == 6) setColor(240);
        cout << "[ XAC NHAN SUA ]\n";
        setColor(7);

        cout << "\nUP/DOWN: di chuyen | Go + BACKSPACE: sua | ENTER: xac nhan | ESC: huy\n";

        int c = _getch();

        // ESC
        if (c == KEY_ESC) return false;

        // Mũi tên
        if (c == 0 || c == 224) {
            int c2 = _getch();
            if (c2 == 72) field = (field - 1 + MAX_FIELD + 1) % (MAX_FIELD + 1);
            else if (c2 == 80) field = (field + 1) % (MAX_FIELD + 1);
            continue;
        }

        // ENTER chỉ cho xác nhận
        if (c == 13 && field == 6) {
            return xacNhanSuaCauHoiPopup();
        }

        // ===== GÕ + BACKSPACE TRỰC TIẾP =====
        char* buf = nullptr;
        size_t maxLen = 0;

        switch (field) {
            case 0: buf = ch.NoiDung; maxLen = sizeof(ch.NoiDung); break;
            case 1: buf = ch.A; maxLen = sizeof(ch.A); break;
            case 2: buf = ch.B; maxLen = sizeof(ch.B); break;
            case 3: buf = ch.C; maxLen = sizeof(ch.C); break;
            case 4: buf = ch.D; maxLen = sizeof(ch.D); break;
            case 5:
                if (c >= 'a' && c <= 'd') ch.DapAn = c - 32;
                if (c >= 'A' && c <= 'D') ch.DapAn = c;
                continue;
            default:
                continue;
        }

        if (c == 8) { // BACKSPACE
            size_t len = strlen(buf);
            if (len > 0) buf[len - 1] = '\0';
        }
        else if (isprint(c)) {
            size_t len = strlen(buf);
            if (len + 1 < maxLen) {
                buf[len] = (char)c;
                buf[len + 1] = '\0';
            }
        }
    }
}


bool xacNhanSuaCauHoiPopup() {
    int choice = 0; // 0 = YES, 1 = NO

    short x = 0;
    short y = 20;

    gotoXY(x, y);
    std::cout << "Ban co chac chan muon SUA cau hoi nay khong?\n";

    auto draw = [&](int c){
        gotoXY(x, y + 2);
        if (c == 0)
            std::cout << "[YES]    no   ";
        else
            std::cout << " yes    [NO]  ";
        std::cout.flush();
    };

    draw(choice);

    gotoXY(x, y + 4);
    std::cout << "Dung TRAI/PHAI hoac A/D, ENTER de xac nhan, ESC de huy";

    while (true) {
        int c = _getch();

        if (c == KEY_ESC) return false;

        if (c == 0 || c == 224) {
            int c2 = _getch();
            if (c2 == 75 || c2 == 77) { // LEFT / RIGHT
                choice = 1 - choice;
                draw(choice);
            }
        }
        else if (c == 'a' || c == 'A' || c == 'd' || c == 'D') {
            choice = 1 - choice;
            draw(choice);
        }
        else if (c == 13) { // ENTER
            return (choice == 0);
        }
    }
}


bool nhapSinhVienBangForm(SinhVien &sv) {
    // Khởi tạo rỗng
    sv.MASV[0]     = '\0';
    sv.HO[0]       = '\0';
    sv.TEN[0]      = '\0';
    sv.PHAI[0]     = '\0';
    sv.password[0] = '\0';
    sv.dsDiem.pHead  = NULL;
    sv.dsDiem.pLast  = NULL;
    sv.dsDiem.SLDiem = 0;


    int current      = 0;    // 0..4 = field, 5 = dong XAC NHAN
    const int total  = 6;    // 5 field + 1 dong confirm
    const int baseY  = 6;

    auto renderRow = [&](int index, bool highlight) {
        if (index <= 4) {
            char buf[80];

            if (index == 0) {
                std::snprintf(buf, sizeof(buf), "MASV : %s", sv.MASV);
            } else if (index == 1) {
                std::snprintf(buf, sizeof(buf), "HO   : %s", sv.HO);
            } else if (index == 2) {
                std::snprintf(buf, sizeof(buf), "TEN  : %s", sv.TEN);
            } else if (index == 3) {
                std::snprintf(buf, sizeof(buf), "PHAI : %s", sv.PHAI);
            } else {
                std::snprintf(buf, sizeof(buf), "PASS : %s", sv.password);
            }

            drawOptionLine(buf, baseY + index, highlight);
        } else {
            // Dong [ XAC NHAN THEM SV ]
            const char* txt = "[ XAC NHAN THEM SINH VIEN ]";
            drawOptionLine(txt, baseY + index, highlight);
        }
    };

    auto redrawAll = [&]() {
        clearConsole();
        hideCursor(true);
        setColor(7);

        gotoXY(0, 0);
        std::cout << "=== NHAP SINH VIEN VAO LOP ===\n\n";
        std::cout << "Dung MUI TEN LEN/XUONG de chon truong hoac nut XAC NHAN.\n";
        std::cout << "Nhap chu truc tiep tren dong duoc to sang.\n";
        std::cout << "ENTER hoac MUI TEN de chuyen dong, ESC de huy.\n\n";

        for (int i = 0; i < total; i = i + 1) {
            renderRow(i, (i == current));
        }
    };

    // Vẽ lần đầu
    redrawAll();

    while (true) {
        int c = _getch();

        // ESC: huy form
        if (c == KEY_ESC) {
            hideCursor(false);
            setColor(7);
            return false;
        }

        // Mũi tên
        if (c == 0 || c == 224) {
            int k2 = _getch();
            if (k2 == 72) { // LEN
                int old = current;
                current = current - 1;
                if (current < 0) {
                    current = total - 1;
                }
                renderRow(old, false);
                renderRow(current, true);
            } else if (k2 == 80) { // XUONG
                int old = current;
                current = current + 1;
                if (current >= total) {
                    current = 0;
                }
                renderRow(old, false);
                renderRow(current, true);
            }
            continue;
        }

        // ENTER
        if (c == 13) {
            // Nếu đang đứng ở dòng [ XAC NHAN ] -> validate + popup YES/NO

            // ❌ Không cho bỏ trống field
if (current <= 3) {
    char* field = nullptr;
    if (current == 0) field = sv.MASV;
    else if (current == 1) field = sv.HO;
    else if (current == 2) field = sv.TEN;
    else if (current == 3) field = sv.PHAI;

    if (field && std::strlen(field) == 0) {
        Beep(900, 120);
        continue;   // ❌ không cho ENTER đi tiếp
    }
}


            if (current == 5) {
                hideCursor(false);
                setColor(7);

                std::string msg;
                if (!validateSinhVien(sv, msg)) {
                    std::cout << "\nLoi: " << msg << "\n";
                    waitEsc("\nNhan ESC de tiep tuc chinh sua...");
                    redrawAll();
                    continue;
                }

                bool dongY = xacNhanThemSinhVienPopup();
                if (dongY) {
                    return true;   // thêm SV
                } else {
                    // quay lại form
                    redrawAll();
                    continue;
                }
            } else {
                // Enter trên field -> chuyển sang field tiếp theo
                int old = current;
                current = current + 1;
                if (current >= 5) {
                    current = 5; // nhảy xuống luôn dòng Xác nhận
                }
                renderRow(old, false);
                renderRow(current, true);
                continue;
            }
        }

        // BACKSPACE: xóa 1 ký tự ở field hiện tại (chỉ cho 0..4)
        if (c == 8 && current <= 4) {
            char* field = NULL;
            size_t maxLen = 0;

            if (current == 0) {
                field = sv.MASV;
                maxLen = sizeof(sv.MASV);
            } else if (current == 1) {
                field = sv.HO;
                maxLen = sizeof(sv.HO);
            } else if (current == 2) {
                field = sv.TEN;
                maxLen = sizeof(sv.TEN);
            } else if (current == 3) {
                field = sv.PHAI;
                maxLen = sizeof(sv.PHAI);
            } else {
                field = sv.password;
                maxLen = sizeof(sv.password);
            }

            size_t len = std::strlen(field);
            if (len > 0) {
                field[len - 1] = '\0';
                renderRow(current, true);
            }
            continue;
        }

        // Ký tự in được -> thêm vào field hiện tại (0..4)
        // ===== NHẬP KÝ TỰ – CHẶN SAI NGAY TỪ BÀN PHÍM =====
if (c >= 32 && c < 127 && current <= 4) {
    char* field = nullptr;
    size_t maxLen = 0;
    bool hopLe = true;

    // MASV: chỉ chữ + số
    if (current == 0) {
        if (!std::isalnum(c)) hopLe = false;
        field = sv.MASV;
        maxLen = sizeof(sv.MASV);
    }
    // HO: chỉ chữ + khoảng trắng
    else if (current == 1) {
        if (!std::isalpha(c) && c != ' ') hopLe = false;
        field = sv.HO;
        maxLen = sizeof(sv.HO);
    }
    // TEN: chỉ chữ + khoảng trắng
    else if (current == 2) {
        if (!std::isalpha(c) && c != ' ') hopLe = false;
        field = sv.TEN;
        maxLen = sizeof(sv.TEN);
    }
    // PHAI: chỉ cho nhập N/n hoặc U/u
    else if (current == 3) {
        if (c == 'n' || c == 'N') {
            strcpy(sv.PHAI, "Nam");
            renderRow(current, true);
            continue;
        }
        if (c == 'u' || c == 'U') {
            strcpy(sv.PHAI, "Nu");
            renderRow(current, true);
            continue;
        }
        Beep(700, 80);
        continue;
    }
    // PASSWORD: không cho space
    else {
        if (std::isspace(c)) {
            Beep(700, 80);
            continue;
        }
        field = sv.password;
        maxLen = sizeof(sv.password);
    }

    if (!hopLe) {
        Beep(700, 80);
        continue;   // ❌ CHẶN NGAY – KHÔNG CHO NHẬP
    }

    size_t len = std::strlen(field);
    if (len + 1 < maxLen) {
        field[len] = (char)c;
        field[len + 1] = '\0';
        renderRow(current, true);
    }
}

    }
}

// gom cac mon hoc ma lop duoc thi vao 1 vector
static void thuThapMonChoLop(TREE_MH t, const char* malop, std::vector<nodeMH*>& ds) {
    if (t == NULL) {
        return;
    }

    thuThapMonChoLop(t->left, malop, ds);

    // neu malop = NULL thi lay tat ca mon,
    // con nguoc lai chi lay mon ma lop duoc thi
    if (malop == NULL || lopDuocThiMon(t->mh.MAMH, malop)) {
        ds.push_back(t);
    }

    thuThapMonChoLop(t->right, malop, ds);
}

nodeMH* chonMonThiBangMuiTen(TREE_MH dsMH, const char* malop) {
    std::vector<nodeMH*> dsMon;
    thuThapMonChoLop(dsMH, malop, dsMon);

    if (dsMon.empty()) {
        clearConsole();
        std::cout << "Lop cua ban khong co mon nao duoc dang ky de thi.\n";
        waitEsc("\nNhan ESC de quay lai menu...");
        clearConsole();
        return NULL;
    }

    // tao chuoi "MAMH - TENMH" cho tung mon
    std::vector<std::string> labels;
    for (size_t i = 0; i < dsMon.size(); i = i + 1) {
        std::string line = std::string(dsMon[i]->mh.MAMH);
        line += " - ";
        line += dsMon[i]->mh.TENMH;
        labels.push_back(line);
    }

    hideCursor(true);
    setColor(7);

    UILayout L;
    beginScreenBox("=== CHON MON THI ===", 90, 22, L);
    boxPrint(L, 3, 2, "DUNG MUI TEN LEN/XUONG DE CHON MON");
    boxPrint(L, 3, 3, "ENTER de xac nhan, ESC de quay lai");

    int current = 0;
    int total   = (int)labels.size();


    short baseX = L.boxX + 3;
    short baseY = L.boxY + 5;
    int lineW   = L.boxW - 6;


    // in lan dau
    for (int i = 0; i < total; i = i + 1) {
        drawOptionLineAt(baseX, baseY+i, labels[i].c_str(), (i == current),lineW);
    }

    while (true) {
        int key = _getch();

        if (key == 0 || key == 224) {
            int k2 = _getch();
            if (k2 == 72) { // LEN
                drawOptionLineAt(baseX, baseY+current,labels[current].c_str(), false,lineW);
                current = current - 1;
                if (current < 0) {
                    current = total - 1;
                }
                drawOptionLineAt(baseX,baseY+current,labels[current].c_str(), true,lineW);
            } 
            else if (k2 == 80) { // XUONG
                drawOptionLineAt(baseX,baseY+current,labels[current].c_str(), false,lineW);
                current = current + 1;
                if (current >= total) {
                    current = 0;
                }
                drawOptionLineAt(baseX,baseY+current,labels[current].c_str(), true,lineW);
            }
        } else if (key == 13) { // ENTER
            hideCursor(false);
            setColor(7);
            clearConsole();
            return dsMon[current]; // tra ve node mon hoc duoc chon
        } else if (key == KEY_ESC) {
            hideCursor(false);
            setColor(7);
            clearConsole();
            return NULL; // huy
        }
    }
}


// Chon 1 cau hoi trong 1 mon bang mui ten
// Tra ve con tro nodeCH duoc chon, hoac NULL neu ESC / khong co cau hoi
nodeCH* chonCauHoiTrongMon(MonHoc &mh) {
    std::vector<nodeCH*> ds;
    PTRCH p = mh.dsCH.pHead;
    while (p != NULL) {
        ds.push_back(p);
        p = p->next;
    }

    if (ds.empty()) {
        clearConsole();
        std::cout << "Mon " << mh.MAMH << " - " << mh.TENMH
                  << " chua co cau hoi nao.\n";
        waitEsc("\nNhan ESC de quay lai...");
        clearConsole();
        return NULL;
    }

    hideCursor(true);
    setColor(7);
    clearConsole();

    int total    = (int)ds.size();
    int current  = 0;      // chi so cau hoi dang duoc chon
    int pageSize = 10;     // so cau/1 trang
    int baseY    = 6;      // dong bat dau ve danh sach
    int pageTop  = 0;      // chi so dau tien cua trang hien tai

    // in header 1 lan
    gotoXY(0, 0);
    std::cout << "=== CHON CAU HOI ===\n\n";
    std::cout << "Mon: " << mh.MAMH << " - " << mh.TENMH << "\n";
    std::cout << "Dung MUI TEN LEN/XUONG de chon cau hoi,\n";
    std::cout << "ENTER de xac nhan, ESC de quay lai.\n";

    auto buildLabel = [&](int idx, char* buf, size_t bufSize) {
        const CauHoi &ch = ds[idx]->ch;
        const char* nd   = ch.NoiDung;

        char ndCut[70];
        std::strncpy(ndCut, nd, 69);
        ndCut[69] = '\0';

        std::snprintf(buf, bufSize, "ID %d: %s", ch.ID, ndCut);
    };

    auto drawPage = [&]() {
        pageTop = (current / pageSize) * pageSize;
        for (int i = 0; i < pageSize; i = i + 1) {
            int idx = pageTop + i;
            char buf[120];

            bool highlight = false;
            if (idx < total) {
                buildLabel(idx, buf, sizeof(buf));
                if (idx == current) {
                    highlight = true;
                }
            } else {
                buf[0] = '\0'; // dong trong
            }

            drawOptionLine(buf, baseY + i, highlight);
        }
    };

    // ve trang dau tien
    drawPage();

    while (true) {
        int c = _getch();

        if (c == KEY_ESC) {
            hideCursor(false);
            setColor(7);
            clearConsole();
            return NULL;
        }

        if (c == 0 || c == 224) {
            int k2 = _getch();
            if (k2 == 72) {        // LEN
                int oldCurrent = current;
                current = current - 1;
                if (current < 0) {
                    current = total - 1;
                }

                int oldPage = oldCurrent / pageSize;
                int newPage = current   / pageSize;

                if (oldPage != newPage) {
                    // sang trang khac -> ve lai ca trang
                    drawPage();
                } else {
                    // cung trang -> chi ve lai 2 dong
                    char buf[120];

                    buildLabel(oldCurrent, buf, sizeof(buf));
                    drawOptionLine(buf,
                                   baseY + (oldCurrent - pageTop),
                                   false);

                    buildLabel(current, buf, sizeof(buf));
                    drawOptionLine(buf,
                                   baseY + (current - pageTop),
                                   true);
                }
            } else if (k2 == 80) { // XUONG
                int oldCurrent = current;
                current = current + 1;
                if (current >= total) {
                    current = 0;
                }

                int oldPage = oldCurrent / pageSize;
                int newPage = current   / pageSize;

                if (oldPage != newPage) {
                    drawPage();
                } else {
                    char buf[120];

                    buildLabel(oldCurrent, buf, sizeof(buf));
                    drawOptionLine(buf,
                                   baseY + (oldCurrent - pageTop),
                                   false);

                    buildLabel(current, buf, sizeof(buf));
                    drawOptionLine(buf,
                                   baseY + (current - pageTop),
                                   true);
                }
            }
        } else if (c == 13) { // ENTER
            hideCursor(false);
            setColor(7);
            nodeCH* res = ds[current];
            clearConsole();
            return res;
        }
    }
}


 
// ======= MENU GIẢNG VIÊN (đầy đủ) =======
// ============================
//  MENU LOGIC (GV / SV / CHINH)
// ============================ 
void menuGV(TREE_MH &dsMH, DS_LOP &dsLop) {
    while (true) {
        int ch = menuArrowGV();   // menu mũi tên, không nháy

        if (ch == -1) {
            return;
        }

        // ================== 0. ĐĂNG XUẤT ==================
        if (ch == 0) {
            hideCursor(false);
            setColor(7);
            clearConsole();
            return;
        }

        // ================== 1. NHẬP LỚP ==================
        else if (ch == 1) {
            clearConsole();

            char ml[16], tl[51];
            if (!readLineEsc("Ma lop (ESC de huy): ", ml, sizeof(ml))) {
                clearConsole();
                continue;
            }
            if (!readLineEsc("Ten lop (ESC de huy): ", tl, sizeof(tl))) {
                clearConsole();
                continue;
            }

            if (themLop(dsLop, ml, tl)) {
                if (saveLop("data/lop.txt", dsLop))
                    cout << "OK (da luu file)\n";
                else
                    cout << "Da them lop nhung KHONG ghi duoc file!\n";
            } else {
                cout << "That bai (day danh sach hoac loi khac).\n";
            }

            waitEsc("\nNhan ESC de quay lai menu...");
            clearConsole();
        }

        // ================== 2. IN DS LỚP ==================
        else if (ch == 2) {
            xemDSLopBangMuiTen(dsLop);
        }

        // ================== 3. NHẬP SV VÀO LỚP ==================
        else if (ch == 3) {
            clearConsole();

            Lop* lop = chonLopBangMuiTen(dsLop);
            if (lop == NULL) {
                clearConsole();
                continue;
            }

            SinhVien sv{};
            if (!nhapSinhVienBangForm(sv)) {
                clearConsole();
                continue;
            }

            sv.dsDiem.pHead  = NULL;
            sv.dsDiem.pLast  = NULL;
            sv.dsDiem.SLDiem = 0;

            if (themSVVaoLop(lop, sv)) {
                cout << "Da them sinh vien vao lop " << lop->MALOP << ".\n";
            } else {
                cout << "Them that bai (co the trung MASV).\n";
            }

            waitEsc("\nNhan ESC de quay lai menu...");
            clearConsole();
        }

        // ================== 4. THÊM / CẬP NHẬT MÔN ==================
        else if (ch == 4) {
            clearConsole();

            char mamh[16], tenmh[51];
            if (!readLineEsc("Ma MH (ESC de huy): ", mamh, sizeof(mamh))) {
                clearConsole();
                continue;
            }
            if (!readLineEsc("Ten MH (ESC de huy): ", tenmh, sizeof(tenmh))) {
                clearConsole();
                continue;
            }

            chenMonHoc(dsMH, mamh, tenmh);
            cout << "Da them/cap nhat mon hoc.\n";

            waitEsc("\nNhan ESC de quay lai menu...");
            clearConsole();
        }

        // ================== 5. THÊM CÂU HỎI ==================
        else if (ch == 5) {
            clearConsole();

            nodeMH* n = chonMonThiBangMuiTen(dsMH, NULL);
            if (n == NULL) {
                clearConsole();
                continue;
            }

            loadCauHoiTheoMon(n->mh);

            CauHoi q{};
            int id;
            do {
                id = randomID();
            } while (idTonTai(n->mh.dsCH.pHead, id));
            q.ID = id;

            cout << "Them cau hoi cho mon: "
                 << n->mh.MAMH << " - " << n->mh.TENMH << "\n\n";

            if (!readLineEsc("Noi dung (ESC de huy): ", q.NoiDung, 256)) { clearConsole(); continue; }
            if (!readLineEsc("A (ESC de huy): ", q.A, 100)) { clearConsole(); continue; }
            if (!readLineEsc("B (ESC de huy): ", q.B, 100)) { clearConsole(); continue; }
            if (!readLineEsc("C (ESC de huy): ", q.C, 100)) { clearConsole(); continue; }
            if (!readLineEsc("D (ESC de huy): ", q.D, 100)) { clearConsole(); continue; }

            char buf[8];
            if (!readLineEsc("Dap an (A/B/C/D): ", buf, sizeof(buf))) {
                clearConsole();
                continue;
            }

            q.DapAn = (char)toupper((unsigned char)buf[0]);
            if (q.DapAn < 'A' || q.DapAn > 'D') q.DapAn = 'A';

            themCauHoi(n->mh, q);
            appendCauHoiToFile(n->mh.MAMH, q);

            cout << "Da them cau hoi (ID " << q.ID << ").\n";
            waitEsc("\nNhan ESC de quay lai menu...");
            clearConsole();
        }

        // ================== 6. SỬA CÂU HỎI ==================
        else if (ch == 6) {
            clearConsole();

            nodeMH* n = chonMonThiBangMuiTen(dsMH, NULL);
            if (n == NULL) {
                clearConsole();
                continue;
            }

            loadCauHoiTheoMon(n->mh);

            nodeCH* qNode = chonCauHoiTrongMon(n->mh);
            if (qNode == NULL) {
                clearConsole();
                continue;
            }

            clearConsole();

            bool daXacNhanSua = formSuaCauHoi(qNode->ch);

            if (daXacNhanSua) {
            if (xacNhanSuaCauHoiPopup()) {
            ghiLaiFileCauHoi(n->mh);   // ✅ chỉ ghi khi YES
            std::cout << "\nDa cap nhat cau hoi.\n";
            } else {
            loadCauHoiTheoMon(n->mh);  // 🔄 rollback
            std::cout << "\nDa huy sua cau hoi.\n";
    }
}
else{
    std::cout << "\nDa huy sua cau hoi\n";
}
waitEsc("\nNhan ESC de quay lai menu...");
clearConsole();
}

        // ================== 7. XOÁ CÂU HỎI ==================
        else if (ch == 7) {
            clearConsole();

            nodeMH* n = chonMonThiBangMuiTen(dsMH, NULL);
            if (n == NULL) {
                clearConsole();
                continue;
            }

            loadCauHoiTheoMon(n->mh);

            nodeCH* qNode = chonCauHoiTrongMon(n->mh);
            if (qNode == NULL) {
                clearConsole();
                continue;
            }

            clearConsole();
            cout << "Ban chac chan muon xoa cau hoi ID "
                 << qNode->ch.ID << " ? (Y/N): ";

            while (true) {
                int c = _getch();
                if (c == 'y' || c == 'Y') {
                    xoaCauHoi(n->mh, qNode->ch.ID, dsLop, n->mh.MAMH);
                    break;
                }
                if (c == 'n' || c == 'N' || c == KEY_ESC) {
                    break;
                }
            }

            waitEsc("\nNhan ESC de quay lai menu...");
            clearConsole();
        }

        // ================== 8. IN CÂU HỎI ==================
        else if (ch == 8) {
            clearConsole();

            nodeMH* n = chonMonThiBangMuiTen(dsMH, NULL);
            if (n == NULL) {
                clearConsole();
                continue;
            }

            loadCauHoiTheoMon(n->mh);

            cout << "=== DANH SACH CAU HOI ===\n";
            inDanhSachCauHoi(n->mh);

            waitEsc("\nNhan ESC de quay lai menu...");
            clearConsole();
        }

        // ================== 9. BÁO CÁO ==================
        else if (ch == 9) {
            clearConsole();

            nodeMH* n = chonMonThiBangMuiTen(dsMH, NULL);
            if (n == NULL) {
                clearConsole();
                continue;
            }

            inTinhTrangThiTheoLop(dsLop, dsMH, n->mh.MAMH);

            waitEsc("\nNhan ESC de quay lai menu...");
            clearConsole();
        }

        // ================== 10. LẬP LỊCH THI ==================
else if (ch == 10) {
    clearConsole();

    nodeMH* n = chonMonThiBangMuiTen(dsMH, NULL);
    if (n == NULL) {
        clearConsole();
        continue;
    }
    loadCauHoiTheoMon(n->mh);
    int tongCH = demCauHoi(n->mh.dsCH.pHead);

    cout << "Lap lich thi cho mon: "
         << n->mh.MAMH << " - " << n->mh.TENMH << "\n\n";

    int soCau, soPhut;

    if (!readIntEsc("Nhap so cau thi: ", soCau)) {
        clearConsole();
        continue;
    }
    if (!readIntEsc("Nhap so phut thi: ", soPhut)) {
        clearConsole();
        continue;
    }

    if (soCau <= 0 || soPhut <= 0) {
        cout << "Gia tri khong hop le.\n";
        waitEsc("\nNhan ESC de quay lai...");
        clearConsole();
        continue;
    }

    tongCH = demCauHoi(n->mh.dsCH.pHead);
    if (soCau > tongCH) {
        cout << "So cau thi lon hon so cau hien co (" << tongCH << ").\n";
        waitEsc("\nNhan ESC de quay lai...");
        clearConsole();
        continue;
    }

    n->mh.soCauThi  = soCau;
    n->mh.soPhutThi = soPhut;
    n->mh.trangThaiThi = 1;

    cout << "Da lap lich thi thanh cong!\n";
    waitEsc("\nNhan ESC de quay lai menu...");
    clearConsole();
}

    }
}


bool xacNhanThiMon(const MonHoc& mh, int& soCau, int& soPhut) {
    UILayout L;
    beginScreenBox("XAC NHAN THI", 72, 12, L);

    boxPrint(L, 3, 2, "Ban chac chan muon thi mon:");
    boxPrint(L, 3, 3, "%s - %s", mh.MAMH, mh.TENMH);

    boxPrint(L, 3, 5, "Nhan Y de dong y, N hoac ESC de huy");

    while (true) {
        int k = readKey();
        if (k == 'Y' || k == 'y') break;

        if (k == 'N' || k == 'n' || k == 27) {
            return false;
        }   
    }
     soCau  = mh.soCauThi;
     soPhut = mh.soPhutThi;

    return true;
}

void drawSidePanel(short x, short y, short w, short h,
                   int soCau, int idx,
                   const std::vector<char>& ans) {

    gotoXY(x + 2, y + 1);
    std::cout << "DAP AN";

    int listTop = y + 3;
    int listMax = h - 5;

    int start = idx - listMax / 2;
    if (start < 0) start = 0;
    if (start > soCau - listMax) start = soCau - listMax;
    if (start < 0) start = 0;

    for (int i = 0; i < listMax; i++) {
        int q = start + i;

        gotoXY(x + 1, (short)(listTop + i));
        for (int t = 0; t < w - 2; t++) std::cout << ' ';

        if (q >= soCau) continue;

        gotoXY(x + 2, (short)(listTop + i));

        bool hl = (q == idx);
        if (hl) setColor(240);
        else setColor(7);

        char a = ans[q];
        if (a == 0) a = '.';

        if (hl) std::cout << "> ";
        else std::cout << "  ";

        if (q + 1 < 10) std::cout << "0";
        std::cout << (q + 1) << ": " << a;

        setColor(7);
    }

    gotoXY(x + 2, y + h - 2);
    std::cout << "UP/DOWN";
}

void drawQuestionMain(const UILayout& L,int idx, int soCau,const CauHoi& q,int curOpt,char chosen) {

    drawScreenBox("DE THI TRAC NGHIEM", L);

    boxPrint(L, 3, 1, "Cau %d/%d (ID %d)", idx + 1, soCau, q.ID);
    boxPrint(L, 3, 3, "%s", q.NoiDung);

    int baseY = L.boxY + 6;

    bool daA = (chosen == 'A');
    bool daB = (chosen == 'B');
    bool daC = (chosen == 'C');
    bool daD = (chosen == 'D');

    drawAnswerLine("A.", q.A, baseY + 0, curOpt == 0, daA);
    drawAnswerLine("B.", q.B, baseY + 1, curOpt == 1, daB);
    drawAnswerLine("C.", q.C, baseY + 2, curOpt == 2, daC);
    drawAnswerLine("D.", q.D, baseY + 3, curOpt == 3, daD);

    boxPrint(L, 3, L.boxH - 3, "ENTER: chon dap an | ESC: huy | <-/-> doi cau | ^/v chon dap an");
}

void drawAnswerPanel(short x, short y, short w, short h,int soCau, int idx,const std::vector<char>& ans) {

    // tiêu đề panel
    gotoXY(x + 2, y + 1);
    std::cout << "DAP AN";

    int top = y + 3;
    int maxLine = h - 4;   // số dòng hiển thị được

    // cửa sổ cuộn theo idx
    int start = idx - maxLine / 2;
    if (start < 0) start = 0;
    if (start > soCau - maxLine) start = soCau - maxLine;
    if (start < 0) start = 0;

    for (int i = 0; i < maxLine; i++) {
        int q = start + i;
        if (q >= soCau) break;

        gotoXY(x + 1, (short)(top + i));
        for (int k = 0; k < w - 2; k++) std::cout << ' ';

        gotoXY(x + 2, (short)(top + i));

        bool hl = (q == idx);
        if (hl) setColor(240);
        else setColor(7);

        char a = ans[q];
        if (a == 0) a = '.';

        // ví dụ: > Cau 01: A
        std::cout << "  ";
        std::cout << "Cau " << (q + 1) << ": " << a;

        setColor(7);
    }
}


static void menuSV(TREE_MH &dsMH, DS_LOP &dsLop, SinhVien* sv, Lop *lop){
    while (true) {
        int ch = menuArrowSV(sv);   // chọn bằng mũi tên

        if (ch == 0) {
            // 0 = Dang xuat / ESC
            return;
        }
        else if (ch == 1) {
            // === THI TRAC NGHIEM như code cũ của anh ===
            clearConsole();

            if (lop == NULL) {
                std::cout << "Khong xac dinh duoc lop cua ban. Khong the vao thi.\n";
                waitEsc("\nNhan ESC de quay lai menu...");
                clearConsole();
                continue;
            }

            nodeMH* n = chonMonThiBangMuiTen(dsMH, lop->MALOP);
            if (n == NULL) continue;
            loadCauHoiTheoMon(n->mh);

            if (n->mh.trangThaiThi !=1) {
                    cout << "mon nay chua len lich\n";
                    waitEsc("\nNhan ESC de quay lai...");
                    clearConsole();
                    continue;
            }

            int soCau = 0;
            int soPhut = 0;
            if (!xacNhanThiMon(n->mh, soCau, soPhut)) {
                waitEsc("\nBan da huy. Nhan ESC de quay lai menu...");
                clearConsole();
                continue;
            }
            int lanThi = demLanThiTrongFile(sv->MASV, n->mh.MAMH) + 1;
            float d = thiTracNghiem(n->mh, soCau, soPhut, sv->MASV, n->mh.MAMH, lanThi);


            int tong = demCauHoi(n->mh.dsCH.pHead);
            if (tong == 0) {
                std::cout << "Mon chua co cau hoi.\n";
                waitEsc("\nNhan ESC de quay lai menu...");
                clearConsole();
                continue;
            }
            if (soCau > tong) {
                soCau = tong;
            }
            if (d >= 0.0f) {
                if (luuDiem(*sv, n->mh.MAMH, d)) {
                    std::cout << "Da luu diem.\n";
                } else {
                    std::cout << "";
                }
            }

            waitEsc("\nNhan ESC de quay lai menu...");
            clearConsole();
        }
    }
}


bool readPassword(const char* prompt, char* out, size_t maxLen) {
    cout << prompt;
    size_t len = 0;
    bool show = false; // false = hiện *, true = hiện rõ

    while (true) {
        int c = _getch();

        // ENTER -> kết thúc nhập
        if (c == 13) {
            cout << "\n";
            out[len] = '\0';
            return true;
        }

        // BACKSPACE
        if (c == 8) {
            if (len > 0) {
                len = len - 1;
                out[len] = '\0';
                cout << "\b \b";
            }
            continue;
        }

        // Nhấn O/o -> bật/tắt hiện mật khẩu
        if (c == 'o' || c == 'O') {
            show = !show;

            // Vẽ lại cả dòng theo mode mới
            cout << "\r";
            cout << prompt;
            for (size_t i = 0; i < len; i = i + 1) {
                if (show == true) {
                    cout << out[i];
                } else {
                    cout << '*';
                }
            }
            continue;
        }

        // Ký tự thường
        if (c >= 32 && c < 127) {
            if (len + 1 < maxLen) {
                out[len] = (char)c;
                len = len + 1;

                if (show == true) {
                    cout << (char)c;
                } else {
                    cout << '*';
                }
            }
        }
    }
}



void menuChinh(TREE_MH &dsMH, DS_LOP &dsLop) {

    while (true) {
        clearConsole();

        short CW, CH;
        getConsoleWindowSize(CW, CH);

        const short BOX_W = 80;
        const short BOX_H = 12;

        short boxX = (CW - BOX_W) / 2;
        short boxY = (CH - BOX_H) / 2;

        boxX += 22;
        boxY += 8;

        // ===== TIÊU ĐỀ =====
        setColor(11); // cyan
        printCenter(boxY - 3, "=== HE THONG THI TRAC NGHIEM ===", CW);
        setColor(7);

        // ===== KHUNG =====
        drawBox(boxX, boxY, BOX_W, BOX_H);

        // ===== LABEL =====
        gotoXY(boxX + 4, boxY + 3);
        cout << "Tai khoan:";

        gotoXY(boxX + 4, boxY + 5);
        cout << "Mat khau :";

        // ===== INPUT =====
        char u[32], p[32];

        gotoXY(boxX + 4, boxY + 3); cout << "Tai khoan:";
        gotoXY(boxX + 4, boxY + 5); cout << "Mat khau :";
        gotoXY(boxX + 4, boxY + 8); setColor(8);
        cout << "Enter: next/submit | Up/Down: chon o | Ctrl+o: hien/an | ESC: thoat";
        setColor(7);

        bool ok = loginForm(u, 32, p, 32, boxX, boxY);
        if (!ok) return; // hoặc break/thoát chương trình

        // ===== XỬ LÝ ĐĂNG NHẬP =====
        Lop* lopOfSV = NULL;
        SinhVien* sv = dangNhap(dsLop, u, p, &lopOfSV);

        if (strcmp(u, "GV") == 0 && strcmp(p, "GV") == 0) {
            menuGV(dsMH, dsLop);
        }
        else if (sv != NULL) {
            menuSV(dsMH, dsLop, sv, lopOfSV);
        }
        else {
            gotoXY(boxX + 4, boxY + BOX_H - 3);
            setColor(12); // đỏ
            cout << "Sai thong tin dang nhap!";
            setColor(7);
            waitEsc("\nNhan ESC de thu lai...");
        }
    }
}


bool saveLop(const char* path, const DS_LOP& ds){
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if(!out.is_open()){
        std::cout << "Khong mo duoc file de ghi: " << path << "\n";
        return false;
    }
    // header
    out << "MALOP|TENLOP\n";
    // dữ liệu
    for(int i=0; i<ds.n; ++i){
        if(ds.nodes[i])
            out << ds.nodes[i]->MALOP << '|' << ds.nodes[i]->TENLOP << "\n";
    }
    out.close();
    return true;
}

// ================= VALIDATE MON HOC / SINH VIEN / CAU HOI =================









void inCauHoiChon(const CauHoi &q, int chiSoLuaChon) {
    // chiSoLuaChon: 0 = A, 1 = B, 2 = C, 3 = D

    //clearConsole(); // dùng hàm của anh

    std::cout << "ID: " << q.ID << "\n";
    std::cout << q.NoiDung << "\n\n";

    const char* label[4] = { "A", "B", "C", "D" };
    const char* nd[4] = { q.A, q.B, q.C, q.D };

    for (int i = 0; i < 4; i++) {
        if (i == chiSoLuaChon) {
            std::cout << ">> " << label[i] << ". " << nd[i] << "\n";
        } else {
            std::cout << "   " << label[i] << ". " << nd[i] << "\n";
        }
    }

    std::cout << "\nDung phim LEN/XUONG de chon, Enter de xac nhan.\n";
}





int chonDapAnBangMuiTen(const CauHoi &q, int yStart, char daChonCu) {
    int pos = 0; // 0=A, 1=B, 2=C, 3=D

    const char* label[4] = { "A.", "B.", "C.", "D." };
    const char* nd[4]    = { q.A, q.B, q.C, q.D };

    // In 4 đáp án lần đầu
    for (int i = 0; i < 4; ++i) {
        bool highlight = (i == pos);
        bool daChon = false;
        if(daChonCu==('A'+i)){
            daChon = true;
        }
        drawAnswerLine(label[i], nd[i], yStart + i, highlight, daChon);
    }
    
    while (true) {
        if (_kbhit()) {
            int c = _getch();

            if (c == KEY_ESC) {
                return -1; // thoát chọn
            }

            if (c == 0 || c == 224) {
                int k = _getch();
                if (k == 72) {            // ↑
                    bool oldChosen = (daChonCu == ('A' + pos));
                    drawAnswerLine(label[pos], nd[pos], yStart + pos, false,oldChosen);
                    pos=pos-1;
                    if (pos < 0) pos = 3;
                    bool newChosen = (daChonCu == ('A' + pos));
                    drawAnswerLine(label[pos], nd[pos], yStart + pos, true);
                } else if (k == 80) {     // ↓
                    bool oldChosen = (daChonCu == ('A' + pos));
                    drawAnswerLine(label[pos], nd[pos], yStart + pos, false);
                    pos=pos+1;
                    if (pos > 3) pos = 0;
                    bool newChosen = (daChonCu == ('A' + pos));
                    drawAnswerLine(label[pos], nd[pos], yStart + pos, true);
                }
            } else if (c == 13) {         // ENTER
                return pos;               // 0..3
            }
        }
        Sleep(10);
    }
}


// ============================
//  PERSISTENCE (SAVE/LOAD)
// ============================
void luuKetQuaThi(const SinhVien &sv, const char* mamh, int diem, int thoiGian) {
    ofstream f("ketquathi.txt", ios::app);
    if (!f.is_open()) {
        cout << "Khong mo duoc file ketquathi.txt\n";
        return;
    }

    // ========================
    // Lấy ngày hiện tại (ĐẶT Ở ĐÂY)
    // ========================
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char ngayThi[11];
    sprintf(ngayThi, "%04d-%02d-%02d",
            1900 + ltm->tm_year,
            1 + ltm->tm_mon,
            ltm->tm_mday);

    // ========================
    // Ghi dữ liệu vào file
    // ========================
    f << sv.MASV << "|"
      << mamh      << "|"
      << diem      << "|"
      << thoiGian  << "|"
      << ngayThi   << "\n";

    f.close(); // đóng file
}

// ============================
//  VALIDATION
// ============================
bool ValidateMonHoc(const MonHoc& mh, TREE_MH root) {
    size_t lenMa  = std::strlen(mh.MAMH);
    size_t lenTen = std::strlen(mh.TENMH);

    if (lenMa == 0) {
        std::cout << " Loi: Ma mon hoc khong duoc rong.\n";
        return false;
    }
    if (lenMa > 15) {
        std::cout << " Loi: Ma mon hoc qua dai (toi da 15 ky tu).\n";
        return false;
    }
    for (size_t i = 0; i < lenMa; ++i) {
        unsigned char c = (unsigned char)mh.MAMH[i];
        if (!std::isalnum(c)) {
            std::cout << " Loi: Ma mon hoc chi duoc chua chu va so.\n";
            return false;
        }
    }

    if (lenTen == 0) {
        std::cout << " Loi: Ten mon hoc khong duoc rong.\n";
        return false;
    }
    if (lenTen > 50) {
        std::cout << " Loi: Ten mon hoc qua dai (toi da 50 ky tu).\n";
        return false;
    }
    for (size_t i = 0; i < lenTen; ++i) {
        unsigned char c = (unsigned char)mh.TENMH[i];
        if (!(std::isalnum(c) || std::isspace(c) || c == '-' || c == '_')) {
            std::cout << " Loi: Ten mon hoc chi duoc chua chu, so, khoang trang, '-' hoac '_'.\n";
            return false;
        }
    }

    // kiểm tra trùng mã trong cây
    if (timMH(root, mh.MAMH) != nullptr) {
        std::cout << " Loi: Ma mon hoc da ton tai.\n";
        return false;
    }

    return true;
}
bool validateSinhVien(const SinhVien &sv, std::string &msg) {
    // MASV
    if (std::strlen(sv.MASV) == 0) {
        msg = "Ma SV khong duoc rong!";
        return false;
    }
    for (const unsigned char* p = (const unsigned char*)sv.MASV; *p; ++p) {
        if (!std::isalnum(*p)) {
            msg = "Ma SV chi duoc chua chu cai va chu so!";
            return false;
        }
    }

    // HO
    if (std::strlen(sv.HO) == 0) {
        msg = "Ho khong duoc rong!";
        return false;
    }
    for (const unsigned char* p = (const unsigned char*)sv.HO; *p; ++p) {
    if (!std::isalpha(*p) && !std::isspace(*p)) {
        msg = "Ho chi duoc chua chu va khoang trang!";
        return false;
    }
}

    // TEN
    if (std::strlen(sv.TEN) == 0) {
        msg = "Ten khong duoc rong!";
        return false;
    }
    for (const unsigned char* p = (const unsigned char*)sv.TEN; *p; ++p) {
    if (!std::isalpha(*p) && !std::isspace(*p)) {
        msg = "Ten chi duoc chua chu va khoang trang!";
        return false;
    }
}
    // PHAI = Nam / Nu (khong dau, ko phan biet hoa thuong)
    std::string phai;
    for (const unsigned char* p = (const unsigned char*)sv.PHAI; *p; ++p) {
        if (std::isspace(*p)) {
            break;
        }
        phai.push_back((char)std::tolower(*p));
    }
    if (phai != "nam" && phai != "nu") {
        msg = "Phai phai la 'Nam' hoac 'Nu'!";
        return false;
    }

    // password >= 4 ký tự
    size_t lenPass = std::strlen(sv.password);
    if (lenPass < 4 || lenPass > 19) {
        msg = "Mat khau phai tu 4–19 ky tu!";
        return false;
    }

    msg.clear();
    return true;
}


bool validateCauHoi(const CauHoi &ch, std::string &msg) {
    if (std::strlen(ch.NoiDung) == 0) {
        msg = "Noi dung cau hoi khong duoc rong!";
        return false;
    }
    if (std::strlen(ch.A) == 0 ||
        std::strlen(ch.B) == 0 ||
        std::strlen(ch.C) == 0 ||
        std::strlen(ch.D) == 0) {
        msg = "Tat ca dap an A, B, C, D phai duoc nhap!";
        return false;
    }

    char d = (char)std::toupper((unsigned char)ch.DapAn);
    if (d < 'A' || d > 'D') {
        msg = "Dap an phai la mot trong cac ky tu A, B, C, D!";
        return false;
    }

    msg.clear();
    return true;
}




