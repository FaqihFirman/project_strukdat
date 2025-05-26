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

// Implementasi Stack dari scratch untuk Donasi
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
    
    // Destruktor untuk membersihkan memory
    ~StackDonasi() {
        clear();
    }
    
    // Copy constructor
    StackDonasi(const StackDonasi& other) : top_node(nullptr), size_count(0) {
        if (other.top_node != nullptr) {
            // Buat array sementara untuk menyimpan data
            Donasi* temp_array = new Donasi[other.size_count];
            Node* current = other.top_node;
            int index = other.size_count - 1;
            
            // Ambil data dari stack lain dalam urutan terbalik
            while (current != nullptr) {
                temp_array[index--] = current->data;
                current = current->next;
            }
            
            // Push data ke stack ini dalam urutan yang benar
            for (int i = 0; i < other.size_count; i++) {
                push(temp_array[i]);
            }
            
            delete[] temp_array;
        }
    }
    
    // Assignment operator
    StackDonasi& operator=(const StackDonasi& other) {
        if (this != &other) {
            clear();
            if (other.top_node != nullptr) {
                Donasi* temp_array = new Donasi[other.size_count];
                Node* current = other.top_node;
                int index = other.size_count - 1;
                
                while (current != nullptr) {
                    temp_array[index--] = current->data;
                    current = current->next;
                }
                
                for (int i = 0; i < other.size_count; i++) {
                    push(temp_array[i]);
                }
                
                delete[] temp_array;
            }
        }
        return *this;
    }
    
    void push(const Donasi& donasi) {
        Node* new_node = new Node(donasi);
        new_node->next = top_node;
        top_node = new_node;
        size_count++;
    }
    
    void pop() {
        if (top_node != nullptr) {
            Node* temp = top_node;
            top_node = top_node->next;
            delete temp;
            size_count--;
        }
    }
    
    Donasi top() const {
        if (top_node != nullptr) {
            return top_node->data;
        }
        // Return empty donasi if stack is empty
        return {"", "", 0.0, ""};
    }
    
    bool empty() const {
        return top_node == nullptr;
    }
    
    int size() const {
        return size_count;
    }
    
    void clear() {
        while (!empty()) {
            pop();
        }
    }
    
    // Method untuk mengambil semua data dalam bentuk vector (untuk tampilan)
    vector<Donasi> getAllData() const {
        vector<Donasi> result;
        Node* current = top_node;
        while (current != nullptr) {
            result.push_back(current->data);
            current = current->next;
        }
        return result;
    }
};

// Data utama
unordered_map<string, User> akun;
vector<Project> daftarProject;

// Stack log donasi per user (username -> custom stack donasi)
unordered_map<string, StackDonasi> logDonasiUser;

// Fungsi untuk mendapatkan timestamp saat ini
string getCurrentTimestamp() {
    auto now = chrono::system_clock::now();
    time_t time_now = chrono::system_clock::to_time_t(now);
    
    stringstream ss;
    ss << put_time(localtime(&time_now), "%d-%m-%Y %H:%M:%S");
    return ss.str();
}

// Fungsi hashing password sederhana
string hashPassword(const string& password) {
    hash<string> hasher;
    return to_string(hasher(password));
}

// Generate ID project otomatis
string generateProjectID() {
    int nextID = daftarProject.size() + 1;
    stringstream ss;
    ss << "PRJ" << setfill('0') << setw(3) << nextID;
    return ss.str();
}

// Tampilkan progress bar donasi
void tampilkanBarProgress(double terkumpul, double target) {
    const int panjangBar = 50;
    double persentase = (terkumpul / target) * 100.0;
    int filled = static_cast<int>((persentase / 100.0) * panjangBar);
    cout << "Terkumpul " << fixed << setprecision(1) << persentase << "% : ";
    for (int i = 0; i < filled; ++i) cout << '|';
    for (int i = filled; i < panjangBar; ++i) cout << '-';
    cout << "\n";
}

