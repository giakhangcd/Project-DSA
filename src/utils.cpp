// utils.cpp
// Triển khai các tiện ích chung cho project

#include "utils.h"
#include "thitracnghiem.h"   // cần TREE_MH, DS_LOP và các hàm thao tác môn/lớp/SV/câu hỏi
#include "ui.h"
#include <string>
#include <vector>
#include <cstring>
#include <cctype>
#include <chrono>
#include <atomic>
#include <iostream>
#include <fstream>
#include <limits>
#include <ctime>
#include <cstdlib>
#include <conio.h>
#include <cstdio>

#ifdef _WIN32
  #include <windows.h>
  #include <direct.h>     // _mkdir
#else
  #include <sys/stat.h>   // mkdir
  #include <sys/types.h>
  #include <errno.h>
#endif

using std::string;
using std::vector;

/* ============================ NAMESPACE util ============================ */
namespace util {

    // isspace/toupper an toàn với byte âm (UTF-8)
    static inline bool is_space(unsigned char c) {
        return std::isspace(c) != 0;
    }
    static inline char to_upper(unsigned char c) {
        return static_cast<char>(std::toupper(c));
    }

    // Tạo thư mục (hỗ trợ lồng nhau). Không lỗi nếu đã tồn tại.
    void ensureDir(const string& dir) {
        if (dir.empty()) return;

    #ifdef _WIN32
        string cur;
        cur.reserve(dir.size());
        for (size_t i = 0; i < dir.size(); ++i) {
            char ch = dir[i];
            cur.push_back(ch);
            if (ch == '\\' || ch == '/') {
                if (cur.size() == 1) continue; // ví dụ "C:\"
                _mkdir(cur.c_str());
            }
        }
        _mkdir(dir.c_str());
    #else
        string cur;
        cur.reserve(dir.size());
        for (size_t i = 0; i < dir.size(); ++i) {
            char ch = dir[i];
            cur.push_back(ch);
            if (ch == '/') {
                if (cur.size() == 1) continue; // "/"
                if (mkdir(cur.c_str(), 0755) != 0 && errno != EEXIST) {
                    // bỏ qua lỗi khác
                }
            }
        }
        if (mkdir(dir.c_str(), 0755) != 0 && errno != EEXIST) {
            // bỏ qua
        }
    #endif
    }

    inline PTRCH taoNodeCH(const CauHoi& x) {
    PTRCH p = new nodeCH;
    p->ch = x;
    p->next = NULL;
    return p;
}

inline void themCuoiCauHoi(DS_CauHoi& ds, const CauHoi& x) {
    PTRCH p = taoNodeCH(x);
    if (ds.pHead == NULL) {
        ds.pHead = p;
        ds.pLast = p;
    } else {
        ds.pLast->next = p;
        ds.pLast = p;
    }
    ds.SLCH++;
}

inline PTRSV taoNodeSV(const SinhVien& x) {
    PTRSV p = new nodeSV;
    p->sv = x;
    p->next = NULL;
    return p;
}

inline void themCuoiSinhVien(DS_SinhVien& ds, const SinhVien& x) {
    PTRSV p = taoNodeSV(x);
    if (ds.pHead == NULL) {
        ds.pHead = p;
        ds.pLast = p;
    } else {
        ds.pLast->next = p;
        ds.pLast = p;
    }
    ds.SLSV++;
}

inline PTRDIEM taoNodeDiem(const DiemThi& x) {
    PTRDIEM p = new nodeDiem;
    p->dt = x;
    p->next = NULL;
    return p;
}

inline void themCuoiDiem(DS_DiemThi& ds, const DiemThi& x) {
    PTRDIEM p = taoNodeDiem(x);
    if (ds.pHead == NULL) {
        ds.pHead = p;
        ds.pLast = p;
    } else {
        ds.pLast->next = p;
        ds.pLast = p;
    }
    ds.SLDiem++;
}


