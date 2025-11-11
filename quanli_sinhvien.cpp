#include <iostream>
#include "include/khai_bao.h"
#include <cstring>
#include <string>
#include <cctype>

using namespace std;

nodeSV::nodeSV(SinhVien SV) {
    sv = SV;
    next = NULL; 
}

// ===== Helpers ASCII-only =====
static inline bool is_all_space(const char* s, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (s[i] == '\0') break;
        if (!std::isspace(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}

static inline bool has_leading_or_trailing_space(const char* s) {
    if (*s == '\0') return false;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
    if (std::isspace(*p)) return true;
    size_t len = std::strlen(s);
    return len > 0 && std::isspace(static_cast<unsigned char>(s[len - 1]));
}

static inline bool has_consecutive_spaces(const char* s) {
    bool prev_space = false;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        bool sp = std::isspace(*p);
        if (sp && prev_space) return true;
        prev_space = sp;
    }
    return false;
}

static inline bool equals_gender_ascii_relaxed(const char* phai) {
    // so sánh "Nam"/"Nu" không phân biệt hoa/thường, không dấu, bỏ khoảng trắng hai đầu
    while (*phai && std::isspace((unsigned char)*phai)) ++phai;
    std::string t;
    for (const unsigned char* p = (const unsigned char*)phai; *p; ++p) {
        if (std::isspace(*p)) break;
        t.push_back((char)std::tolower(*p));
    }
    return (t == "nam" || t == "nu");
}

// độ dài an toàn trong buffer char[]
static inline size_t safe_len(const char* a, size_t cap, bool &null_terminated) {
    const char* end = (const char*)memchr(a, '\0', cap);
    if (end) { null_terminated = true; return size_t(end - a); }
    null_terminated = false; return cap;
}

// chỉ chữ cái + số (ASCII)
static inline bool is_alnum_ascii_str(const char* s) {
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (!std::isalnum(*p)) return false;
    }
    return true;
}

// chỉ chữ cái + khoảng trắng (ASCII, không dấu)
static inline bool is_alpha_space_ascii_str(const char* s) {
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (!(std::isalpha(*p) || std::isspace(*p))) return false;
    }
    return true;
}

// ===== Validator chính =====
bool validateSinhVien(const SinhVien &sv, std::string &msg) {
    const size_t capMASV = sizeof sv.MASV;
    const size_t capHO   = sizeof sv.HO;
    const size_t capTEN  = sizeof sv.TEN;
    const size_t capPHAI = sizeof sv.PHAI;
    const size_t capPASS = sizeof sv.password;

    bool nt = true;

    size_t lenMASV = safe_len(sv.MASV, capMASV, nt);
    if (!nt) { msg = "MASV qua dai/khong co \\0"; return false; }
    if (lenMASV == 0 || is_all_space(sv.MASV, lenMASV)) { msg = "Ma SV khong duoc rong!"; return false; }
    if (has_leading_or_trailing_space(sv.MASV)) { msg = "Ma SV khong duoc co khoang trang o dau/cuoi!"; return false; }
    if (!is_alnum_ascii_str(sv.MASV)) { msg = "Ma SV chi duoc chua chu cai va chu so (ASCII)!"; return false; }

    size_t lenHO = safe_len(sv.HO, capHO, nt);
    if (!nt) { msg = "HO qua dai/khong co \\0"; return false; }
    if (lenHO == 0 || is_all_space(sv.HO, lenHO)) { msg = "Ho khong hop le!"; return false; }
    if (has_leading_or_trailing_space(sv.HO) || has_consecutive_spaces(sv.HO)) {
        msg = "Ho khong duoc co khoang trang thua!"; return false;
    }
    if (!is_alpha_space_ascii_str(sv.HO)) { msg = "Ho chi duoc chua chu cai va khoang trang (khong dau)!"; return false; }

    size_t lenTEN = safe_len(sv.TEN, capTEN, nt);
    if (!nt) { msg = "TEN qua dai/khong co \\0"; return false; }
    if (lenTEN == 0 || is_all_space(sv.TEN, lenTEN)) { msg = "Ten khong hop le!"; return false; }
    if (has_leading_or_trailing_space(sv.TEN) || has_consecutive_spaces(sv.TEN)) {
        msg = "Ten khong duoc co khoang trang thua!"; return false;
    }
    if (!is_alpha_space_ascii_str(sv.TEN)) { msg = "Ten chi duoc chua chu cai va khoang trang (khong dau)!"; return false; }

    size_t lenPHAI = safe_len(sv.PHAI, capPHAI, nt);
    if (!nt) { msg = "Phai qua dai/khong co \\0"; return false; }
    if (lenPHAI == 0) { msg = "Phai khong duoc rong!"; return false; }
    if (!equals_gender_ascii_relaxed(sv.PHAI)) {
        msg = "Phai phai la 'Nam' hoac 'Nu' (khong dau, khong phan biet hoa/thuong)!"; return false;
    }

    size_t lenPASS = safe_len(sv.password, capPASS, nt);
    if (!nt) { msg = "Mat khau qua dai/khong co \\0"; return false; }
    if (lenPASS < 4 || lenPASS > 19) { msg = "Mat khau phai tu 4-19 ky tu!"; return false; }
    for (size_t i = 0; i < lenPASS; ++i) {
        unsigned char c = (unsigned char)sv.password[i];
        if (std::iscntrl(c)) { msg = "Mat khau khong duoc chua ky tu control!"; return false; }
    }
    // (Nếu muốn: cấm khoảng trắng trong mật khẩu)
    // for (size_t i = 0; i < lenPASS; ++i) if (std::isspace((unsigned char)sv.password[i])) { msg = "Mat khau khong duoc co khoang trang!"; return false; }

    return true;
}