// Validasi tahun kabisat
bool isKabisat(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Validasi format tanggal DD-MM-YYYY
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
        getline(ss, p.id_project, ','); getline(ss, p.nama_project, ','); getline(ss, p.deskripsi, ',');
        getline(ss, p.nama_pembuat, ','); getline(ss, target, ','); getline(ss, terkumpul, ',');
        p.target_dana = stod(target); p.dana_terkumpul = stod(terkumpul);
        daftarProject.push_back(p);
    }
    file.close();
}

// Simpan project ke file
void simpanProjectKeFile() {
    ofstream file("project.txt");
    for (const auto& p : daftarProject)
        file << p.id_project << "," << p.nama_project << "," << p.deskripsi << "," << p.nama_pembuat << ","
             << p.target_dana << "," << p.dana_terkumpul << "\n";
    file.close();
}

// Load log donasi dari file
void muatLogDonasiDariFile() {
    ifstream file("log_donasi.txt");
    string baris;
    
    // Temporary storage untuk mengurutkan data per user
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
            Donasi d = {id_project, nama_project, stod(nominal_str), timestamp};
            tempUserDonasi[username].push_back(d);
        }
    }
    file.close();
    
    // Push ke stack dalam urutan yang benar (dari yang terlama ke terbaru)
    for (auto& [username, donasiList] : tempUserDonasi) {
        for (const auto& donasi : donasiList) {
            logDonasiUser[username].push(donasi);
        }
    }
}

// Simpan log donasi ke file
void simpanLogDonasiKeFile() {
    ofstream file("log_donasi.txt");
    
    // Untuk setiap user, ambil semua donasi dari custom stack dan simpan
    for (auto& [username, stackDonasi] : logDonasiUser) {
        vector<Donasi> donasiList = stackDonasi.getAllData();
        
        // Simpan ke file dari yang terlama ke terbaru (reverse dari stack order)
        for (int i = donasiList.size() - 1; i >= 0; i--) {
            const Donasi& d = donasiList[i];
            file << username << "," << d.id_project << "," << d.nama_project << "," 
                 << d.nominal << "," << d.timestamp << "\n";
        }
    }
    file.close();
}

// Buat project baru
void buatProject(const User& user) {
    cin.ignore();
    Project p;
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

// Lihat project yang dibuat user
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
        cout << "\n+-----+--------------------------------+------------------+------------------+\n";
        cout << "| No. |          Nama Project          |   Target Dana    |   Dana Terkumpul |\n";
        cout << "+-----+--------------------------------+------------------+------------------+\n";
        int no = 1;
        for (const auto& p : daftarProject) {
            if (p.nama_pembuat == user.nama_lengkap) {
                cout << "| " << setw(3) << no++ << " | " 
                     << setw(30) << left << p.nama_project << " | "
                     << setw(16) << right << fixed << setprecision(2) << p.target_dana << " | "
                     << setw(16) << right << fixed << setprecision(2) << p.dana_terkumpul << " |\n";
            }
        }
        cout << "+-----+--------------------------------+------------------+------------------+\n";
    }
}

// Lihat semua project
void lihatSemuaProject() {
    cout << "\n===== DAFTAR PROJECT TERSEDIA =====\n";
    
    if (daftarProject.empty()) {
        cout << "⚠️ Belum ada project yang tersedia.\n";
        return;
    }
    
    cout << "+-----+----------+---------------------------+------------------+------------------+------------------+\n";
    cout << "| No. |    ID    |       Nama Project        |    Pembuat       |   Target Dana    |   Dana Terkumpul |\n";
    cout << "+-----+----------+---------------------------+------------------+------------------+------------------+\n";
    
    for (size_t i = 0; i < daftarProject.size(); ++i) {
        const auto& p = daftarProject[i];
        cout << "| " << setw(3) << (i + 1) << " | "
             << setw(8) << left << p.id_project << " | "
             << setw(25) << left << p.nama_project << " | "
             << setw(16) << left << p.nama_pembuat << " | "
             << setw(16) << right << fixed << setprecision(2) << p.target_dana << " | "
             << setw(16) << right << fixed << setprecision(2) << p.dana_terkumpul << " |\n";
        
        // Tampilkan progress bar setelah baris tabel
        cout << "|     |          | ";
        const int panjangBar = 25;
        double persentase = (p.dana_terkumpul / p.target_dana) * 100.0;
        int filled = static_cast<int>((persentase / 100.0) * panjangBar);
        for (int j = 0; j < filled; ++j) cout << '|';
        for (int j = filled; j < panjangBar; ++j) cout << '-';
        cout << " | " << setw(16) << left << (to_string((int)persentase) + "%") 
             << " |                  |                  |\n";
    }
    cout << "+-----+----------+---------------------------+------------------+------------------+------------------+\n";
}

