#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <conio.h>
#include "utils.h"
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

using namespace std;

PTRCH taoNodeCH(const CauHoi& x) {
    PTRCH p = new nodeCH;
    p->ch = x;
    p->next = NULL;
    return p;
}

bool idTonTai(PTRCH head, int id){
    nodeCH* p = head;
    while(p){
        if(p->ch.ID == id) return true;
        p = p->next;
    }
    return false;
}

void themCauHoi(MonHoc &mh, const CauHoi &ch) {
    PTRCH p = new nodeCH;
    p->ch = ch;
    p->next = NULL;

    if (mh.dsCH.pHead == NULL) {
        mh.dsCH.pHead = p;
        mh.dsCH.pLast = p;
    } else {
        mh.dsCH.pLast->next = p;
        mh.dsCH.pLast = p;
    }
    mh.dsCH.SLCH++;
}


bool suaCauHoi(MonHoc &mh, int id) {
    nodeCH* p = mh.dsCH.pHead;
    while (p) {
        if (p->ch.ID == id) {
            CauHoi backup = p->ch;   // để rollback

            if (!formSuaCauHoi(p->ch)) {
                p->ch = backup;      // ESC → không sửa
                return false;
            }
            return true;             // đã sửa trong RAM
        }
        p = p->next;
    }
    return false;
}


bool daCoDiemMonTrongLop(const DS_LOP& ds, const char* mamh){
    for(int i = 0; i < ds.n; i = i + 1){
        PTRSV p = ds.nodes[i]->dsSV.pHead;
        while(p){
            PTRDIEM d = p->sv.dsDiem.pHead;
            while(d){
                if(strcmp(d->dt.MAMH, mamh) == 0) return true;
                d = d->next;
            }
            p = p->next;
        }
    }
    return false;
}

void ghiLaiFileCauHoi(const MonHoc& mh) {
    std::string path = fileCauHoiTheoMon(mh.MAMH);
    std::ofstream f(path, std::ios::out | std::ios::trunc);
    if (!f.is_open()) return;

    PTRCH p = mh.dsCH.pHead;
    while (p != NULL) {
        const CauHoi& q = p->ch;
        f << mh.MAMH << '|'
          << q.ID << '|'
          << q.NoiDung << '|'
          << q.A << '|'
          << q.B << '|'
          << q.C << '|'
          << q.D << '|'
          << q.DapAn << '\n';
        p = p->next;
    }
    f.close();
}

bool xoaCauHoi(MonHoc &mh, int id, const DS_LOP& ds, const char* mamh){
    if(daCoDiemMonTrongLop(ds, mamh)){
        cout << "Khong the xoa: Mon '" << mamh << "' da co sinh vien thi.\n";
        return false;
    }

    nodeCH* prev = NULL;
    nodeCH* cur = mh.dsCH.pHead;

    while(cur){
        if(cur->ch.ID == id){
            if(prev == NULL){
                mh.dsCH.pHead = cur->next;
                if(mh.dsCH.pHead == NULL)
                    mh.dsCH.pLast = NULL;
            } else {
                prev->next = cur->next;
                if(cur == mh.dsCH.pLast)
                    mh.dsCH.pLast = prev;
            }
            delete cur;
            mh.dsCH.SLCH--;
            ghiLaiFileCauHoi(mh);
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}


int demCauHoi(PTRCH head){
    int c=0; nodeCH* p=head; while(p){ c++; p=p->next; } return c;
}

void inDanhSachCauHoi(const MonHoc &mh){
    nodeCH* p = mh.dsCH.pHead; int stt=1;
    while(p){
        cout << "----- Cau " << stt++ << " (ID " << p->ch.ID << ") -----\n";
        cout << p->ch.NoiDung << "\n";
        cout << "A. " << p->ch.A << "\n";
        cout << "B. " << p->ch.B << "\n";
        cout << "C. " << p->ch.C << "\n";
        cout << "D. " << p->ch.D << "\n";
        cout << "Dap an: " << p->ch.DapAn << "\n";
        p = p->next;
    }
}

void themCuoiCH(DS_CauHoi& ds, const CauHoi& x) {
    PTRCH n = taoNodeCH(x);
    if (ds.pHead == NULL) {
        ds.pHead = n;
        ds.pLast = n;
    } else {
        ds.pLast->next = n;
        ds.pLast = n;
    }
    ds.SLCH = ds.SLCH + 1;
}