PTRSV TimSinhVien(PTRSV first, const char* maSV) {
    for (PTRSV p = first; p != NULL; p = p->next) {
        if (strcmp(p->sv.MASV, maSV) == 0) {
            return p;
        }
    } 
    return NULL;
}

bool ThemSinhVien(PTRSV svmoi, Lop* lop, DS_LOP &ds) {

    if (TimLop(ds, lop->MALOP) == -1) {
        cout << "lop khong ton tai";
        return false;
    }

    // Nếu danh sách rỗng → thêm đầu
    if (lop->FirstSV == NULL) {
        lop->FirstSV = svmoi;
        return true;
    }

    int cmpHead = strcmp(svmoi->sv.MASV, lop->FirstSV->sv.MASV);
    if (cmpHead < 0) {
        svmoi->next = lop->FirstSV;
        lop->FirstSV = svmoi;
        return true;
    } else if (cmpHead == 0) {
        cout << "Ma sinh vien da ton tai!\n";
        return false;
    }
    
    // Duyệt để tìm vị trí chèn
    PTRSV truoc = lop->FirstSV;
    PTRSV sau = lop->FirstSV->next;

    while (sau != NULL && strcmp(svmoi->sv.MASV, sau->sv.MASV) > 0) {
        truoc = sau;
        sau = sau->next;
    }

    // Nếu MASV đã tồn tại → không thêm
    if (sau != NULL && strcmp(sau->sv.MASV, svmoi->sv.MASV) == 0) {
        cout << "Ma sinh vien da ton tai!\n";
        return false;
    }

    // Chèn svMoi vào giữa truoc và sau
    svmoi->next = sau;
    truoc->next = svmoi;

    return true;
}

bool XoaSinhVien(PTRSV &first, const char* maSV) {
    PTRSV p = TimSinhVien(first, maSV);
    if (p == NULL) {
        cout << "Khong tim thay sinh vien!" << endl;
        return false;
    }

    if (p == first) {
        first = first->next;
        delete p;
        cout << "Da xoa sinh vien" << endl;
        return true;
    }

    PTRSV prev = first;
    while (prev->next != NULL && prev->next != p) {
        prev = prev->next;
    }

    if (prev->next == p) {
        prev->next = p->next;
        delete p;
        cout << "Da xoa sinh vien" << endl;
        return true;
    }
    
    return false; 
}

