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

MonLop* g_arrMonLop = NULL;   // mảng các môn
int     g_soMonLop  = 0;      // số môn trong mảng

const int MAX_MON_LOP      = 50;
const int MAX_LOP_PER_MON  = 50;

// (tuỳ thích) hàm giải phóng, gọi khi kết thúc chương trình
void giaiPhongMonLop() {
    if (g_arrMonLop == NULL) {
        return;
    }

    for (int i = 0; i < g_soMonLop; i = i + 1) {
        if (g_arrMonLop[i].dsLop != NULL) {
            for (int j = 0; j < g_arrMonLop[i].soLop; j = j + 1) {
                delete[] g_arrMonLop[i].dsLop[j];
            }
            delete[] g_arrMonLop[i].dsLop;
        }
    }

    delete[] g_arrMonLop;
    g_arrMonLop = NULL;
    g_soMonLop  = 0;
}


// Cắt khoảng trắng đầu/cuối chuỗi (in-place)
static void trimSpaces(char* s) {
    // bỏ khoảng trắng đầu
    char* start = s;
    while (*start && std::isspace((unsigned char)*start)) {
        start++;
    }

    // tìm cuối
    char* end = start + std::strlen(start);
    while (end > start && std::isspace((unsigned char)*(end - 1))) {
        end--;
    }

    *end = '\0'; // kết thúc chuỗi mới

    // nếu start != s thì dồn chuỗi về đầu
    if (start != s) {
        std::memmove(s, start, end - start + 1);
    }
}


void docFileMonLop() {
    giaiPhongMonLop();

    g_arrMonLop = new MonLop[MAX_MON_LOP];
    g_soMonLop  = 0;

    ifstream f("data/mon_lop.txt");
    if (!f.is_open()) {
        cout << "Khong mo duoc file data/mon_lop.txt\n";
        return;
    }

    char line[256];

    while (f.getline(line, sizeof(line))) {
        if (line[0] == '\0') continue;
        if (g_soMonLop >= MAX_MON_LOP) break;

        char* token = strtok(line, "|");
        if (token == NULL) continue;

        MonLop& ml = g_arrMonLop[g_soMonLop];

        // MAMH
        trimSpaces(token);
        std::strncpy(ml.MAMH, token, 15);
        ml.MAMH[15] = '\0';

        ml.soLop = 0;
        ml.dsLop = new char*[MAX_LOP_PER_MON];

        // các MALOP
        token = strtok(NULL, "|");
        while (token != NULL) {
            if (ml.soLop >= MAX_LOP_PER_MON) break;

            trimSpaces(token);

            ml.dsLop[ml.soLop] = new char[16];
            std::strncpy(ml.dsLop[ml.soLop], token, 15);
            ml.dsLop[ml.soLop][15] = '\0';

            ml.soLop++;
            token = strtok(NULL, "|");
        }

        g_soMonLop++;
    }

    f.close();
}


// tra xem 1 lop co duoc thi 1 mon hay khong
bool lopDuocThiMon(const char* mamh, const char* malop) {
    for (int i = 0; i < g_soMonLop; i = i + 1) {
        if (strcmp(g_arrMonLop[i].MAMH, mamh) == 0) {
            for (int j = 0; j < g_arrMonLop[i].soLop; j = j + 1) {
                if (strcmp(g_arrMonLop[i].dsLop[j], malop) == 0) {
                    return true;
                }
            }
            // tim dung mon, nhung khong co lop nay
            return false;
        }
    }
    // khong tim thay mon trong file rang buoc -> KHONG cho thi
    return false;
}



void khoiTaoTree(TREE_MH &t){ t = NULL; }

nodeMH* taoNodeMH(const char* mamh, const char* tenmh){
    nodeMH* p = new nodeMH;
    strcpy(p->mh.MAMH, mamh);
    strcpy(p->mh.TENMH, tenmh);
    p->mh.height = 1;
    p->mh.dsCH.SLCH = 0;
    p->mh.dsCH.pHead = NULL;
    p->mh.dsCH.pLast = NULL;

    p->mh.soCauThi = 0;
    p->mh.soPhutThi = 0;

    p->left = p->right = NULL;
    return p;
}

void chenMonHoc(TREE_MH &t, const char* mamh, const char* tenmh){
    if(t == NULL){
        t = taoNodeMH(mamh, tenmh);
    }else{
        if(strcmp(mamh, t->mh.MAMH) < 0){
            chenMonHoc(t->left, mamh, tenmh);
        }else if(strcmp(mamh, t->mh.MAMH) > 0){
            chenMonHoc(t->right, mamh, tenmh);
        }else{
            strcpy(t->mh.TENMH, tenmh); // trùng mã -> cập nhật tên
        }
    }
}

nodeMH* timMH(TREE_MH t, const char* mamh){
    if(t == NULL) return NULL;
    int cmp = strcmp(mamh, t->mh.MAMH);
    if(cmp == 0) return t;
    if(cmp < 0) return timMH(t->left, mamh);
    return timMH(t->right, mamh);
}

static nodeMH* minNode(nodeMH* t){
    nodeMH* cur = t;
    while(cur && cur->left) cur = cur->left;
    return cur;
}

bool xoaMonHoc(TREE_MH &t, const char* mamh){
    if(t == NULL) return false;
    int cmp = strcmp(mamh, t->mh.MAMH);
    if(cmp < 0) return xoaMonHoc(t->left, mamh);
    if(cmp > 0) return xoaMonHoc(t->right, mamh);

    // kiểm tra không có câu hỏi
    if(t->mh.dsCH.pHead != NULL){
        cout << "Khong the xoa mon hoc: dang co cau hoi.\n";
        return false;
    }
    // (Tuỳ đề, có thể ràng buộc thêm: nếu có ai đã thi mon này thì cũng không xoá.
    // Ở đây ta duy trì ràng buộc này ở cấp xoá câu hỏi, còn xoá MH chỉ check câu hỏi.)

    if(t->left == NULL && t->right == NULL){
        delete t;
        t = NULL;
        return true;
    }
    else if(t->left == NULL){
        nodeMH* r = t->right; delete t; t = r; return true;
    }
    else if(t->right == NULL){
        nodeMH* l = t->left; delete t; t = l; return true;
    }
    else{
        nodeMH* m = minNode(t->right);
        t->mh = m->mh; // copy cấu trúc (shallow) – an toàn vì PTRCH của m->mh là NULL (do đã check)
        return xoaMonHoc(t->right, m->mh.MAMH);
    }
}