// Donasi ke project dan simpan log donasi dengan timestamp di custom stack user
void donasiKeProject(const string& username) {
    lihatSemuaProject();
    int pilihan;
    cout << "\nMasukkan nomor project yang ingin Anda donasi: ";
    cin >> pilihan;
    if (pilihan < 1 || pilihan > (int)daftarProject.size()) {
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

    // Simpan log donasi ke custom stack user dengan timestamp
    string timestamp = getCurrentTimestamp();
    Donasi d = {p.id_project, p.nama_project, nominal, timestamp};
    logDonasiUser[username].push(d);
    
    // Simpan log donasi ke file
    simpanLogDonasiKeFile();

    cout << "✅ Terima kasih! Donasi Anda telah diterima pada " << timestamp << "\n";
    tampilkanBarProgress(p.dana_terkumpul, p.target_dana);
}

// Tampilkan riwayat donasi user dari custom stack dengan timestamp
void lihatLogDonasi(const string& username) {
    cout << "\n===== RIWAYAT DONASI ANDA =====\n";
    if (logDonasiUser.find(username) == logDonasiUser.end() || logDonasiUser[username].empty()) {
        cout << "⚠️ Anda belum melakukan donasi apapun.\n";
        return;
    }
    
    // Ambil semua data donasi dari custom stack
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
    cout << "| Total donasi yang telah Anda berikan: " << setw(2) << donasiList.size() << " kali" << setw(26) << " |\n";
    cout << "| Menggunakan Custom Stack Implementation (bukan STL)" << setw(19) << " |\n";
    cout << "+-----+---------------------+--------------------------+----------------+\n";
}

// Menu home setelah login
void homeMenu(const User& user) {
    int pilihan;
    do {
        cout << "\n+================================================+\n";
        cout << "|                  HOME MENU                     |\n";
        cout << "+================================================+\n";
        cout << "| Halo, " << setw(38) << left << user.nama_lengkap << " |\n";
        cout << "| Apa yang ingin Anda lakukan?                   |\n";
        cout << "+------------------------------------------------+\n";
        cout << "| 1. Donatur - Donasi ke project                 |\n";
        cout << "| 2. Creator - Buat/Lihat project                |\n";
        cout << "| 3. Lihat Riwayat Donasi                        |\n";
        cout << "| 4. Logout                                      |\n";
        cout << "+------------------------------------------------+\n";
        cout << "Pilih menu: ";
        cin >> pilihan;

        switch (pilihan) {
            case 1:
                donasiKeProject(user.username);
                break;
            case 2: {
                int sub;
                cout << "\n+----------------------------------------+\n";
                cout << "|             MENU CREATOR               |\n";
                cout << "+----------------------------------------+\n";
                cout << "| 1. Buat Project                        |\n";
                cout << "| 2. Lihat Project Saya                  |\n";
                cout << "+----------------------------------------+\n";
                cout << "Pilih: ";
                cin >> sub;
                if (sub == 1) buatProject(user);
                else if (sub == 2) lihatProjectSaya(user);
                else cout << "❌ Pilihan tidak valid.\n";
                break;
            }
            case 3:
                lihatLogDonasi(user.username);
                break;
            case 4:
                cout << "👋 Logout berhasil.\n";
                break;
            default:
                cout << "❌ Pilihan tidak valid.\n";
        }
    } while (pilihan != 4);
}

// Fungsi registrasi
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
    cout << "✅ Registrasi berhasil! Silakan login.\n";
}

// Fungsi login
void login() {
    string username, password;
    cout << "\n--- Login ---\n";
    cin.ignore();
    cout << "Username: "; getline(cin, username);
    cout << "Password: "; getline(cin, password);
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
    muatLogDonasiDariFile();  // Load log donasi saat startup
    menuUtama();
    return 0;
}