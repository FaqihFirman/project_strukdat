#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <functional>
#include <iomanip>
#include <cctype>

using namespace std;

struct User {
    string nama_lengkap;
    string tanggal_lahir;
    string username;
    string hashed_password;
};

struct Project {
    string id_project;
    string nama_project;
    string deskripsi;
    string nama_pembuat;
    double target_dana;
    double dana_terkumpul;
};

unordered_map<string, User> akun;
vector<Project> daftarProject;

string hashPassword(const string& password) {
    hash<string> hasher;
    return to_string(hasher(password));
}

string generateProjectID() {
    int nextID = daftarProject.size() + 1;
    stringstream ss;
    ss << "PRJ" << setfill('0') << setw(3) << nextID;
    return ss.str();
}

void tampilkanBarProgress(double terkumpul, double target) {
    const int panjangBar = 50;
    double persentase = (terkumpul / target) * 100.0;
    int filled = static_cast<int>((persentase / 100.0) * panjangBar);
    cout << "Terkumpul " << fixed << setprecision(1) << persentase << "% : ";
    for (int i = 0; i < filled; ++i) cout << '|';
    for (int i = filled; i < panjangBar; ++i) cout << '-';
    cout << "\n";
}

bool isKabisat(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

bool formatTanggalValid(const string& tgl, string& pesan_error) {
    if (tgl.length() != 10 || tgl[2] != '-' || tgl[5] != '-') {
        pesan_error = "❌ Format salah! Gunakan format DD-MM-YYYY (contoh: 13-04-2003)";
        return false;
    }
    string dd = tgl.substr(0, 2), mm = tgl.substr(3, 2), yyyy = tgl.substr(6, 4);
    for (char c : dd + mm + yyyy) if (!isdigit(c)) {
        pesan_error = "❌ Tanggal harus hanya berisi angka dan tanda '-' di posisi ke-3 dan ke-6.";
        return false;
    }
    int day = stoi(dd), month = stoi(mm), year = stoi(yyyy);
    if (month < 1 || month > 12) { pesan_error = "❌ Bulan tidak valid."; return false; }
    if (year < 1900 || year > 2100) { pesan_error = "❌ Tahun tidak valid."; return false; }
    int daysInMonth[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (isKabisat(year)) daysInMonth[2] = 29;
    if (day < 1 || day > daysInMonth[month]) {
        pesan_error = (day == 29 && month == 2) ? "❌ Bukan tahun kabisat." : "❌ Hari tidak valid.";
        return false;
    }
    return true;
}

void muatAkunDariFile() {
    ifstream file("akun.txt");
    string baris;
    while (getline(file, baris)) {
        stringstream ss(baris);
        string username, nama, tgl, pass;
        getline(ss, username, ','); getline(ss, nama, ','); getline(ss, tgl, ','); getline(ss, pass, ',');
        akun[username] = {nama, tgl, username, pass};
    }
    file.close();
}

void simpanAkunKeFile() {
    ofstream file("akun.txt");
    for (const auto& [username, user] : akun)
        file << user.username << "," << user.nama_lengkap << "," << user.tanggal_lahir << "," << user.hashed_password << "\n";
    file.close();
}

void muatProjectDariFile() {
    ifstream file("project.txt");
    string baris;
    while (getline(file, baris)) {
        stringstream ss(baris);
        Project p;
        string target, terkumpul;
        getline(ss, p.id_project, ','); getline(ss, p.nama_project, ','); getline(ss, p.deskripsi, ',');
        getline(ss, p.nama_pembuat, ','); getline(ss, target, ','); getline(ss, terkumpul, ',');
        p.target_dana = stod(target); p.dana_terkumpul = stod(terkumpul);
        daftarProject.push_back(p);
    }
    file.close();
}

void simpanProjectKeFile() {
    ofstream file("project.txt");
    for (const auto& p : daftarProject)
        file << p.id_project << "," << p.nama_project << "," << p.deskripsi << "," << p.nama_pembuat << ","
             << p.target_dana << "," << p.dana_terkumpul << "\n";
    file.close();
}

void buatProject(const User& user) {
    cin.ignore(); Project p;
    p.id_project = generateProjectID();
    cout << "\n--- Buat Project Baru ---\n";
    cout << "Nama Project: "; getline(cin, p.nama_project);
    cout << "Deskripsi Singkat Project: "; getline(cin, p.deskripsi);
    p.nama_pembuat = user.nama_lengkap;
    cout << "Target Dana yang Dibutuhkan: "; cin >> p.target_dana;
    p.dana_terkumpul = 0;
    daftarProject.push_back(p);
    simpanProjectKeFile();
    cout << "✅ Project berhasil dibuat dan disimpan!\n";
}

void lihatProjectSaya(const User& user) {
    cout << "\n===== PROJECT YANG SUDAH KAMU BUAT =====\n";
    int jumlah = 0;
    for (const auto& p : daftarProject) {
        if (p.nama_pembuat == user.nama_lengkap) {
            jumlah++;
        }
    }
    cout << "\nJumlah project : " << jumlah << "\n";
    if (jumlah == 0) {
        cout << "⚠️ Anda belum memiliki project.\n";
    } else {
        cout << "\nList project :\n";
        int no = 1;
        for (const auto& p : daftarProject) {
            if (p.nama_pembuat == user.nama_lengkap) {
                cout << no++ << ". " << p.nama_project << "\n";
            }
        }
    }
}

void lihatSemuaProject() {
    cout << "\n===== DAFTAR PROJECT TERSEDIA =====\n";
    for (size_t i = 0; i < daftarProject.size(); ++i) {
        const auto& p = daftarProject[i];
        cout << i + 1 << ". [" << p.id_project << "] " << p.nama_project << "\n";
        tampilkanBarProgress(p.dana_terkumpul, p.target_dana);
    }
}

void donasiKeProject() {
    lihatSemuaProject();
    int pilihan;
    cout << "\nMasukkan nomor project yang ingin Anda donasi: ";
    cin >> pilihan;
    if (pilihan < 1 || pilihan > daftarProject.size()) {
        cout << "❌ Nomor tidak valid.\n";
        return;
    }
    Project& p = daftarProject[pilihan - 1];
    cout << "\nDeskripsi : " << p.deskripsi << "\n";
    cout << "Target Dana: " << p.target_dana << "\n";
    cout << "Dana Terkumpul: " << p.dana_terkumpul << "\n";
    double nominal;
    cout << "Masukkan jumlah donasi: ";
    cin >> nominal;
    if (nominal <= 0) {
        cout << "❌ Nominal donasi tidak valid.\n";
        return;
    }
    p.dana_terkumpul += nominal;
    simpanProjectKeFile();
    cout << "✅ Terima kasih! Donasi Anda telah diterima.\n";
    tampilkanBarProgress(p.dana_terkumpul, p.target_dana);
}

void homeMenu(const User& user) {
    int pilihan;
    do {
        cout << "\n=== Home Menu ===\n";
        cout << "Halo, " << user.nama_lengkap << "!\n";
        cout << "Apa yang ingin Anda lakukan?\n";
        cout << "1. Donatur - Donasi ke project\n";
        cout << "2. Creator - Buat/Lihat project\n";
        cout << "3. Logout\n";
        cout << "Pilih menu: ";
        cin >> pilihan;

        switch (pilihan) {
            case 1:
                donasiKeProject();
                break;
            case 2: {
                int sub;
                cout << "\n--- Menu Creator ---\n1. Buat Project\n2. Lihat Project Saya\nPilih: ";
                cin >> sub;
                if (sub == 1) buatProject(user);
                else if (sub == 2) lihatProjectSaya(user);
                else cout << "❌ Pilihan tidak valid.\n";
                break;
            }
            case 3:
                cout << "👋 Logout berhasil.\n";
                break;
            default:
                cout << "❌ Pilihan tidak valid.\n";
        }
    } while (pilihan != 3);
}

void registrasi() {
    string nama, tgl, username, password, pesan_error;
    cout << "\n--- Registrasi Akun ---\n";
    cin.ignore();
    cout << "Nama Lengkap: "; getline(cin, nama);
    do {
        cout << "Tanggal Lahir (DD-MM-YYYY): "; getline(cin, tgl);
        if (!formatTanggalValid(tgl, pesan_error)) cout << pesan_error << "\n";
    } while (!formatTanggalValid(tgl, pesan_error));
    cout << "Username: "; getline(cin, username);
    if (akun.find(username) != akun.end()) {
        cout << "❌ Username sudah digunakan.\n";
        return;
    }
    cout << "Password: "; getline(cin, password);
    akun[username] = {nama, tgl, username, hashPassword(password)};
    simpanAkunKeFile();
    cout << "✅ Registrasi berhasil!\n";
}

void login() {
    string username, password;
    cout << "\n--- Login ---\nUsername: "; cin >> username;
    cout << "Password: "; cin >> password;
    if (akun.count(username) && akun[username].hashed_password == hashPassword(password)) {
        cout << "✅ Login berhasil.\n";
        homeMenu(akun[username]);
    } else {
        cout << "❌ Login gagal. Username atau password salah.\n";
    }
}

void menuUtama() {
    int pilihan;
    do {
        cout << "\n=== Menu Crowdfunding ===\n";
        cout << "1. Registrasi\n2. Login\n3. Keluar\nPilih menu: ";
        cin >> pilihan;
        if (pilihan == 1) registrasi();
        else if (pilihan == 2) login();
        else if (pilihan == 3) cout << "👋 Terima kasih telah menggunakan sistem crowdfunding!\n";
        else cout << "❌ Pilihan tidak valid.\n";
    } while (pilihan != 3);
}

int main() {
    muatAkunDariFile();
    muatProjectDariFile();
    menuUtama();
    return 0;
}