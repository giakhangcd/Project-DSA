#include "include/khai_bao.h"
#include"include/utils.h"

using namespace std;

void ChuanHoaTen(char* s) {
    // Bỏ khoảng trắng đầu & cuối
    while (isspace(s[0])) memmove(s, s + 1, strlen(s));
    while (strlen(s) > 0 && isspace(s[strlen(s) - 1])) s[strlen(s) - 1] = '\0';

    // Viết thường toàn bộ, rồi viết hoa chữ đầu
    strlwr(s);
    if (strlen(s) > 0) s[0] = toupper(s[0]);
    for (int i = 1; i < strlen(s); i++) {
        if (s[i - 1] == ' ')
            s[i] = toupper(s[i]);
    }
}

TREE_MH TimMonHoc(TREE_MH root, const char* key) {
    if (root == NULL) return NULL;  

    int cmp = strcmp(key, root->mh.MAMH);

    if (cmp == 0) return root;          
    else if (cmp < 0) return TimMonHoc(root->left, key);   
    else return TimMonHoc(root->right, key);              
}

bool ValidateMonHoc(const MonHoc& mh, TREE_MH root) {
    // Lấy độ dài sẵn (tránh gọi strlen nhiều lần)
    const size_t lenMa  = strlen(mh.MAMH);
    const size_t lenTen = strlen(mh.TENMH);

    //Kiểm tra MAMH
    if (lenMa == 0) {
        cout << "❌ Loi: Ma mon hoc khong duoc rong.\n";
        return false;
    }
    if (lenMa > 15) { // vì MAMH[16] có 1 ký tự null
        cout << "❌ Loi: Ma mon hoc qua dai (toi da 15 ky tu).\n";
        return false;
    }

    //Kiểm tra từng ký tự
    for (unsigned char c : string(mh.MAMH, lenMa)) {
        if (!isalnum(c)) {
            cout << "❌ Loi: Ma mon hoc chi duoc chua chu va so.\n";
            return false;
        }
    }

    //Kiểm tra TENMH
    if (lenTen == 0) {
        cout << "❌ Loi: Ten mon hoc khong duoc rong.\n";
        return false;
    }
    if (lenTen > 50) { // vì TENMH[51]
        cout << "❌ Loi: Ten mon hoc qua dai (toi da 50 ky tu).\n";
        return false;
    }

    for (unsigned char c : string(mh.TENMH, lenTen)) {
        if (!(isalnum(c) || isspace(c) || c == '-' || c == '_')) {
            cout << "❌ Loi: Ten mon hoc chi duoc chua chu, so, khoang trang, dau '-' hoac '_'.\n";
            return false;
        }
    }

    //Kiểm tra trùng mã trong cây
    if (TimMonHoc(root, mh.MAMH) != nullptr) {
        cout << "❌ Loi: Ma mon hoc da ton tai trong he thong.\n";
        return false;
    }

    // Hợp lệ
    return true;
}

bool operator <(const MonHoc& x, const MonHoc& y) {
    if (strcmp(x.MAMH, y.MAMH) == 0 || strcmp(x.MAMH, y.MAMH) > 0) return false;
    else return true;
}

bool operator >(const MonHoc& x, const MonHoc& y) {
    if (strcmp(x.MAMH, y.MAMH) == 0 || strcmp(x.MAMH, y.MAMH) < 0) return false;
    else return true;
}

bool operator ==(const MonHoc& x, const MonHoc& y) {
    if (strcmp(x.MAMH, y.MAMH) == 0) return true;
    else return false;
}

nodeMH::nodeMH(MonHoc x) {
    mh = x;
    left = right = NULL;
    height = 1;
}

TREE_MH minnode(TREE_MH root) {
    TREE_MH tmp = root;
    while (tmp != NULL && tmp->left != NULL) tmp = tmp->left;
    return tmp;
}

TREE_MH deletenode(TREE_MH root, MonHoc key) {
    if (root == NULL) return root;
    if (key < root->mh) {
        root->left = deletenode(root->left, key);
    } else if (key > root->mh) {
        root->right = deletenode(root->right, key);
    } else {
        if (root->left == NULL) {
            TREE_MH tmp = root->right;
            delete root;
            return tmp;
        } else if (root->right == NULL) {
            TREE_MH tmp = root->left;
            delete root;
            return tmp;
        } else {
            TREE_MH tmp = minnode(root->right);
            root->mh = tmp->mh;
            root->right = deletenode(root->right, tmp->mh);
        }
    }
    return root;
}

int getheight(TREE_MH root) {
    if (root == NULL) return 0;
    return root->height;
}

int getbalance(TREE_MH root) {
    if (root == NULL) return 0;
    return getheight(root->left) - getheight(root->right);
}

void updateHeight(TREE_MH root) {
    if (root != NULL) {
        root->height = 1 + max(getheight(root->left), getheight(root->right));
    }
}

TREE_MH leftRotate(TREE_MH x) { 
    TREE_MH y = x->right;
    TREE_MH z = y->left;

    y->left = x;
    x->right = z;

    updateHeight(x);
    updateHeight(y);

    return y;
}

TREE_MH rightRotate(TREE_MH x) {
    TREE_MH y = x->left;
    TREE_MH z = y->right;

    y->right = x;
    x->left = z;

    updateHeight(x);
    updateHeight(y);

    return y;
}

TREE_MH themMonHoc(TREE_MH root, MonHoc key) {
    if (root == NULL) {
        TREE_MH newnode = new nodeMH(key);
        return newnode;
    }
    if (key < root->mh) {
        root->left = themMonHoc(root->left, key);
    } else if (key > root->mh) {
        root->right = themMonHoc(root->right, key);
    } else {
        return root;
    }
    
    updateHeight(root);

    int balance = getbalance(root);

    if (balance > 1 && key < root->left->mh) {
        return rightRotate(root);
    }

    if (balance < -1 && key > root->right->mh) {
        return leftRotate(root);
    }

    if (balance > 1 && key > root->left->mh) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }

    if (balance < -1 && key < root->right->mh) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}


