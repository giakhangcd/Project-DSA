#include <iostream>
#include "include/khai_bao.h"
#include "include/utils.h"

using namespace std;

int main() {
    DS_LOP ds;
    loaddulieu_sv_lop("ds_lop", ds);
    string s = "N24DECE089";
    PTRSV x = TimSinhVien(ds.nodes[1]->FirstSV, s.c_str());
    cout << ds.n << endl;
    cout << ds.nodes[1]->MALOP << " " << ds.nodes[1]->TENLOP << " " << ds.nodes[1]->FirstSV->sv.MASV << endl;
    cout << x->sv.MASV << " " << x->sv.HO << " " << x->sv.TEN << " " << x->sv.PHAI; 
    system("pause");
    return 0;
}