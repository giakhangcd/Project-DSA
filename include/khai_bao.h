#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <windows.h>
#include <conio.h>
#include <cctype>

using namespace std;

struct CauHoi {
    int ID;            
    char NoiDung[256]; 
    char A[100], B[100], C[100], D[100]; 
    char DapAn;        
};

struct nodeCH {
    CauHoi ch;
    nodeCH *next;
    nodeCH(CauHoi x);
};
typedef nodeCH* PTRCH;

struct MonHoc {
    char MAMH[16];     
    char TENMH[51];        
	PTRCH Firstch = NULL;
    friend bool operator <(const MonHoc& x, const MonHoc& y); 
    friend bool operator >(const MonHoc& x, const MonHoc& y); 
    friend bool operator ==(const MonHoc& x, const MonHoc& y);
};

struct nodeMH {
    MonHoc mh;
    int height;   
    nodeMH *left, *right;
    nodeMH(MonHoc x);
};
typedef nodeMH* TREE_MH;

struct DiemThi {
    char MAMH[16];
    float diem;
};

struct nodeDiem {
    DiemThi dt;
    nodeDiem *next;
};
typedef nodeDiem* PTRDIEM;

struct SinhVien {
    char MASV[16];
    char HO[51];
    char TEN[50];
    char PHAI[4];
    char password[20];
    PTRDIEM dsDiemThi = NULL; 
};

struct nodeSV {
    SinhVien sv;
    nodeSV* next;
    nodeSV(SinhVien SV);
};
typedef nodeSV* PTRSV;

const int MAX_LOP = 10001;
struct Lop {
    char MALOP[16];
    char TENLOP[51];
    PTRSV FirstSV = NULL; 
};

struct DS_LOP {
    int n = 0;
    Lop* nodes[MAX_LOP];
};

// khai báo hàm

//  quản lí lớp
int TimLop(DS_LOP &ds, const char *maLop);
bool ThemLop(DS_LOP &ds, Lop *lopMoi);
bool XoaLop(DS_LOP &ds, const char *maLop);
void Xuatdanhsachlop(DS_LOP &ds);
Lop* NhapLop();
void GiaiPhongLop(DS_LOP &ds);

// quản lí sinh viên
bool ThemSinhVien(PTRSV svMoi, Lop* lop, DS_LOP &ds);
PTRSV TimSinhVien(PTRSV first, const char* maSV);
bool isAlphaNum(const char* s);
bool isAlphaSpace(const char* s);
bool validateSinhVien(const SinhVien &sv, string &msg);

// quản lí câu hỏi
bool validateCauHoi(const CauHoi &ch, string &msg);
PTRCH khoiTaoDanhSach(PTRCH first);
bool themCauHoi(PTRCH &first, const CauHoi &ch);
PTRCH timCauHoi(PTRCH first, int id);
bool xoaCauHoi(PTRCH &first, int id);
void hienThiDanhSachCauHoi(PTRCH first);

// quản lí môn học 
TREE_MH minnode(TREE_MH root);
TREE_MH deletenode(TREE_MH root, MonHoc key);
int getheight(TREE_MH root);
int getbalance(TREE_MH root);
void updateHeight(TREE_MH root);
TREE_MH leftRotate(TREE_MH x);
TREE_MH rightRotate(TREE_MH x);
TREE_MH themMonHoc(TREE_MH root, MonHoc key);
void ChuanHoaTen(char* s);
bool ValidateMonHoc(const MonHoc& mh, TREE_MH root);