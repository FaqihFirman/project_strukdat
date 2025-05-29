#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <functional>
#include <iomanip>
#include <cctype>
#include <ctime>
#include <chrono>

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

struct Donasi {
    string id_project;
    string nama_project;
    double nominal;
    string timestamp;
};

class StackDonasi {
private:
    struct Node {
        Donasi data;
        Node* next;
        Node(const Donasi& d) : data(d), next(nullptr) {}
    };

    Node* top_node;
    int size_count;

public:
    StackDonasi() : top_node(nullptr), size_count(0) {}
    ~StackDonasi() { clear(); }

    StackDonasi(const StackDonasi& other) : top_node(nullptr), size_count(0) {
        if (other.top_node) {
            Donasi* temp = new Donasi[other.size_count];
            Node* curr = other.top_node;
            int idx = other.size_count - 1;
            while (curr) {
                temp[idx--] = curr->data;
                curr = curr->next;
            }
            for (int i = 0; i < other.size_count; i++) push(temp[i]);
            delete[] temp;
        }
    }

    StackDonasi& operator=(const StackDonasi& other) {
        if (this != &other) {
            clear();
            if (other.top_node) {
                Donasi* temp = new Donasi[other.size_count];
                Node* curr = other.top_node;
                int idx = other.size_count - 1;
                while (curr) {
                    temp[idx--] = curr->data;
                    curr = curr->next;
                }
                for (int i = 0; i < other.size_count; i++) push(temp[i]);
                delete[] temp;
            }
        }
        return *this;
    }

    void push(const Donasi& d) {
        Node* new_node = new Node(d);
        new_node->next = top_node;
        top_node = new_node;
        size_count++;
    }

    void pop() {
        if (top_node) {
            Node* temp = top_node;
            top_node = top_node->next;
            delete temp;
            size_count--;
        }
    }

    Donasi top() const {
        if (top_node) return top_node->data;
        return {"", "", 0.0, ""};
    }

    bool empty() const { return top_node == nullptr; }
    int size() const { return size_count; }

    void clear() {
        while (!empty()) pop();
    }

    vector<Donasi> getAllData() const {
        vector<Donasi> result;
        Node* curr = top_node;
        while (curr) {
            result.push_back(curr->data);
            curr = curr->next;
        }
        return result;
    }
};

struct NodeProject {
    Project data;
    NodeProject* next;
    NodeProject(const Project& p) : data(p), next(nullptr) {}
};

class LinkedListProject {
private:
    NodeProject* head;
    NodeProject* tail;
    int count;

public:
    LinkedListProject() : head(nullptr), tail(nullptr), count(0) {}
    ~LinkedListProject() { clear(); }

    void add(const Project& p) {
        NodeProject* newNode = new NodeProject(p);
        if (!head) head = tail = newNode;
        else {
            tail->next = newNode;
            tail = newNode;
        }
        count++;
    }

    Project& get(int index) {
        NodeProject* curr = head;
        for (int i = 0; i < index && curr; ++i) {
            curr = curr->next;
        }
        if (!curr) throw out_of_range("Index di luar batas");
        return curr->data;
    }

    int size() const { return count; }

    void clear() {
        NodeProject* curr = head;
        while (curr) {
            NodeProject* tmp = curr;
            curr = curr->next;
            delete tmp;
        }
        head = tail = nullptr;
        count = 0;
    }

    NodeProject* begin() const { return head; }
};

unordered_map<string, User> akun;
LinkedListProject daftarProject;
unordered_map<string, StackDonasi> logDonasiUser;
// Global variables


// Timestamp sekarang
string getCurrentTimestamp() {
    auto now = chrono::system_clock::now();
    time_t time_now = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&time_now), "%d-%m-%Y %H:%M:%S");
    return ss.str();
}

// Hash password sederhana
string hashPassword(const string& password) {
    hash<string> hasher;
    return to_string(hasher(password));
}

