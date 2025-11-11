#include "include/khai_bao.h"
#include <cstring>
using namespace std; 
// ======= TÌM LỚP THEO MÃ =======
int TimLop(DS_LOP& ds, const char* malop) {
    int left = 1, right = ds.n;

    while (left <= right) {
        int mid = (left + right) / 2;
        int cmp = strcmp(malop, ds.nodes[mid]->MALOP);
        if (cmp == 0) return mid;
        else if (cmp > 0) left = mid;
        else right = mid;
    }

    return -1;
}


// ======= THÊM LỚP MỚI =======
bool ThemLop(DS_LOP& ds, Lop* lopmoi) {
    if (ds.n >= MAX_LOP - 1) {
        cout << "Danh sach lop da day!" << endl;
        return false;
    }

    if (TimLop(ds, lopmoi->MALOP) != -1) {
        cout << "Ma lop da ton tai!" << endl;
        return false;
    }
    
    int pos = 1;

    while (pos <= ds.n && strcmp(lopmoi->MALOP, ds.nodes[pos]->MALOP) > 0) {
        pos++;
    }

    for (int i = ds.n; i >= pos; i--) {
        ds.nodes[i] = ds.nodes[i - 1];
    }

    ds.nodes[pos] = lopmoi;
    ds.n++;

    return true;
}

// ======= XÓA LỚP =======
bool XoaLop(DS_LOP& ds, const char* maLop) {
    int index = TimLop(ds, maLop);
    if (index == -1) {
        cout << "Khong tim thay lop can xoa!" << endl;
        return false;
    }

    // Giải phóng danh sách sinh viên của lớp
    PTRSV p = ds.nodes[index]->FirstSV;
    while (p != NULL) {
        PTRSV temp = p;
        p = p->next;
        delete temp;
    }

    delete ds.nodes[index]; // xóa lớp trong bộ nhớ

    // Dồn mảng
    for (int i = index; i < ds.n - 1; i++) {
        ds.nodes[i] = ds.nodes[i + 1];
    }
    ds.n--;

    return true;
}

// ======= XUẤT DANH SÁCH LỚP =======
void XuatDanhSachLop(DS_LOP& ds) {
    cout << "===== DANH SACH LOP =====" << endl;
    for (int i = 0; i < ds.n; i++) {
        cout << i + 1 << ". " << ds.nodes[i]->MALOP
             << " - " << ds.nodes[i]->TENLOP << endl;
    }
}

// ======= GIẢI PHÓNG TOÀN BỘ DANH SÁCH =======
void GiaiPhongLop(DS_LOP& ds) {
    for (int i = 0; i < ds.n; i++) {
        PTRSV p = ds.nodes[i]->FirstSV;
        while (p != NULL) {
            PTRSV temp = p;
            p = p->next;
            delete temp;
        }
        delete ds.nodes[i];
    }
    ds.n = 0;
}