    // Cắt chuỗi theo delim, giữ cả trường rỗng
    vector<string> split(const string& s, char delim) {
        vector<string> out;
        string cur;
        out.reserve(8);
        for (char ch : s) {
            if (ch == delim) {
                out.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(ch);
            }
        }
        out.push_back(cur);
        return out;
    }

    // Trim đầu-cuối
    string trim(string s) {
        size_t i = 0, j = s.size();
        while (i < j && is_space(static_cast<unsigned char>(s[i]))) ++i;
        while (j > i && is_space(static_cast<unsigned char>(s[j - 1]))) --j;
        return s.substr(i, j - i);
    }

    // Upper ASCII
    string upper(string s) {
        for (char& ch : s) ch = to_upper(static_cast<unsigned char>(ch));
        return s;
    }

    // ID ms<<10 | counter
    long long randId() {
        using namespace std::chrono;
        static std::atomic<uint16_t> counter{0};
        const uint64_t ms = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
        ).count();
        uint16_t c = counter.fetch_add(1) & 0x3FFu; // 10 bit
        return static_cast<long long>((ms << 10) | c);
    }

} // namespace util

/* ============================ GLOBAL UTILS ============================ */
// Những hàm dưới được thitracnghiem.cpp gọi trực tiếp
// Xóa màn hình console
void clearConsole() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) return;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(h, &csbi)) return;

    DWORD cellCount = static_cast<DWORD>(csbi.dwSize.X) * static_cast<DWORD>(csbi.dwSize.Y);
    DWORD count = 0;
    COORD home = { 0, 0 };

    FillConsoleOutputCharacterA(h, ' ', cellCount, home, &count);
    FillConsoleOutputAttribute(h, csbi.wAttributes, cellCount, home, &count);
    SetConsoleCursorPosition(h, home);
#else
    std::cout << "\x1b[2J\x1b[H" << std::flush; // ANSI
#endif
}

static inline bool _getline_skip_comment(std::ifstream& f, std::string& line) {
    while (std::getline(f, line)) {
        if (!line.empty() && line[0] != '#') return true;
    }
    return false;
}

static inline void _trim(std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && std::isspace((unsigned char)s[i])) ++i;
    while (j > i && std::isspace((unsigned char)s[j-1])) --j;
    s = s.substr(i, j - i);
}

// Ghi lại toàn bộ câu hỏi của 1 môn vào file data/cauhoi_<MAMH>.txt
#include <fstream>
#include <cstdio>   // remove, rename
#include <string>

bool saveCauHoiTheoMon(const char* mamh, const /*MonHoc*/ MonHoc& mh) {
    std::string path = fileCauHoiTheoMon(mamh);          // VD: "data/cauhoi_CTDL.txt"
    std::string tmp  = path + ".tmp";

    std::ofstream f(tmp.c_str(), std::ios::out);
    if (!f.is_open()) {
        return false;
    }

    // IMPORTANT: ghi TỪ RAM ra file (duyệt danh sách câu hỏi của mh)
    // Ví dụ pseudo:
    // for (CauHoi* p = mh.dsCH; p != nullptr; p = p->next) { ... f << ... << "\n"; }

    f.flush();
    f.close();

    // Đổi tên kiểu "atomic" tương đối
    std::remove(path.c_str());
    int ok = std::rename(tmp.c_str(), path.c_str());
    if (ok != 0) {
        // nếu rename fail thì đừng mất file tmp
        return false;
    }
    return true;
}