// Validasi tahun kabisat
bool isKabisat(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Validasi format tanggal
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

// Generate ID Project otomatis
string generateProjectID() {
    int nextID = daftarProject.size() + 1;
    stringstream ss;
    ss << "PRJ" << setfill('0') << setw(3) << nextID;
    return ss.str();
}

// Load akun dari file
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

// Simpan akun ke file
void simpanAkunKeFile() {
    ofstream file("akun.txt");
    for (const auto& [username, user] : akun)
        file << user.username << "," << user.nama_lengkap << "," << user.tanggal_lahir << "," << user.hashed_password << "\n";
    file.close();
}

// Load project dari file
void muatProjectDariFile() {
    ifstream file("project.txt");
    string baris;
    while (getline(file, baris)) {
        stringstream ss(baris);
        Project p;
        string target, terkumpul;

        getline(ss, p.id_project, ',');
        getline(ss, p.nama_project, ',');
        getline(ss, p.deskripsi, ',');
        getline(ss, p.nama_pembuat, ',');
        getline(ss, target, ',');
        getline(ss, terkumpul, ',');

        try {
            p.target_dana = stod(target);
            p.dana_terkumpul = stod(terkumpul);
            daftarProject.add(p);
        } catch (const exception& e) {
            cerr << "❌ Error membaca project: " << baris << "\n";
            cerr << "   → Alasan: " << e.what() << "\n";
            continue;
        }
    }
    file.close();
}

// Simpan project ke file
void simpanProjectKeFile() {
    ofstream file("project.txt");
    for (NodeProject* curr = daftarProject.begin(); curr != nullptr; curr = curr->next)
        file << curr->data.id_project << "," << curr->data.nama_project << "," << curr->data.deskripsi << "," << curr->data.nama_pembuat << ","
             << curr->data.target_dana << "," << curr->data.dana_terkumpul << "\n";
    file.close();
}

// Buat project baru
void buatProject(const User& user) {
    cin.ignore();
    Project p;
    p.id_project = generateProjectID();
    cout << "\n--- Buat Project Baru (ketik -1 untuk batal) ---\n";

    cout << "Nama Project: ";
    getline(cin, p.nama_project);
    if (p.nama_project == "-1") {
        cout << "❌ Pembuatan project dibatalkan.\n";
        return;
    }

    cout << "Deskripsi Singkat Project: ";
    getline(cin, p.deskripsi);
    if (p.deskripsi == "-1") {
        cout << "❌ Pembuatan project dibatalkan.\n";
        return;
    }

    p.nama_pembuat = user.nama_lengkap;

    cout << "Target Dana yang Dibutuhkan: ";
    string inputTarget;
    getline(cin, inputTarget);
    if (inputTarget == "-1") {
        cout << "❌ Pembuatan project dibatalkan.\n";
        return;
    }

    try {
        p.target_dana = stod(inputTarget);
        if (p.target_dana <= 0) {
            cout << "❌ Target dana harus lebih dari 0.\n";
            return;
        }
    } catch (...) {
        cout << "❌ Input target dana tidak valid.\n";
        return;
    }

    p.dana_terkumpul = 0;
    daftarProject.add(p);
    simpanProjectKeFile();
    cout << "✅ Project berhasil dibuat dan disimpan!\n";
}

// Lihat project yang dibuat user
void lihatProjectSaya(const User& user) {
    bool ada = false;
    int no = 1;
    for (NodeProject* curr = daftarProject.begin(); curr != nullptr; curr = curr->next) {
        const auto& p = curr->data;
        if (p.nama_pembuat == user.nama_lengkap) {
            if (!ada) {
                cout << "\n===== PROJECT YANG SUDAH KAMU BUAT =====\n";
                cout << "+-----+------------------------------+---------------------+---------------------+\n";
                cout << "| No. | Nama Project                | Target Dana         | Dana Terkumpul      |\n";
                cout << "+-----+------------------------------+---------------------+---------------------+\n";
                ada = true;
            }
            cout << "| " << setw(3) << no++ << " | "
                 << setw(28) << left << p.nama_project << " | "
                 << setw(19) << right << fixed << setprecision(2) << p.target_dana << " | "
                 << setw(19) << right << fixed << setprecision(2) << p.dana_terkumpul << " |\n";
        }
    }
    if (!ada) {
        cout << "⚠️ Anda belum memiliki project.\n";
    } else {
        cout << "+-----+------------------------------+---------------------+---------------------+\n";
    }
}

// Lihat semua project
void lihatSemuaProject() {
    cout << "\n===== DAFTAR PROJECT TERSEDIA =====\n";

    if (daftarProject.size() == 0) {
        cout << "⚠️ Belum ada project yang tersedia.\n";
        return;
    }

    cout << "+-----+----------+-------------------------------+----------------------------+------------------------+-------------------------+--------------+\n";
    cout << "| No. | ID       | Nama Project                  | Pembuat                    | Target Dana            | Dana Terkumpul          | Progress (%)  |\n";
    cout << "+-----+----------+-------------------------------+----------------------------+------------------------+-------------------------+--------------+\n";

    int i = 1;
    for (NodeProject* curr = daftarProject.begin(); curr != nullptr; curr = curr->next) {
        const auto& p = curr->data;
        double persentase = (p.target_dana > 0) ? (p.dana_terkumpul / p.target_dana * 100.0) : 0.0;

        cout << "| " << setw(3) << i++ << " | "
             << setw(8) << left << p.id_project << " | "
             << setw(31) << left << p.nama_project << " | "
             << setw(26) << left << p.nama_pembuat << " | "
             << setw(22) << right << fixed << setprecision(2) << p.target_dana << " | "
             << setw(23) << right << fixed << setprecision(2) << p.dana_terkumpul << " | "
             << setw(11) << right << fixed << setprecision(1) << persentase << "% |\n";
    }

    cout << "+-----+----------+-------------------------------+----------------------------+------------------------+-------------------------+--------------+\n";
}



// Fungsi untuk menyimpan log donasi ke file
void simpanLogDonasiKeFile() {
    ofstream file("log_donasi.txt");
    for (auto& [username, stackDonasi] : logDonasiUser) {
        vector<Donasi> donasiList = stackDonasi.getAllData();
        for (int i = donasiList.size() - 1; i >= 0; i--) {
            const Donasi& d = donasiList[i];
            file << username << "," << d.id_project << "," << d.nama_project << "," 
                 << d.nominal << "," << d.timestamp << "\n";
        }
    }
    file.close();
}

// Fungsi untuk memuat log donasi dari file
void muatLogDonasiDariFile() {
    ifstream file("log_donasi.txt");
    string baris;
    unordered_map<string, vector<Donasi>> tempUserDonasi;

    while (getline(file, baris)) {
        stringstream ss(baris);
        string username, id_project, nama_project, nominal_str, timestamp;
        getline(ss, username, ',');
        getline(ss, id_project, ',');
        getline(ss, nama_project, ',');
        getline(ss, nominal_str, ',');
        getline(ss, timestamp);

        if (!username.empty() && !id_project.empty()) {
            try {
                Donasi d = {id_project, nama_project, stod(nominal_str), timestamp};
                tempUserDonasi[username].push_back(d);
            } catch (...) {
                cerr << "⚠️ Gagal parsing donasi: " << baris << endl;
                continue;
            }
        }
    }
    file.close();

    for (auto& [username, donasiList] : tempUserDonasi) {
        for (const auto& donasi : donasiList) {
            logDonasiUser[username].push(donasi);
        }
    }
}

void tampilkanLeaderboardDonasi(const string& id_project) {
    unordered_map<string, double> totalDonasiPerUser;

    for (const auto& [username, stackDonasi] : logDonasiUser) {
        vector<Donasi> donasiList = stackDonasi.getAllData();
        for (const auto& d : donasiList) {
            if (d.id_project == id_project) {
                totalDonasiPerUser[username] += d.nominal;
            }
        }
    }

    vector<pair<string, double>> leaderboard(totalDonasiPerUser.begin(), totalDonasiPerUser.end());
    sort(leaderboard.begin(), leaderboard.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    cout << "\n🏆 LEADERBOARD DONATUR PROJECT INI 🏆\n";
    cout << "+-----+----------------+-----------------+\n";
    cout << "| No. | Username       | Total Donasi    |\n";
    cout << "+-----+----------------+-----------------+\n";
    int no = 1;
    for (const auto& [username, total] : leaderboard) {
        cout << "| " << setw(3) << no++ << " | "
             << setw(14) << left << username << " | "
             << setw(15) << right << fixed << setprecision(2) << total << " |\n";
        if (no > 5) break; // top 5 aja
    }
    cout << "+-----+----------------+-----------------+\n";
}


void donasiKeProject(const string& username) {
    lihatSemuaProject();
    cout << "\nMasukkan nomor project yang ingin Anda donasi (ketik -1 untuk batal): ";
    string input;
    cin >> input;

    if (input == "-1") {
        cout << "❌ Donasi dibatalkan.\n";
        return;
    }

    int pilihan;
    try {
        pilihan = stoi(input);
    } catch (...) {
        cout << "❌ Input tidak valid.\n";
        return;
    }

    if (pilihan < 1 || pilihan > daftarProject.size()) {
        cout << "❌ Nomor tidak valid.\n";
        return;
    }

    // Cari node ke-(pilihan - 1)
    NodeProject* curr = daftarProject.begin();
    for (int i = 1; i < pilihan && curr != nullptr; ++i) {
        curr = curr->next;
    }

    if (!curr) {
        cout << "❌ Project tidak ditemukan.\n";
        return;
    }

    Project& p = curr->data;

    cout << "\nDeskripsi : " << p.deskripsi << "\n";
    tampilkanLeaderboardDonasi(p.id_project);
    cout << "Target Dana: " << p.target_dana << "\n";
    cout << "Dana Terkumpul: " << p.dana_terkumpul << "\n";

    cout << "Masukkan jumlah donasi (ketik -1 untuk batal): ";
    string nominal_str;
    cin >> nominal_str;

    if (nominal_str == "-1") {
        cout << "❌ Donasi dibatalkan.\n";
        return;
    }

    double nominal;
    try {
        nominal = stod(nominal_str);
        if (nominal <= 0) {
            cout << "❌ Nominal donasi harus lebih dari 0.\n";
            return;
        }
    } catch (...) {
        cout << "❌ Nominal donasi tidak valid.\n";
        return;
    }

    p.dana_terkumpul += nominal;
    simpanProjectKeFile();

    string timestamp = getCurrentTimestamp();
    Donasi d = {p.id_project, p.nama_project, nominal, timestamp};
    logDonasiUser[username].push(d);
    simpanLogDonasiKeFile();

    cout << "✅ Terima kasih! Donasi Anda telah diterima pada " << timestamp << "\n";
}

// Lihat riwayat donasi
void lihatLogDonasi(const string& username) {
    cout << "\n===== RIWAYAT DONASI ANDA =====\n";
    if (logDonasiUser.find(username) == logDonasiUser.end() || logDonasiUser[username].empty()) {
        cout << "⚠️ Anda belum melakukan donasi apapun.\n";
        return;
    }

    vector<Donasi> donasiList = logDonasiUser[username].getAllData();

    cout << "+-----+---------------------+--------------------------+----------------+\n";
    cout << "| No. |      Timestamp      |       Nama Project       | Jumlah Donasi  |\n";
    cout << "+-----+---------------------+--------------------------+----------------+\n";

    for (size_t i = 0; i < donasiList.size(); ++i) {
        const Donasi& d = donasiList[i];
        cout << "| " << setw(3) << (i + 1) << " | "
             << setw(19) << left << d.timestamp << " | "
             << setw(24) << left << d.nama_project << " | "
             << setw(14) << right << fixed << setprecision(2) << d.nominal << " |\n";
    }

    cout << "+-----+---------------------+--------------------------+----------------+\n";
    cout << "| Total donasi: " << setw(3) << donasiList.size() << " kali" << setw(45) << " |\n";
    cout << "| (Menggunakan struktur data stack manual)            |\n";
    cout << "+------------------------------------------------------+\n";
}

// Menu home setelah login
void homeMenu(const User& user) {
    string pilihan;
    while (true) {
        cout << "\n+================================================+\n";
        cout << "|                  HOME MENU                     |\n";
        cout << "+================================================+\n";
        cout << "| Halo, " << setw(38) << left << user.nama_lengkap << " |\n";
        cout << "+------------------------------------------------+\n";
        cout << "| 1. Donasi ke project                           |\n";
        cout << "| 2. Buat/Lihat Project                          |\n";
        cout << "| 3. Lihat Riwayat Donasi                        |\n";
        cout << "| 4. Logout                                      |\n";
        cout << "+------------------------------------------------+\n";
        cout << "Pilih menu: ";
        cin >> pilihan;

        if (pilihan == "1") donasiKeProject(user.username);
        else if (pilihan == "2") {
            string sub;
            cout << "\n--- MENU CREATOR ---\n";
            cout << "1. Buat Project\n2. Lihat Project Saya\nKetik -1 untuk kembali\nPilih: ";
            cin >> sub;
            if (sub == "-1") continue;
            else if (sub == "1") buatProject(user);
            else if (sub == "2") lihatProjectSaya(user);
            else cout << "❌ Pilihan tidak valid.\n";
        }
        else if (pilihan == "3") lihatLogDonasi(user.username);
        else if (pilihan == "4") {
            cout << "👋 Logout berhasil.\n";
            break;
        }
        else cout << "❌ Pilihan tidak valid.\n";
    }
}

// Fungsi registrasi
void registrasi() {
    string nama, tgl, username, password, pesan_error;
    cout << "\n--- Registrasi Akun ---\n";
    cin.ignore();
    cout << "Nama Lengkap: "; getline(cin, nama);
    do {
        cout << "Tanggal Lahir (DD-MM-YYYY) (ketik -1 untuk batal): "; 
        getline(cin, tgl);
        if (tgl == "-1") {
            cout << "❌ Registrasi dibatalkan.\n";
            return;
        }
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
    cout << "✅ Registrasi berhasil! Silakan login.\n";
}

// Fungsi login
void login() {
    string username, password;
    cout << "\n--- Login ---\n";
    cin.ignore();
    cout << "Username (ketik -1 untuk batal): ";
    getline(cin, username);
    if (username == "-1") return;

    cout << "Password: ";
    getline(cin, password);

    if (akun.find(username) == akun.end()) {
        cout << "❌ Username tidak ditemukan.\n";
        return;
    }
    if (akun[username].hashed_password != hashPassword(password)) {
        cout << "❌ Password salah.\n";
        return;
    }

    cout << "✅ Login berhasil!\n";
    homeMenu(akun[username]);
}

// Menu utama
void menuUtama() {
    string pilihan;
    do {
        cout << "\n=== Menu Crowdfunding ===\n";
        cout << "1. Registrasi\n2. Login\n3. Keluar\nPilih menu: ";
        cin >> pilihan;
        if (pilihan == "1") registrasi();
        else if (pilihan == "2") login();
        else if (pilihan == "3") cout << "👋 Terima kasih telah menggunakan sistem crowdfunding!\n";
        else cout << "❌ Pilihan tidak valid.\n";
    } while (pilihan != "3");
}

int main() {
    muatAkunDariFile();
    muatProjectDariFile();
    muatLogDonasiDariFile();  
    menuUtama();
    return 0;
}    
