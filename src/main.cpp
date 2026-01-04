#include <iostream>
#include "thitracnghiem.h"
#include "utils.h"
#include "ui.h"

using namespace std;
int main(){
    setConsoleSize(120, 30);
    hideCursor(false);
    clearConsole();

    TREE_MH dsMH; khoiTaoTree(dsMH);
    DS_LOP dsLop; initDSLop(dsLop);

    // 1) nạp dữ liệu từ các file
    loadMonHoc("data/monhoc.txt", dsMH);
    loadLop("data/lop.txt", dsLop);
    loadSinhVien("data/sinhvien.txt", dsLop);
    // loadCauHoi("data/cauhoi.txt", dsMH);
    loadDiemThi("data/diemthi.txt", dsLop, dsMH); // nếu cần
    docFileMonLop();

    // 2) chạy menu
    menuChinh(dsMH, dsLop);
    return 0;
}