bool loadMonHoc(const char* path, TREE_MH &dsMH) {
    std::ifstream f(path);
    if (!f) return false;

    std::string line;
    while (_getline_skip_comment(f, line)) {
        // MAMH|TENMH
        std::string a, b, cur;
        int col = 0;

        for (char c : line) {
            if (c == '|') {
                if (col == 0) {
                    a = cur;
                } else if (col == 1) {
                    b = cur;
                }
                cur.clear();
                ++col;
            } else {
                cur.push_back(c);
            }
        }

        if (col == 1) {
            b = cur;
        } else {
            continue; // format sai
        }

        _trim(a);
        _trim(b);

        if (a.empty() || b.empty()) {
            continue;
        }

        MonHoc mh;
        std::memset(&mh, 0, sizeof(MonHoc));
        std::strncpy(mh.MAMH, a.c_str(), sizeof(mh.MAMH) - 1);
        std::strncpy(mh.TENMH, b.c_str(), sizeof(mh.TENMH) - 1);

        // dùng validate từ file MonHoc của bạn anh
        if (!ValidateMonHoc(mh, dsMH)) {
            // ValidateMonHoc tự in lỗi rồi, nên chỉ bỏ qua
            continue;
        }

        // Nếu hợp lệ thì chèn vào cây (dùng hàm cũ của anh)
        chenMonHoc(dsMH, mh.MAMH, mh.TENMH);
    }
    return true;
}

static void _writeMonHocInOrder(std::ofstream& out, TREE_MH t) {
    if (t == NULL) return;
    _writeMonHocInOrder(out, t->left);

    out << t->mh.MAMH << "|" << t->mh.TENMH << "\n";

    _writeMonHocInOrder(out, t->right);
}

bool saveMonHoc(const char* path, TREE_MH dsMH) {
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) return false;

    // header (tuỳ bạn có muốn hay không)
    out << "MAMH|TENMH\n";

    _writeMonHocInOrder(out, dsMH);

    out.close();
    return true;
}

bool saveSinhVien(const char* path, const DS_LOP& dsLop) {
    std::ofstream f(path, std::ios::out | std::ios::trunc);
    if (!f.is_open()) return false;

    for (int i = 0; i < dsLop.n; i++) {
        Lop* lop = dsLop.nodes[i];
        PTRSV sv = lop->dsSV.pHead;

        while (sv != NULL) {
            f << lop->MALOP << "|"
              << sv->sv.MASV << "|"
              << sv->sv.HO << "|"
              << sv->sv.TEN << "|"
              << sv->sv.PHAI << "|"
              << sv->sv.password << "\n";

            sv = sv->next;
        }
    }

    f.close();
    return true;
}



bool loadLop(const char* path, DS_LOP &dsLop) {
    std::ifstream f(path);
    if (!f) return false;
    std::string line;
    while (_getline_skip_comment(f, line)) {
        // MALOP|TENLOP
        std::string a, b, cur; int col = 0;
        for (char c : line) {
            if (c == '|') { if (col==0) a=cur; else b=cur; cur.clear(); ++col; }
            else cur.push_back(c);
        }
        if (col==1) b = cur; else continue;
        _trim(a); _trim(b);
        if (!a.empty() && !b.empty()) {
            themLop(dsLop, a.c_str(), b.c_str());
        }
    }
    return true;
}

bool loadSinhVien(const char* path, DS_LOP &dsLop) {
    std::ifstream f(path);
    if (!f) return false;

    std::string line;
    while (_getline_skip_comment(f, line)) {
        // MALOP|MASV|HO|TEN|PHAI|PASSWORD
        std::string c[6];
        std::string cur;
        int k = 0;

        for (char ch : line) {
            if (ch == '|') {
                if (k < 6) {
                    c[k] = cur;
                    ++k;
                }
                cur.clear();
            } else {
                cur.push_back(ch);
            }
        }
        if (k < 6) {
            c[k] = cur;
            ++k;
        }
        if (k != 6) {
            continue; // sai format
        }

        for (int i = 0; i < 6; i++) {
            _trim(c[i]);
        }

        // Tìm lớp theo MALOP
        Lop* lop = timLop(dsLop, c[0].c_str());
        if (lop == NULL) {
           // std::cerr << "loadSinhVien: Khong tim thay lop " << c[0]
            //          << " trong dong: " << line << "\n";
            continue;
        }

        SinhVien sv;
        std::memset(&sv, 0, sizeof(SinhVien));

        std::strncpy(sv.MASV,    c[1].c_str(), sizeof(sv.MASV)    - 1);
        std::strncpy(sv.HO,      c[2].c_str(), sizeof(sv.HO)      - 1);
        std::strncpy(sv.TEN,     c[3].c_str(), sizeof(sv.TEN)     - 1);
        std::strncpy(sv.PHAI,    c[4].c_str(), sizeof(sv.PHAI)    - 1);
        std::strncpy(sv.password,c[5].c_str(), sizeof(sv.password)- 1);
        sv.dsDiem.pHead  = NULL;
        sv.dsDiem.pLast  = NULL;
        sv.dsDiem.SLDiem = 0;

        // validate giống bản mạnh của bạn anh
        std::string msg;
        if (!validateSinhVien(sv, msg)) {
            //std::cerr << "loadSinhVien: Sinh vien khong hop le (" << line
            //          << "): " << msg << "\n";
            continue;
        }

        // Thêm vào lớp: tạm thời vẫn dùng hàm cũ của anh
        // Sau này mình sẽ nâng cấp themSVVaoLop để nó sắp xếp MASV.
        themSVVaoLop(lop, sv);
    }
    return true;
}

