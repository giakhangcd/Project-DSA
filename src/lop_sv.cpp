#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "thitracnghiem.h"
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

PTRSV taoNodeSV(const SinhVien& x) {
    PTRSV p = new nodeSV;
    p->sv = x;
    p->next = NULL;
    return p;
}

void initDSLop(DS_LOP &ds){ ds.n = 0; }

bool themLop(DS_LOP &ds, const char* malop, const char* tenlop){
    if(ds.n >= MAX_LOP) return false;
    for(int i=0;i<ds.n;i++){
        if(strcmp(ds.nodes[i]->MALOP, malop) == 0){
            strcpy(ds.nodes[i]->TENLOP, tenlop);
            return true;
        }
    }
    Lop* l = new Lop;
    strcpy(l->MALOP, malop);
    strcpy(l->TENLOP, tenlop);
    l->dsSV.pHead = NULL;
    l->dsSV.pLast = NULL;
    l->dsSV.SLSV  = 0;

    ds.nodes[ds.n++] = l;
    return true;
}

Lop* timLop(DS_LOP &ds, const char* malop){
    for(int i=0;i<ds.n;i++){
        if(strcmp(ds.nodes[i]->MALOP, malop) == 0) return ds.nodes[i];
    }
    return NULL;
}

bool themSVVaoLop(Lop* lop, const SinhVien &sv){
    if(lop == NULL) return false;

    // không trùng MASV
    PTRSV p = lop->dsSV.pHead;
    while(p){
        if(strcmp(p->sv.MASV, sv.MASV) == 0) return false;
        p = p->next;
    }

    themCuoiSV(lop->dsSV, sv);
    return true;
}

void toUpperCase(char* s) {
    for (int i = 0; s[i]; i++) {
        s[i] = (char)toupper((unsigned char)s[i]);
    }
}


SinhVien* dangNhap(DS_LOP &dsLop, const char* u, const char* p, Lop** lopOut) {
    char userUpper[32];
    strcpy(userUpper, u);
    toUpperCase(userUpper);

    for (int i = 0; i < dsLop.n; i++) {
        Lop* lop = dsLop.nodes[i];
        PTRSV sv = lop->dsSV.pHead;

        while (sv != NULL) {
            if (strcmp(sv->sv.MASV, userUpper) == 0 &&
                strcmp(sv->sv.password, p) == 0) {

                if (lopOut) *lopOut = lop;
                return &sv->sv;
            }
            sv = sv->next;
        }
    }
    return NULL;
}


void themCuoiSV(DS_SinhVien& ds, const SinhVien& x) {
    PTRSV n = taoNodeSV(x);
    if (ds.pHead == NULL) {
        ds.pHead = n;
        ds.pLast = n;
    } else {
        ds.pLast->next = n;
        ds.pLast = n;
    }
    ds.SLSV = ds.SLSV + 1;
}