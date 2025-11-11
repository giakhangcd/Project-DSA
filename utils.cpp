#include "include/utils.h"
#include "include/khai_bao.h"
#include <fstream>
#include <filesystem>

using namespace std;

void loaddulieu_sv_lop(const string &folder_name, DS_LOP &ds) {
    fs::path root = fs::path("data") / folder_name;

    if (!fs::exists(root) || !fs::is_directory(root)) {
        cerr << "Thu muc du lieu khong ton tai hoac khong hop le: " << root << endl;
        return;
    }

    for (auto &lop : fs::directory_iterator(root)) {
        if (!lop.is_directory()) continue;

        // Cấp phát động cho Lop
        Lop* new_lop = new Lop;
        memset(new_lop, 0, sizeof(Lop)); // tránh rác trong mảng ký tự

        fs::path file_info_lop = lop.path() / "info_lop.txt";
        if (!fs::exists(file_info_lop)) {
            cerr << "Khong tim thay file thong tin lop: " << file_info_lop << endl;
            delete new_lop;
            continue;
        }

        ifstream file(file_info_lop);
        if (!file.is_open()) {
            cerr << "Khong the mo file: " << file_info_lop << endl;
            delete new_lop;
            continue;
        }

        string ingredient;
        int index = 0;
        while (getline(file, ingredient, '|')) {
            switch (index++) {
                case 0:
                    strncpy(new_lop->MALOP, ingredient.c_str(), sizeof(new_lop->MALOP) - 1);
                    new_lop->MALOP[sizeof(new_lop->MALOP) - 1] = '\0';
                    break;
                case 1:
                    strncpy(new_lop->TENLOP, ingredient.c_str(), sizeof(new_lop->TENLOP) - 1);
                    new_lop->TENLOP[sizeof(new_lop->TENLOP) - 1] = '\0';
                    break;
                default:
                    break;
            }
        }
        file.close();

        // Kiểm tra tối thiểu dữ liệu lớp
        if (strlen(new_lop->MALOP) == 0 || strlen(new_lop->TENLOP) == 0) {
            cerr << "Thong tin lop khong hop le trong: " << file_info_lop << endl;
            delete new_lop;
            continue;
        }

        // Thêm lớp vào danh sách
        ThemLop(ds, new_lop);

        // Duyệt thư mục sinh viên trong lớp
        for (auto &sv : fs::directory_iterator(lop.path())) {
            if (!sv.is_directory()) continue;

            fs::path file_info_sv = sv.path() / "info_sv.txt";
            if (!fs::exists(file_info_sv)) continue;

            ifstream file_sv(file_info_sv);
            if (!file_sv.is_open()) {
                cerr << "Khong the mo file: " << file_info_sv << endl;
                continue;
            }

            SinhVien new_sv;
            memset(&new_sv, 0, sizeof(SinhVien));

            string Ingredient;
            int Index = 0;
            while (getline(file_sv, Ingredient, '|')) {
                switch (Index++) {
                    case 0:
                        strncpy(new_sv.MASV, Ingredient.c_str(), sizeof(new_sv.MASV) - 1);
                        new_sv.MASV[sizeof(new_sv.MASV) - 1] = '\0';
                        break;
                    case 1:
                        strncpy(new_sv.HO, Ingredient.c_str(), sizeof(new_sv.HO) - 1);
                        new_sv.HO[sizeof(new_sv.HO) - 1] = '\0';
                        break;
                    case 2:
                        strncpy(new_sv.TEN, Ingredient.c_str(), sizeof(new_sv.TEN) - 1);
                        new_sv.TEN[sizeof(new_sv.TEN) - 1] = '\0';
                        break;
                    case 3:
                        strncpy(new_sv.PHAI, Ingredient.c_str(), sizeof(new_sv.PHAI) - 1);
                        new_sv.PHAI[sizeof(new_sv.PHAI) - 1] = '\0';
                        break;
                    case 4:
                        strncpy(new_sv.password, Ingredient.c_str(), sizeof(new_sv.password) - 1);
                        new_sv.password[sizeof(new_sv.password) - 1] = '\0';
                        break;
                    default:
                        break;
                }
            }
            file_sv.close();

            // Kiểm tra hợp lệ sinh viên
            std::string msg;
            if (!validateSinhVien(new_sv, msg)) {
                cerr << "Sinh vien khong hop le (" << file_info_sv << "): " << msg << endl;
                continue;
            }

            PTRSV New_sv = new nodeSV(new_sv);
            if (!ThemSinhVien(New_sv, new_lop, ds)) {
                cerr << "Khong the them sinh vien vao lop: " << new_lop->MALOP << endl;
                delete New_sv; // tránh rò rỉ bộ nhớ
            }
        }
    }
} 