static std::string safeMamh(const char* mamh){
    if(mamh == nullptr) return "UNKNOWN";
    return std::string(mamh);
}

// Ví dụ: data/cauhoi_<MAMH>.txt
std::string fileCauHoiTheoMon(const char* mamh){
    std::string m = safeMamh(mamh);

    // Nếu anh muốn lọc ký tự lạ (space, /, \, :) thì có thể thay thế ở đây
    // cho đơn giản: giả sử MAMH không có ký tự đặc biệt

    return "data/cauhoi_" + m + ".txt";
}

bool loadCauHoi(const char* path, TREE_MH &dsMH) {
    std::ifstream f(path);
    if (!f) return false;

    std::string line;
    while (_getline_skip_comment(f, line)) {
        // MAMH|ID|NOIDUNG|A|B|C|D|DAPAN
        std::string c[8];
        std::string cur;
        int k = 0;

        for (char ch : line) {
            if (ch == '|') {
                if (k < 8) {
                    c[k] = cur;
                    ++k;
                }
                cur.clear();
            } else {
                cur.push_back(ch);
            }
        }
        if (k < 8) {
            c[k] = cur;
            ++k;
        }
        if (k != 8) {
            continue; // sai format
        }

        for (int i = 0; i < 8; i++) {
            _trim(c[i]);
        }

        // Tìm môn học
        nodeMH* p = timMH(dsMH, c[0].c_str());
        if (p == NULL) {
           // std::cerr << "loadCauHoi: Khong tim thay mon hoc " << c[0]
           //           << " trong dong: " << line << "\n";
            continue;
        }

        CauHoi q;
        std::memset(&q, 0, sizeof(CauHoi));

        try {
            q.ID = std::stoi(c[1]);
        } catch (...) {
            //std::cerr << "loadCauHoi: ID khong hop le trong dong: " << line << "\n";
            continue;
        }

        std::strncpy(q.NoiDung, c[2].c_str(), sizeof(q.NoiDung) - 1);
        std::strncpy(q.A,       c[3].c_str(), sizeof(q.A)       - 1);
        std::strncpy(q.B,       c[4].c_str(), sizeof(q.B)       - 1);
        std::strncpy(q.C,       c[5].c_str(), sizeof(q.C)       - 1);
        std::strncpy(q.D,       c[6].c_str(), sizeof(q.D)       - 1);

        if (!c[7].empty()) {
            q.DapAn = c[7][0];
        } else {
            q.DapAn = 'A';
        }

        std::string msg;
        if (!validateCauHoi(q, msg)) {
            //std::cerr << "loadCauHoi: Cau hoi khong hop le (" << line
             //         << "): " << msg << "\n";
            continue;
        }

        themCauHoi(p->mh, q);
    }
    return true;
}

