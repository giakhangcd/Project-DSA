#pragma once
#include"utils.h"
#include <vector>
#include <iostream>
#include <cstring>
#include <chrono>
struct CauHoi {
    int ID;
    char NoiDung[256];
    char A[100], B[100], C[100], D[100];
    char DapAn;
    bool daThi; // giống nhóm kia (nếu anh cần)
};

struct nodeCH {
    CauHoi ch;
    nodeCH* next;
};

typedef nodeCH* PTRCH;

struct DS_CauHoi {
    int SLCH;
    PTRCH pHead;
    PTRCH pLast;

    DS_CauHoi() {
        SLCH = 0;
        pHead = NULL;
        pLast = NULL;
    }
};

// ======================= MON HOC (BST) =======================
struct MonHoc {
    char MAMH[16];
    char TENMH[51];
    int height;      // giữ theo đề (không dùng AVL cũng OK)
    DS_CauHoi dsCH;  // thay vì chỉ Firstcht

    MonHoc() {
        MAMH[0] = '\0';
        TENMH[0] = '\0';
        height = 1;
    }
};

struct nodeMH {
    MonHoc mh;
    nodeMH* left;
    nodeMH* right;

    nodeMH() {
        left = NULL;
        right = NULL;
    }
};

typedef nodeMH* TREE_MH;

struct MonLop {
    char MAMH[16];   // mã môn
    int  soLop;      // số lớp được phép thi
    char** dsLop;    // mảng con trỏ: mỗi phần tử là 1 MALOP
};

// ======================= DIEM THI =======================
struct DiemThi {
    char MAMH[16];
    float diem;
};

struct nodeDiem {
    DiemThi dt;
    nodeDiem* next;
};

typedef nodeDiem* PTRDIEM;

struct DS_DiemThi {
    int SLDiem;
    PTRDIEM pHead;
    PTRDIEM pLast;

    DS_DiemThi() {
        SLDiem = 0;
        pHead = NULL;
        pLast = NULL;
    }
};

// ======================= SINH VIEN =======================
struct SinhVien {
    char MASV[16];
    char HO[51];
    char TEN[11];
    char PHAI[4];
    char password[20];
    DS_DiemThi dsDiem; // thay cho PTRDIEM dsDiemThi

    SinhVien() {
        MASV[0] = '\0';
        HO[0] = '\0';
        TEN[0] = '\0';
        PHAI[0] = '\0';
        password[0] = '\0';
    }
};

struct nodeSV {
    SinhVien sv;
    nodeSV* next;
};

typedef nodeSV* PTRSV;

struct DS_SinhVien {
    int SLSV;
    PTRSV pHead;
    PTRSV pLast;

    DS_SinhVien() {
        SLSV = 0;
        pHead = NULL;
        pLast = NULL;
    }
};

// ======================= LOP + DS LOP (MANG CON TRO) =======================
const int MAX_LOP = 10000;

struct Lop {
    char MALOP[16];
    char TENLOP[51];
    DS_SinhVien dsSV;

    Lop() {
        MALOP[0] = '\0';
        TENLOP[0] = '\0';
    }
};

struct DS_LOP {
    int n;
    Lop* nodes[MAX_LOP];

    DS_LOP() {
        n = 0;
        for (int i = 0; i < MAX_LOP; i++) {
            nodes[i] = NULL;
        }
    }
};

struct FooterGrid {
    short x, y;
    short cellW;
    short cols;
};

// ====== Khai báo hàm ======
void khoiTaoTree(TREE_MH &t);
nodeMH* taoNodeMH(const char* mamh, const char* tenmh);
void chenMonHoc(TREE_MH &t, const char* mamh, const char* tenmh);
nodeMH* timMH(TREE_MH t, const char* mamh);
bool xoaMonHoc(TREE_MH &t, const char* mamh); // chỉ xoá khi MH không có câu hỏi và không ai đã thi
bool lopDuocThiMon(const char* mamh, const char* malop);

// Câu hỏi
bool idTonTai(PTRCH head, int id);
bool daCoDiemMonTrongLop(const DS_LOP& ds, const char* mamh); // hỗ trợ báo cáo
void themCauHoi(MonHoc &mh, const CauHoi &ch);
bool suaCauHoi(MonHoc &mh, int id);
bool xoaCauHoi(MonHoc &mh, int id, const DS_LOP& ds, const char* mamh);
int  demCauHoi(PTRCH head);
void inDanhSachCauHoi(const MonHoc &mh);
char lamMotCauHoi(
    const CauHoi &q,
    int tongGiay,
    std::chrono::steady_clock::time_point batDau
);
void inCauHoiChon(const CauHoi &q, int chiSoLuaChon);
void drawAnswerLine(const char* label, const char* nd, int y, bool highlight);
int  chonDapAnBangMuiTen(const CauHoi &q, int yStart, char daChonCu);

// Lớp & SV
void initDSLop(DS_LOP &ds);
bool themLop(DS_LOP &ds, const char* malop, const char* tenlop);
bool saveLop(const char* path, const DS_LOP& ds);

Lop* timLop(DS_LOP &ds, const char* malop);
bool themSVVaoLop(Lop* lop, const SinhVien &sv);
SinhVien* dangNhap(DS_LOP &ds, const char* masv_or_gv, const char* pass, Lop** outLop);
void themCuoiSV(DS_SinhVien& ds, const SinhVien& x);

// Thi
bool svDaThiMon(const SinhVien& sv, const char* mamh);
bool xacNhanThiMon(const MonHoc& mh, int& soCau, int& soPhut);
bool luuDiem(SinhVien &sv, const char* mamh, float diem);
float thiTracNghiem(MonHoc &mh, int soCau, int soPhut,const char* masv, const char* mamh); // rút ngẫu nhiên, shuffle Fisher–Yates

// Báo cáo
void inDSLop(const DS_LOP &ds);
void inTinhTrangThiTheoLop(const DS_LOP &ds, const char* mamh);
void luuKetQuaThi(const SinhVien &sv, const char* mamh, int diem, int thoiGian);
void docCauHoiChoMotMon(MonHoc &mh);
void docFileMonLop();
void giaiPhongMonLop();

// ===== CAU HOI THEO MON =====
void giaiPhongDanhSachCauHoi(MonHoc &mh);
void loadCauHoiTheoMon(MonHoc &mh);
void themCuoiCH(DS_CauHoi& ds, const CauHoi& x);

int menuArrowGV();