void ghidulieu_sv_lop(const string &folder_name, DS_LOP &ds) {
    fs::path root = fs::path("data") / folder_name;

    // Tạo thư mục gốc nếu chưa có
    if (!fs::exists(root)) {
        fs::create_directories(root);
    }

    for (int i = 0; i < ds.n; i++) {  
        Lop* lop_ptr = ds.nodes[i];
        if (lop_ptr == nullptr) continue;

        // --- GHI DỮ LIỆU LỚP ---
        fs::path folder_lop = root / lop_ptr->MALOP;

        if (!fs::exists(folder_lop)) {
            fs::create_directories(folder_lop);
        }

        fs::path file_info_lop = folder_lop / "info_lop.txt";
        ofstream file_lop(file_info_lop, ios::out | ios::trunc);

        if (!file_lop.is_open()) {
            cerr << "Khong the mo file lop: " << file_info_lop << endl;
            continue;
        }

        // Ghi MALOP | TENLOP
        file_lop << lop_ptr->MALOP << "|" << lop_ptr->TENLOP;
        file_lop.close();

        // --- GHI DỮ LIỆU SINH VIÊN ---
        PTRSV tmp = lop_ptr->FirstSV;
        while (tmp != nullptr) {
            fs::path folder_sv = folder_lop / tmp->sv.MASV;

            if (!fs::exists(folder_sv)) {
                fs::create_directories(folder_sv);
            }

            fs::path file_info_sv = folder_sv / "info_sv.txt";
            ofstream file_sv(file_info_sv, ios::out | ios::trunc);

            if (!file_sv.is_open()) {
                cerr << "Khong the mo file SV: " << file_info_sv << endl;
                tmp = tmp->next;
                continue;
            }

            file_sv << tmp->sv.MASV << "|"
                    << tmp->sv.HO << "|"
                    << tmp->sv.TEN << "|"
                    << tmp->sv.PHAI << "|"
                    << tmp->sv.password;

            file_sv.close();
            tmp = tmp->next;
        }
    }
}

void loaddulieumonhoc(const string& folder_name, TREE_MH &first) {

    fs::path root = fs::path("data") / folder_name; 

    // Kiểm tra thư mục tồn tại
    if (!fs::exists(root) || !fs::is_directory(root)) {
        cerr << "Thu muc " << root << " khong ton tai!\n";
        return;
    }

    
    for (const auto &mh : fs::directory_iterator(root)) {
        if (fs::is_directory(mh.path())) {
            fs::path info_mh = mh.path() / "info_monhoc";

            ifstream file(info_mh);
            if (!file.is_open()) {
                cerr << "Khong the mo file" << endl;
                continue;
            }

            MonHoc mh;
            string mamh, tenmh;

            // Đọc 2 dòng đầu tiên trong file info_monhoc
            getline(file, mamh);
            getline(file, tenmh);
            file.close();

            strncpy(mh.MAMH, mamh.c_str(), sizeof(mh.MAMH) - 1);
            mh.MAMH[sizeof(mh.MAMH) - 1] = '\0';

            strncpy(mh.TENMH, tenmh.c_str(), sizeof(mh.TENMH) - 1);
            mh.TENMH[sizeof(mh.TENMH) - 1] = '\0';

            if (!ValidateMonHoc(mh, first)) {
                continue; // bỏ qua môn học lỗi
            }

            first = themMonHoc(first, mh);

        }
    }
}

void luumotmonhoc(const MonHoc& mh, const fs::path &folder_mh) {
    // Tạo thư mục môn học nếu chưa có
    if (!fs::exists(folder_mh)) {
        fs::create_directories(folder_mh);
    }

    // Ghi file info_monhoc
    fs::path info_mh = folder_mh / "info_monhoc";
    ofstream file(info_mh);
    if (!file.is_open()) {
        cerr << "Khong the mo file de ghi" << endl;
        return;
    }

    file << mh.MAMH << endl;
    file << mh.TENMH << endl;
    file.close();
}

void ghidulieumonhoc(TREE_MH first, const string& folder_name) {
    if (first == NULL) return;

    fs::path root = fs::path("data") / folder_name;
    if (!fs::exists(root)) {
        fs::create_directories(root);
    }

    // Tạo đường dẫn riêng cho từng môn học: data/monhoc/<MAMH>
    fs::path folder_mh = root / first->mh.MAMH;
    luumotmonhoc(first->mh, folder_mh);

    // Ghi đệ quy các nhánh con
    ghidulieumonhoc(first->left, folder_name);
    ghidulieumonhoc(first->right, folder_name);
}