void giaiPhongDanhSachCauHoi(MonHoc& mh) {
    PTRCH p = mh.dsCH.pHead;
    while (p != NULL) {
        PTRCH x = p;
        p = p->next;
        delete x;
    }
    mh.dsCH.pHead = NULL;
    mh.dsCH.pLast = NULL;
    mh.dsCH.SLCH  = 0;
}


// Tuỳ dùng: MASV|MAMH|DIEM — để trống cài đặt nếu project chưa có cấu trúc điểm
bool loadDiemThi(const char* path, DS_LOP &dsLop, TREE_MH &dsMH) {
    std::ifstream f(path);
    if (!f) return false;
    std::string line;
    while (_getline_skip_comment(f, line)) {
        // TODO: nếu có struct DiemThi + hàm thêm điểm thì parse ở đây rồi gán cho SV tương ứng
        // Hiện tại bỏ qua để không ảnh hưởng build.
    }
    return true;
}


static short cursorXAtEnd(const char* s, short xStart, int maxLen) {
    int n = (int)strlen(s);
    if (n > maxLen - 1) n = maxLen - 1;
    return (short)(xStart + n);
}


static void clearInputArea(short x, short y, int len) {
    gotoXY(x, y);
    for (int i = 0; i < len; i++) std::cout << ' ';
    gotoXY(x, y);
}

void inputField(char* out, int maxLen, short x, short y, bool passwordMode, bool active, bool showPass) {
    // vẽ lại ô input theo trạng thái active
    if (active) setColor(240);
    else setColor(7);

    clearInputArea(x, y, maxLen - 1);

    int n = (int)strlen(out);
    gotoXY(x, y);
    if (passwordMode) {
        if (showPass)
            std::cout << out;
        else
            for (int i = 0; i < n;i++)
                std::cout << "*";
    } else {
            std::cout << out;
    }
    setColor(7);
}


bool loginForm(char* user, int userMax, char* pass, int passMax, short boxX, short boxY) {
    user[0] = '\0';
    pass[0] = '\0';

    const short ux = boxX + 16;
    const short uy = boxY + 3;
    const short px = boxX + 16;
    const short py = boxY + 5;

    int field = 0; // 0=user, 1=pass
    bool showPass = false;

    while (true) {
        // render 2 field
        inputField(user, userMax, ux, uy, false, (field == 0), true);
        inputField(pass, passMax, px, py, true,  (field == 1), showPass);

        // đặt con trỏ nháy đúng ô active
        if (field == 0) {
        short cx = cursorXAtEnd(user, ux, userMax);
        gotoXY(cx, uy);
        } 
        else {
            short cx = cursorXAtEnd(pass, px, passMax);
            gotoXY(cx, py);
            }


        int key = readKey();

        // ESC
        if (key == 27) return false;

        if(key == 15 ){
            showPass = !showPass;
            continue;
        }

        // UP / DOWN
        if (key == 1000 + 72) { // Up
            if (field == 1) field = 0;
            continue;
        }
        if (key == 1000 + 80) { // Down
            if (field == 0) field = 1;
            continue;
        }

        // ENTER
        if (key == 13) {
            if (field == 0) field = 1;
            else return true; // enter ở password => submit
            continue;
        }

        // BACKSPACE
        if (key == 8) {
            if (field == 0) {
                int n = (int)strlen(user);
                if (n > 0) user[n - 1] = '\0';
            } else {
                int n = (int)strlen(pass);
                if (n > 0) pass[n - 1] = '\0';
            }
            continue;
        }
        

        // Ký tự thường (chặn ký tự không in được)
        if (key >= 32 && key <= 126) {
            if (field == 0) {
                int n = (int)strlen(user);
                if (n < userMax - 1) {
                    user[n] = (char)key;
                    user[n + 1] = '\0';
                }
            } else {
                int n = (int)strlen(pass);
                if (n < passMax - 1) {
                    pass[n] = (char)key;
                    pass[n + 1] = '\0';
                }
            }
            continue;
        }

        // Các phím khác bỏ qua để không làm bẩn input
    }
}

