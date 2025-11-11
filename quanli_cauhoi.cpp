#include "include/khai_bao.h"
using namespace std;

nodeCH::nodeCH(CauHoi x) {
    ch = x;
    next = NULL;
}

PTRCH khoiTaoDanhSach(PTRCH first) {
    first = nullptr;
    return first;
}

// Kiểm tra hợp lệ
bool validateCauHoi(const CauHoi &ch, string &msg) {
    if (strlen(ch.NoiDung) == 0) {
        msg = "Noi dung cau hoi khong duoc de trong!";
        return false;
    }
    if (strlen(ch.A) == 0 || strlen(ch.B) == 0 || strlen(ch.C) == 0 || strlen(ch.D) == 0) {
        msg = "Tat ca dap an A, B, C, D phai duoc nhap!";
        return false;
    }
    if (toupper(ch.DapAn) < 'A' || toupper(ch.DapAn) > 'D') {
        msg = "Dap an phai la mot trong cac ky tu A, B, C, D!";
        return false;
    }
    return true;
}

// Tìm câu hỏi theo ID
PTRCH timCauHoi(PTRCH first, int id) {
    for (PTRCH p = first; p != nullptr; p = p->next)
        if (p->ch.ID == id) return p;
    return nullptr;
}

// Thêm câu hỏi
bool themCauHoi(PTRCH &first, const CauHoi &ch) {
    // Kiểm tra trùng ID
    if (timCauHoi(first, ch.ID) != nullptr) {
        cout << "❌ Loi: ID " << ch.ID << " da ton tai!\n";
        return false;
    }

    PTRCH p = new nodeCH(ch);

    if (first == nullptr) first = p;
    else {
        PTRCH q = first;
        while (q->next != nullptr) q = q->next;
        q->next = p;
    }
    return true;
}

// Xóa câu hỏi
bool xoaCauHoi(PTRCH &first, int id) {
    PTRCH p = first, prev = nullptr;
    while (p != nullptr && p->ch.ID != id) {
        prev = p;
        p = p->next;
    }
    if (p == nullptr) return false; // không tìm thấy
    if (prev == nullptr) first = p->next;
    else prev->next = p->next;
    delete p;
    return true;
}

// Hiển thị danh sách
void hienThiDanhSachCauHoi(PTRCH first) {
    cout << "\n=== DANH SACH CAU HOI ===\n";
    for (PTRCH p = first; p != nullptr; p = p->next) {
        cout << "ID: " << p->ch.ID << endl;
        cout << "Noi dung: " << p->ch.NoiDung << endl;
        cout << "A. " << p->ch.A << endl;
        cout << "B. " << p->ch.B << endl;
        cout << "C. " << p->ch.C << endl;
        cout << "D. " << p->ch.D << endl;
        cout << "Dap an: " << p->ch.DapAn << "\n\n";
    }
}
