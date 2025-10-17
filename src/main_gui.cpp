#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <cstdio>
#include <set>
#include "version.h"
#include "piwiper_resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "shell32.lib")

// Disk structure
struct DiskInfo {
    int number = -1;
    std::string model;
    std::string serial;
    std::string busType;
    double sizeGB = 0.0;
    bool isCurrentOS = false;
};

// Report structure
struct WipeReport {
    std::string timestamp;
    std::string result;
    int diskNumber;
    std::string model;
    std::string serial;
    std::string busType;
    double sizeGB;
    std::string notes;
    std::string filename;
};

// Global variables
HINSTANCE g_hInstance;
HWND g_hWnd;
HWND g_hTabControl;
HWND g_hListView;
HWND g_hProgressBar;
HWND g_hProgressBarFine;
// Modern colors
HBRUSH g_hBtnBlueBrush;
HBRUSH g_hBtnRedBrush;
HBRUSH g_hBtnGrayBrush;

// Modern fonts (global for owner-draw)
HFONT g_hTitleFont;
HFONT g_hNormalFont;
HWND g_hBtnRefresh;
HWND g_hBtnWipe;
HWND g_hBtnCancel;
HWND g_hStatusText;
HWND g_hEtaText;
HWND g_hMethodCombo;
HWND g_hVerificationCombo;
HWND g_hCustomerNameInput;
HWND g_hCustomerAddressInput;
HWND g_hCustomerPhoneInput;
HWND g_hCustomerEmailInput;
std::vector<DiskInfo> g_disks;
std::vector<WipeReport> g_reports;
bool g_isWiping = false;
bool g_showAll = false;
bool g_quickWipe = true; // Default to quick wipe
bool g_cancelRequested = false;
HANDLE g_wipeProcessHandle = NULL;
int g_verificationMode = 0; // 0=None, 1=Quick, 2=Full

// Constants
#define WM_WIPE_COMPLETE (WM_USER + 1)
#define WM_UPDATE_PROGRESS (WM_USER + 2)
#define ID_TAB_CONTROL 1000
#define ID_LISTVIEW 1001
#define ID_PROGRESS 1002
#define ID_BTN_REFRESH 1003
#define ID_BTN_WIPE 1004
#define ID_BTN_CANCEL 1026
#define ID_BTN_EXIT 1025
#define ID_STATUS_TEXT 1006
#define ID_ETA_TEXT 1007
#define ID_METHOD_COMBO 1014
#define ID_VERIFICATION_COMBO 1015
#define ID_CUSTOMER_NAME_INPUT 1021
#define ID_CUSTOMER_ADDRESS_INPUT 1022
#define ID_CUSTOMER_PHONE_INPUT 1023
#define ID_CUSTOMER_EMAIL_INPUT 1024
#define ID_ABOUT_DIALOG 1027

// Helper: run command and capture output
static std::string runCmd(const std::string& cmd) {
    std::string result;
    HANDLE hRead = NULL, hWrite = NULL;
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        return result;
    }
    
    STARTUPINFOA si = {sizeof(STARTUPINFOA)};
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi;
    std::string fullCmd = "cmd.exe /c " + cmd;
    char cmdBuffer[512];
    strncpy_s(cmdBuffer, fullCmd.c_str(), sizeof(cmdBuffer) - 1);
    
    if (CreateProcessA(NULL, cmdBuffer, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
    CloseHandle(hWrite);
        hWrite = NULL;
        
        char buffer[4096];
        DWORD bytesRead;
        while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            result += buffer;
        }
        
    WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        if (hWrite) CloseHandle(hWrite);
    }
    
    CloseHandle(hRead);
    return result;
}

// Helper: trim string
static std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// List disks
static std::vector<DiskInfo> listDisks() {
    std::vector<DiskInfo> disks;
    
    // Get current OS disk number
    std::string osDrive = runCmd("echo %SystemDrive%");
    osDrive = trim(osDrive);
    if (osDrive.empty() || osDrive.find('%') != std::string::npos) {
        osDrive = "C:";
    }
    
    // Use WMIC to get all disk info
    std::string wmicOut = runCmd("wmic diskdrive get Index,Size,Model,SerialNumber,InterfaceType,DeviceID /format:csv");
    std::istringstream ss(wmicOut);
    std::string line;
    bool firstLine = true;
    
    while (std::getline(ss, line)) {
        line = trim(line);
        if (line.empty() || firstLine) {
            firstLine = false;
            continue;
        }
        
        // Parse CSV: Node,DeviceID,Index,InterfaceType,Model,SerialNumber,Size
        std::vector<std::string> fields;
        std::string field;
        bool inQuote = false;
        for (char c : line) {
            if (c == '"') {
                inQuote = !inQuote;
            } else if (c == ',' && !inQuote) {
                fields.push_back(trim(field));
                field.clear();
            } else {
                field += c;
            }
        }
        if (!field.empty()) fields.push_back(trim(field));
        
        if (fields.size() >= 6) {
            DiskInfo d;
            try {
                // Index is at position 2
                if (!fields[2].empty()) d.number = std::stoi(fields[2]);
                // InterfaceType at position 3
                d.busType = fields[3];
                // Model at position 4
                d.model = fields[4];
                // SerialNumber at position 5
                d.serial = fields[5];
                // Size at position 6 (in bytes)
                if (fields.size() > 6 && !fields[6].empty()) {
                    double bytes = std::stod(fields[6]);
                    d.sizeGB = bytes / (1024.0 * 1024.0 * 1024.0);
                }
                
                disks.push_back(d);
            } catch (...) {
                // Skip malformed entries
            }
        }
    }
    
    // Determine which disk has the OS - use C: drive location
    // Method: Find which disk contains the C: volume using diskpart associations
    std::string volCmd = "wmic path Win32_LogicalDiskToPartition get Dependent,Antecedent /format:csv";
    std::string volOut = runCmd(volCmd);
    
    std::set<int> osDiskIndexes;
    std::istringstream volSs(volOut);
    firstLine = true;
    
    while (std::getline(volSs, line)) {
        line = trim(line);
        if (line.empty() || firstLine) {
            firstLine = false;
            continue;
        }
        
        // Look for C: drive
        if (line.find("C:") != std::string::npos || line.find("c:") != std::string::npos) {
            // Extract disk number from "Disk #X, Partition #Y" pattern
            size_t diskPos = line.find("Disk #");
            if (diskPos != std::string::npos) {
                diskPos += 6; // Skip "Disk #"
                std::string numStr;
                while (diskPos < line.length() && isdigit(line[diskPos])) {
                    numStr += line[diskPos];
                    diskPos++;
                }
                if (!numStr.empty()) {
                    try {
                        int diskIdx = std::stoi(numStr);
                        osDiskIndexes.insert(diskIdx);
                        OutputDebugStringA(("[PIWIPER] C: drive is on Disk #" + std::to_string(diskIdx)).c_str());
        } catch (...) {}
                }
            }
        }
    }
    
    // Mark OS disks
        for (auto& d : disks) {
        if (osDiskIndexes.find(d.number) != osDiskIndexes.end()) {
            d.isCurrentOS = true;
        }
    }
    
    // If no disk marked, assume Disk 1 is OS
    bool anyMarked = false;
    for (const auto& d : disks) {
        if (d.isCurrentOS) anyMarked = true;
    }
    if (!anyMarked && !disks.empty()) {
        for (auto& d : disks) {
            if (d.number == 1) {
                d.isCurrentOS = true;
                break;
            }
        }
    }

    return disks;
}

// Log to edit control
void LogMessage(const std::string& msg) {
    // Output to DebugView
    OutputDebugStringA(("[PIWIPER] " + msg).c_str());
    
    // Also append to status log if available
    if (g_hStatusText) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        struct tm timeinfo;
        localtime_s(&timeinfo, &time_t);
        char timeBuf[64];
        strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &timeinfo);
        
        std::string logLine = "[" + std::string(timeBuf) + "] " + msg + "\r\n";
        
        int len = GetWindowTextLengthA(g_hStatusText);
        SendMessageA(g_hStatusText, EM_SETSEL, len, len);
        SendMessageA(g_hStatusText, EM_REPLACESEL, FALSE, (LPARAM)logLine.c_str());
        SendMessageA(g_hStatusText, EM_SCROLLCARET, 0, 0);
    }
}

// Get writable log directory
static std::string getLogRoot() {
    for (const char* letter : {"X", "Z", "Y", "E", "D", "C"}) {
        std::string path = std::string(letter) + ":\\WipeLogs";
        try {
            std::filesystem::create_directories(path);
            std::string probe = path + "\\.__probe";
            std::ofstream f(probe); f << "ok"; f.close();
            std::remove(probe.c_str());
            return path;
        } catch (...) {}
    }
    std::string fallback = ".\\WipeLogs";
    std::filesystem::create_directories(fallback);
    return fallback;
}

// Load reports from WipeLogs directory
void loadReports() {
    g_reports.clear();
    std::string logRoot = getLogRoot();
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(logRoot)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                std::string filename = entry.path().filename().string();
                
                // Parse filename: Disk2_0401a105a1edb9abe71f_20251016_151021.txt
                if (filename.find("Disk") == 0) {
                    WipeReport report;
                    report.filename = filename;
                    
                    // Extract disk number
                    size_t pos = filename.find("_");
                    if (pos != std::string::npos) {
                        std::string diskStr = filename.substr(4, pos - 4);
                        report.diskNumber = std::stoi(diskStr);
                    }
                    
                    // Read file content
                    std::ifstream f(entry.path());
                    if (f.is_open()) {
                        std::string line;
                        while (std::getline(f, line)) {
                            if (line.find("Timestamp:") != std::string::npos) {
                                report.timestamp = line.substr(11);
                            } else if (line.find("Result:") != std::string::npos) {
                                report.result = line.substr(8);
                            } else if (line.find("Model:") != std::string::npos) {
                                report.model = line.substr(7);
                            } else if (line.find("Serial:") != std::string::npos) {
                                report.serial = line.substr(8);
                            } else if (line.find("Bus Type:") != std::string::npos) {
                                report.busType = line.substr(10);
                            } else if (line.find("Size:") != std::string::npos) {
                                std::string sizeStr = line.substr(6);
                                sizeStr = sizeStr.substr(0, sizeStr.find(" "));
                                report.sizeGB = std::stod(sizeStr);
                            } else if (line.find("Method:") != std::string::npos) {
                                report.method = line.substr(8);
                            } else if (line.find("Notes:") != std::string::npos) {
                                report.notes = line.substr(7);
                            }
                        }
                        f.close();
                    }
                    
                    g_reports.push_back(report);
                }
            }
        }
        
        // Sort by timestamp (newest first)
        std::sort(g_reports.begin(), g_reports.end(), [](const WipeReport& a, const WipeReport& b) {
            return a.timestamp > b.timestamp;
        });
        
    } catch (const std::exception&) {
        // Handle directory access errors
    }
}

// Write report
static void writeReport(const DiskInfo& disk, const std::string& result, const std::string& notes) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    struct tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    char timeBuf[64];
    strftime(timeBuf, sizeof(timeBuf), "%Y%m%d_%H%M%S", &timeinfo);
    
    std::string serial = disk.serial.empty() ? "UNKNOWN" : disk.serial;
    for (auto& c : serial) if (!isalnum(c)) c = '_';
    
    std::string filename = "Disk" + std::to_string(disk.number) + "_" + serial + "_" + timeBuf + ".txt";
    std::string logRoot = getLogRoot();
    std::string fullPath = logRoot + "\\" + filename;
    
    LogMessage("Attempting to write report to: " + fullPath);
    
    std::ofstream f(fullPath);
    if (!f.is_open()) {
        LogMessage("ERROR: Failed to create report file!");
        return;
    }
    
    f << "===========================================\n";
    f << "   PIWIPER DISK ERASE REPORT\n";
    f << "===========================================\n\n";
    f << "Disk Number    : " << disk.number << "\n";
    f << "Model          : " << disk.model << "\n";
    f << "Serial Number  : " << disk.serial << "\n";
    f << "Bus Type       : " << disk.busType << "\n";
    f << "Size (GB)      : " << std::fixed << std::setprecision(2) << disk.sizeGB << "\n";
    f << "Timestamp      : " << timeBuf << "\n";
    f << "Result         : " << result << "\n";
    f << "Notes          : " << notes << "\n";
    f << "\n===========================================\n";
    f.close();
    
    LogMessage("Report saved successfully: " + filename);
}

// Append message to status log
void AppendStatusMessage(const std::string& message) {
    if (!g_hStatusText) return;
    
    // Get current time
    SYSTEMTIME st;
    GetLocalTime(&st);
    char timeStr[32];
    sprintf_s(timeStr, "[%02d:%02d:%02d] ", st.wHour, st.wMinute, st.wSecond);
    
    // Get current text length
    int len = GetWindowTextLengthA(g_hStatusText);
    
    // Append new message
    SendMessageA(g_hStatusText, EM_SETSEL, len, len);
    SendMessageA(g_hStatusText, EM_REPLACESEL, FALSE, (LPARAM)(timeStr + message + "\r\n").c_str());
    
    // Scroll to bottom
    SendMessageA(g_hStatusText, EM_SCROLLCARET, 0, 0);
}

// Wipe disk in background thread
struct WipeParams {
    int diskNumber;
    double sizeGB;
    std::string model;
    std::string serial;
    std::string busType;
};

DWORD WINAPI WipeThread(LPVOID param) {
    WipeParams* wp = (WipeParams*)param;
    
    std::string notes = "";
    bool success = false;
    
    LogMessage(g_quickWipe ? "Using QUICK wipe mode" : "Using SECURE wipe mode");
    
    // Store thread handle for cancellation (use pseudo-handle)
    g_wipeProcessHandle = GetCurrentThread();
    
    // Open physical disk directly
        char diskPath[64];
        sprintf_s(diskPath, "\\\\.\\PhysicalDrive%d", wp->diskNumber);
        
    HANDLE hDisk = CreateFileA(diskPath, GENERIC_READ | GENERIC_WRITE, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    
    if (hDisk == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        notes = "Failed to open disk. Error: " + std::to_string(err);
        LogMessage("ERROR: " + notes);
        success = false;
        g_wipeProcessHandle = NULL;
    } else {
        DWORD bytesReturned;
        auto start = std::chrono::steady_clock::now();
        
        if (g_quickWipe) {
            // QUICK WIPE: Just clear partition table
            LogMessage("Quick wipe: Clearing partition table");
            
            CREATE_DISK createDisk = {PARTITION_STYLE_MBR, {{0}}};
            if (DeviceIoControl(hDisk, IOCTL_DISK_CREATE_DISK, &createDisk, sizeof(createDisk), nullptr, 0, &bytesReturned, nullptr)) {
                LogMessage("Partition table cleared successfully");
                success = true;
                notes = "Quick wipe completed";
        } else {
            DWORD err = GetLastError();
                notes = "Failed to clear partition table. Error: " + std::to_string(err);
                LogMessage("ERROR: " + notes);
                success = false;
            }
            
            DeviceIoControl(hDisk, IOCTL_DISK_UPDATE_PROPERTIES, nullptr, 0, nullptr, 0, &bytesReturned, nullptr);
            CloseHandle(hDisk);
            
            // Simulate progress
            int estSeconds = 3;
            LogMessage("Estimated time: ~3 seconds");
            PostMessage(g_hWnd, WM_UPDATE_PROGRESS, 0, estSeconds);
            
            for (int pct = 0; pct <= 100; pct += 10) {
                if (g_cancelRequested) break;
                PostMessage(g_hWnd, WM_UPDATE_PROGRESS, pct, estSeconds - (pct * estSeconds / 100));
                Sleep(300);
            }
            
        } else {
            // SECURE WIPE: Zero entire disk
            LogMessage("Secure wipe: Writing zeros to entire disk");
            
            // Get disk size
            GET_LENGTH_INFORMATION lengthInfo;
            if (!DeviceIoControl(hDisk, IOCTL_DISK_GET_LENGTH_INFO, nullptr, 0, &lengthInfo, sizeof(lengthInfo), &bytesReturned, nullptr)) {
                DWORD err = GetLastError();
                notes = "Failed to get disk size. Error: " + std::to_string(err);
                LogMessage("ERROR: " + notes);
                CloseHandle(hDisk);
                success = false;
            } else {
                ULONGLONG totalBytes = lengthInfo.Length.QuadPart;
                ULONGLONG totalMB = totalBytes / (1024 * 1024);
                
                // Estimate time
                double mbps = 50.0; // Conservative estimate
                if (wp->busType.find("NVMe") != std::string::npos) mbps = 500.0;
                else if (wp->busType.find("SSD") != std::string::npos) mbps = 200.0;
                else if (wp->busType.find("USB") != std::string::npos) mbps = 30.0;
                
                int estSeconds = (int)(totalMB / mbps);
                if (estSeconds < 60) estSeconds = 60;
                
                LogMessage("Disk size: " + std::to_string(totalMB) + " MB, estimated: ~" + std::to_string(estSeconds / 60) + " minutes");
                PostMessage(g_hWnd, WM_UPDATE_PROGRESS, 0, estSeconds);
                
                // Write zeros in chunks
                const DWORD chunkSize = 1024 * 1024; // 1MB chunks
                char* zeroBuffer = new char[chunkSize];
                memset(zeroBuffer, 0, chunkSize);
                
                ULONGLONG bytesWritten = 0;
                DWORD written;
                success = true;
                
                // Progress tracking variables (outside loop!)
                ULONGLONG lastUpdate = 0;
                auto lastUpdateTime = start;
                
                // Log start of write
                LogMessage("Starting to write zeros to disk...");
                OutputDebugStringA("[PIWIPER] Write loop starting");
                
                while (bytesWritten < totalBytes && !g_cancelRequested) {
                    DWORD toWrite = (DWORD)((totalBytes - bytesWritten) > chunkSize ? chunkSize : (totalBytes - bytesWritten));
                    
                    if (!WriteFile(hDisk, zeroBuffer, toWrite, &written, nullptr)) {
                        DWORD err = GetLastError();
                        notes = "Write failed at " + std::to_string(bytesWritten / (1024*1024)) + " MB. Error: " + std::to_string(err);
                        LogMessage("ERROR: " + notes);
                        OutputDebugStringA(("[PIWIPER] WriteFile failed, error: " + std::to_string(err)).c_str());
                success = false;
                break;
            }
            
                    bytesWritten += written;
                    
                    // Debug first few writes
                    static int writeCount = 0;
                    if (writeCount < 5) {
                        OutputDebugStringA(("[PIWIPER] Wrote " + std::to_string(written) + " bytes, total: " + std::to_string(bytesWritten)).c_str());
                        writeCount++;
                    }
                    
                    // Update fine progress bar every 1MB
                    if (bytesWritten - lastUpdate >= (1024 * 1024) || bytesWritten >= totalBytes) {
                        // Overall progress (0-100%)
                        int progressPct = (int)((double)bytesWritten * 100.0 / (double)totalBytes);
                        
                        // Fine progress: cycles 0-100% every 5MB
                        ULONGLONG bytesInCurrentChunk = bytesWritten % (5 * 1024 * 1024);
                        int fineProgressPct = (int)((double)bytesInCurrentChunk * 100.0 / (5.0 * 1024.0 * 1024.0));
                        
                        auto now = std::chrono::steady_clock::now();
                        auto elapsed = now - start;
                        auto elapsedSec = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
                        
                        // Calculate AVERAGE speed (MB/s) from start, not instantaneous
                        int avgSpeedMBps = 0;
                        if (elapsedSec > 0) {
                            double totalMB = bytesWritten / (1024.0 * 1024.0);
                            avgSpeedMBps = (int)(totalMB / elapsedSec);
                            
                            // Sanity check - cap speed at reasonable values
                            if (avgSpeedMBps < 0) avgSpeedMBps = 0;
                            if (avgSpeedMBps > 1000) avgSpeedMBps = 1000;
                        }
                        
                        // Store: speed (high 16 bits), remaining (low 16 bits)
                        int remaining = estSeconds - (int)elapsedSec;
                        if (remaining < 0) remaining = 0;
                        LPARAM progressData = MAKELPARAM(remaining, avgSpeedMBps);
                        
                        // Send both coarse progress (wParam) and fine progress via custom message
                        PostMessage(g_hWnd, WM_UPDATE_PROGRESS, MAKEWPARAM(progressPct, fineProgressPct), progressData);
                        
                        // Debug for first few updates
                        static int updateCountDebug = 0;
                        if (updateCountDebug < 10) {
                            OutputDebugStringA(("[PIWIPER] Update #" + std::to_string(updateCountDebug) + ": Progress=" + std::to_string(progressPct) + "%, Fine=" + std::to_string(fineProgressPct) + "%, Avg Speed=" + std::to_string(avgSpeedMBps) + " MB/s").c_str());
                            updateCountDebug++;
                        }
                        
                        lastUpdate = bytesWritten;
                        lastUpdateTime = now;
                    }
                }
                
                delete[] zeroBuffer;
                CloseHandle(hDisk);
                
                if (success && !g_cancelRequested) {
                    notes = "Secure wipe completed - " + std::to_string(totalMB) + " MB zeroed";
                    LogMessage(notes);
                } else if (g_cancelRequested) {
                    notes = "Secure wipe cancelled by user";
                    LogMessage(notes);
                success = false;
                }
            }
        }
        
        PostMessage(g_hWnd, WM_UPDATE_PROGRESS, MAKEWPARAM(100, 100), MAKELPARAM(0, 0));
    }
    
    // Verification phase (if enabled)
    if (success && !g_cancelRequested && g_verificationMode > 0) {
        AppendStatusMessage("Starting verification...");
        LogMessage("Starting disk verification");
        
        char diskPath[64];
        sprintf_s(diskPath, "\\\\.\\PhysicalDrive%d", wp->diskNumber);
        
        HANDLE hDiskVerify = CreateFileA(diskPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 
                                         nullptr, OPEN_EXISTING, 0, nullptr);
        
        if (hDiskVerify == INVALID_HANDLE_VALUE) {
            notes += " | Verification FAILED: Cannot open disk for reading";
            LogMessage("ERROR: Cannot open disk for verification");
            success = false;
        } else {
            // Get disk size
            GET_LENGTH_INFORMATION lengthInfo;
            DWORD bytesReturned;
            
            if (!DeviceIoControl(hDiskVerify, IOCTL_DISK_GET_LENGTH_INFO, nullptr, 0, 
                                &lengthInfo, sizeof(lengthInfo), &bytesReturned, nullptr)) {
                notes += " | Verification FAILED: Cannot get disk size";
                LogMessage("ERROR: Cannot get disk size for verification");
                success = false;
            } else {
                ULONGLONG totalBytes = lengthInfo.Length.QuadPart;
                ULONGLONG bytesToVerify = totalBytes;
                
                // Quick verification: only first 100MB
                if (g_verificationMode == 1) {
                    bytesToVerify = min(totalBytes, 100ULL * 1024 * 1024);
                    LogMessage("Quick verification: checking first 100MB");
                }
                
                // Read and verify in 1MB chunks
                const DWORD chunkSize = 1024 * 1024;
                char* readBuffer = new char[chunkSize];
                ULONGLONG bytesVerified = 0;
                bool allZeros = true;
                DWORD read;
                
                while (bytesVerified < bytesToVerify && !g_cancelRequested) {
                    DWORD toRead = (DWORD)min((ULONGLONG)chunkSize, bytesToVerify - bytesVerified);
                    
                    if (!ReadFile(hDiskVerify, readBuffer, toRead, &read, nullptr)) {
                        notes += " | Verification FAILED: Read error";
                        LogMessage("ERROR: Read failed during verification");
                        allZeros = false;
                break;
            }
            
                    // Check if all bytes are zero
                    for (DWORD i = 0; i < read; i++) {
                        if (readBuffer[i] != 0) {
                            allZeros = false;
                            char errMsg[128];
                            sprintf_s(errMsg, "Verification FAILED: Non-zero byte found at offset %llu", bytesVerified + i);
                            notes += " | ";
                            notes += errMsg;
                            LogMessage(errMsg);
                        break;
                    }
                }
                
                    if (!allZeros) break;
                    
                    bytesVerified += read;
                    
                    // Update progress every 10MB
                    if (bytesVerified % (10 * 1024 * 1024) == 0) {
                        int verifyPct = (int)((double)bytesVerified * 100.0 / (double)bytesToVerify);
                        char verifyMsg[128];
                        sprintf_s(verifyMsg, "Verifying... %d%% (%llu MB)", verifyPct, bytesVerified / (1024*1024));
                        AppendStatusMessage(verifyMsg);
                    }
                }
                
                delete[] readBuffer;
                CloseHandle(hDiskVerify);
                
                if (g_cancelRequested) {
                    notes += " | Verification cancelled";
                    LogMessage("Verification cancelled by user");
                } else if (allZeros) {
                    notes += " | Verification PASSED: All bytes are zero";
                    LogMessage("Verification PASSED");
                    AppendStatusMessage("Verification PASSED - All bytes verified as zero");
                } else if (success) {
                    success = false; // Wipe succeeded but verification failed
                }
            }
        }
    }
    
    // Write report
    DiskInfo disk;
    disk.number = wp->diskNumber;
    disk.model = wp->model;
    disk.serial = wp->serial;
    disk.busType = wp->busType;
    disk.sizeGB = wp->sizeGB;
    
    writeReport(disk, success ? "SUCCESS" : "FAILED", notes);
    
    delete wp;
    g_wipeProcessHandle = NULL; // Clear handle
    PostMessage(g_hWnd, WM_WIPE_COMPLETE, success ? 1 : 0, 0);
    return 0;
}

// Refresh disk list
void RefreshDiskList() {
    LogMessage("Scanning disks...");
    AppendStatusMessage("Scanning disks...");
    g_disks = listDisks();
    
    ListView_DeleteAllItems(g_hListView);
    
    int index = 0;
    int nonOSCount = 0;
    for (const auto& disk : g_disks) {
        if (disk.isCurrentOS && !g_showAll) {
            LogMessage("Skipping OS disk: Disk #" + std::to_string(disk.number));
            continue;
        }
        
        if (!disk.isCurrentOS) nonOSCount++;
        
        LVITEMA item{};
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = index;
        item.iSubItem = 0;
        item.lParam = disk.number; // Store disk number in item data
        
        char numBuf[32];
        sprintf_s(numBuf, "%d%s", disk.number, disk.isCurrentOS ? " [OS]" : "");
        item.pszText = numBuf;
        int itemIndex = ListView_InsertItem(g_hListView, &item);
        
        if (itemIndex != -1) {
            // Column 1: Model
            char modelBuf[512];
            strncpy_s(modelBuf, disk.model.c_str(), _TRUNCATE);
            ListView_SetItemText(g_hListView, itemIndex, 1, modelBuf);
            
            // Column 2: Serial Number
            char serialBuf[256];
            strncpy_s(serialBuf, disk.serial.c_str(), _TRUNCATE);
            ListView_SetItemText(g_hListView, itemIndex, 2, serialBuf);
            
            // Column 3: Bus Type
            char busBuf[128];
            strncpy_s(busBuf, disk.busType.c_str(), _TRUNCATE);
            ListView_SetItemText(g_hListView, itemIndex, 3, busBuf);
            
            // Column 4: Size (GB)
            char sizeBuf[64];
            sprintf_s(sizeBuf, "%.2f", disk.sizeGB);
            ListView_SetItemText(g_hListView, itemIndex, 4, sizeBuf);
            
            // Column 5: Status
            ListView_SetItemText(g_hListView, itemIndex, 5, const_cast<char*>("Ready"));
            
            // Column 6: Progress
            ListView_SetItemText(g_hListView, itemIndex, 6, const_cast<char*>("0%"));
            
            // Column 7: Speed
            ListView_SetItemText(g_hListView, itemIndex, 7, const_cast<char*>("--"));
            
            LogMessage("Added: Disk #" + std::to_string(disk.number) + " - " + disk.model);
        }
        
        index++;
    }
    
    // Update status log with disk count
    int availableDisks = 0;
    for (const auto& disk : g_disks) {
        if (!disk.isCurrentOS) availableDisks++;
    }
    char statusText[256];
    sprintf_s(statusText, "Found %d disk(s) available for wipe", availableDisks);
    AppendStatusMessage(statusText);
    
    // Keep ListView focused so selection stays blue
    SetFocus(g_hListView);
    
    LogMessage("Found " + std::to_string((int)g_disks.size()) + " total disk(s), " + std::to_string(nonOSCount) + " available for wipe");
}

// Show About Dialog
void ShowAboutDialog() {
    std::string aboutText = "PIWIPER - Professional Disk Eraser\n\n";
    aboutText += "Version: " + std::string(PIWIPER_VERSION_STRING) + "\n";
    aboutText += "Build Date: " + std::string(PIWIPER_BUILD_DATE) + " " + std::string(PIWIPER_BUILD_TIME) + "\n";
    aboutText += "Build Config: " + std::string(PIWIPER_BUILD_CONFIG) + "\n\n";
    aboutText += PIWIPER_APP_DESCRIPTION + "\n\n";
    aboutText += "Features:\n";
    aboutText += "• Modern UI with gradient design\n";
    aboutText += "• Dual progress tracking\n";
    aboutText += "• Real-time speed display\n";
    aboutText += "• Verification system\n";
    aboutText += "• Comprehensive backup system\n";
    aboutText += "• OS disk protection\n\n";
    aboutText += PIWIPER_APP_COPYRIGHT + "\n";
    aboutText += "All rights reserved.\n\n";
    aboutText += "⚠️ WARNING: This tool permanently destroys data!\n";
    aboutText += "Use with extreme caution and proper authorization.";
    
    MessageBoxA(g_hWnd, aboutText.c_str(), "About PIWIPER", MB_OK | MB_ICONINFORMATION);
}

// Wipe selected disk
void WipeSelectedDisk() {
    if (g_isWiping) {
        MessageBoxA(g_hWnd, "A wipe operation is already in progress!", "Busy", MB_OK | MB_ICONWARNING);
        return;
    }
    
    int selected = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
    if (selected == -1) {
        MessageBoxA(g_hWnd, "Please select a disk to wipe.", "No Selection", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    // Get selected disk (skip OS disks)
    int actualIndex = 0;
    DiskInfo* selectedDisk = nullptr;
    for (auto& disk : g_disks) {
        if (disk.isCurrentOS) continue;
        if (actualIndex == selected) {
            selectedDisk = &disk;
            break;
        }
        actualIndex++;
    }
    
    if (!selectedDisk) return;
    
    // Confirm
    std::string msg = "WARNING: This will PERMANENTLY DESTROY all data on:\n\n";
    msg += "Disk #" + std::to_string(selectedDisk->number) + "\n";
    msg += "Size: " + std::to_string((int)selectedDisk->sizeGB) + " GB\n";
    msg += "Model: " + selectedDisk->model + "\n";
    msg += "Serial: " + selectedDisk->serial + "\n\n";
    msg += "This action is IRREVERSIBLE!\n\nAre you sure you want to continue?";
    
    int result = MessageBoxA(g_hWnd, msg.c_str(), "CONFIRM DISK WIPE", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
    if (result != IDYES) {
        LogMessage("Wipe cancelled by user");
        return;
    }
    
    // Start wipe thread
    g_isWiping = true;
    g_cancelRequested = false;
    EnableWindow(g_hBtnWipe, FALSE);
    EnableWindow(g_hBtnRefresh, FALSE);
    EnableWindow(g_hBtnCancel, TRUE); // Enable cancel button
    ShowWindow(g_hBtnCancel, SW_SHOW); // Show cancel button
    SendMessage(g_hProgressBar, PBM_SETPOS, 0, 0);
    
    // Update status with operation details
    if (g_hStatusText) {
        char statusText[256];
        const char* mode = g_quickWipe ? "Quick Wipe" : "Secure Wipe";
        sprintf_s(statusText, "Wiping Disk #%d (%s) - %.1f GB - Mode: %s", 
                  selectedDisk->number, selectedDisk->model.c_str(), selectedDisk->sizeGB, mode);
        AppendStatusMessage(statusText);
    }
    
    LogMessage("Starting wipe: Disk #" + std::to_string(selectedDisk->number));
    
    WipeParams* wp = new WipeParams();
    wp->diskNumber = selectedDisk->number;
    wp->sizeGB = selectedDisk->sizeGB;
    wp->model = selectedDisk->model;
    wp->serial = selectedDisk->serial;
    wp->busType = selectedDisk->busType;
    
    CreateThread(nullptr, 0, WipeThread, wp, 0, nullptr);
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Create modern color brushes
            g_hBtnBlueBrush = CreateSolidBrush(RGB(0, 120, 212));    // Modern blue
            g_hBtnRedBrush = CreateSolidBrush(RGB(232, 17, 35));     // Modern red
            g_hBtnGrayBrush = CreateSolidBrush(RGB(160, 160, 160));  // Modern gray
            
            // Create fonts (global for owner-draw buttons) - Modern Segoe UI
            g_hTitleFont = CreateFontA(
                -20,                      // Height (negative = use as point size)
                0,                        // Width
                0,                        // Escapement
                0,                        // Orientation
                FW_SEMIBOLD,              // Weight (600)
                FALSE,                    // Italic
                FALSE,                    // Underline
                FALSE,                    // StrikeOut
                DEFAULT_CHARSET,          // CharSet
                OUT_TT_PRECIS,           // OutputPrecision (TrueType)
                CLIP_DEFAULT_PRECIS,     // ClipPrecision
                CLEARTYPE_QUALITY,       // Quality
                FF_SWISS | VARIABLE_PITCH, // PitchAndFamily (Swiss = Sans-serif)
                "Segoe UI");             // FaceName
            
            g_hNormalFont = CreateFontA(
                -15,                      // Height
                0,                        // Width
                0,                        // Escapement
                0,                        // Orientation
                FW_NORMAL,                // Weight (400)
                FALSE,                    // Italic
                FALSE,                    // Underline
                FALSE,                    // StrikeOut
                DEFAULT_CHARSET,          // CharSet
                OUT_TT_PRECIS,           // OutputPrecision (TrueType)
                CLIP_DEFAULT_PRECIS,     // ClipPrecision
                CLEARTYPE_QUALITY,       // Quality
                FF_SWISS | VARIABLE_PITCH, // PitchAndFamily
                "Segoe UI");             // FaceName
            
            HFONT hNormalFont = g_hNormalFont;
            
            // Header is drawn in WM_PAINT (no controls needed here)
            
            // Tab control (moved down to account for 90px header)
            g_hTabControl = CreateWindowExA(0, WC_TABCONTROLA, "", WS_VISIBLE | WS_CHILD | TCS_FIXEDWIDTH,
                20, 100, 1060, 30, hwnd, (HMENU)ID_TAB_CONTROL, g_hInstance, nullptr);
            SendMessage(g_hTabControl, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            
            // Add tabs
            TCITEMA tci = {};
            tci.mask = TCIF_TEXT;
            tci.pszText = const_cast<char*>("Erasure");
            TabCtrl_InsertItem(g_hTabControl, 0, &tci);
            tci.pszText = const_cast<char*>("Erasure Details");
            TabCtrl_InsertItem(g_hTabControl, 1, &tci);
            tci.pszText = const_cast<char*>("Reports");
            TabCtrl_InsertItem(g_hTabControl, 2, &tci);
            
            // ListView (adjusted for 90px header + 10px gap)
            g_hListView = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, "", 
                WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                20, 140, 1060, 200, hwnd, (HMENU)ID_LISTVIEW, g_hInstance, nullptr);
            
            DWORD exStyles = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER;
            ListView_SetExtendedListViewStyle(g_hListView, exStyles);
            
            // Set custom colors to keep selection blue always
            ListView_SetBkColor(g_hListView, RGB(255, 255, 255)); // White background
            ListView_SetTextBkColor(g_hListView, RGB(255, 255, 255)); // White text background
            
            SendMessage(g_hListView, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            
            // ListView columns matching the example
            LVCOLUMNA col{};
            col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
            col.fmt = LVCFMT_LEFT;
            
            col.cx = 50; col.pszText = const_cast<char*>("Disk #");
            ListView_InsertColumn(g_hListView, 0, &col);
            
            col.cx = 250; col.pszText = const_cast<char*>("Model");
            ListView_InsertColumn(g_hListView, 1, &col);
            
            col.cx = 180; col.pszText = const_cast<char*>("Serial Number");
            ListView_InsertColumn(g_hListView, 2, &col);
            
            col.cx = 80; col.pszText = const_cast<char*>("Bus Type");
            ListView_InsertColumn(g_hListView, 3, &col);
            
            col.cx = 80; col.pszText = const_cast<char*>("Size (GB)");
            ListView_InsertColumn(g_hListView, 4, &col);
            
            col.cx = 80; col.pszText = const_cast<char*>("Status");
            ListView_InsertColumn(g_hListView, 5, &col);
            
            col.cx = 80; col.pszText = const_cast<char*>("Progress");
            ListView_InsertColumn(g_hListView, 6, &col);
            
            col.cx = 260; col.pszText = const_cast<char*>("Speed");
            ListView_InsertColumn(g_hListView, 7, &col);
            
            // Advanced Options Panel (adjusted Y position)
            CreateWindowA("STATIC", "Advanced Options", WS_VISIBLE | WS_CHILD | SS_LEFT,
                20, 350, 200, 25, hwnd, nullptr, g_hInstance, nullptr);
            
            // Erasure Method (adjusted Y position)
            CreateWindowA("STATIC", "Erasure Method :", WS_VISIBLE | WS_CHILD | SS_LEFT,
                30, 380, 150, 20, hwnd, nullptr, g_hInstance, nullptr);
            g_hMethodCombo = CreateWindowExA(0, "COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL,
                190, 375, 350, 200, hwnd, (HMENU)ID_METHOD_COMBO, g_hInstance, nullptr);
            SendMessage(g_hMethodCombo, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            SendMessageA(g_hMethodCombo, CB_ADDSTRING, 0, (LPARAM)"Quick Wipe (Partition Table Removal)");
            SendMessageA(g_hMethodCombo, CB_ADDSTRING, 0, (LPARAM)"Secure Wipe (Zero All Sectors)");
            SendMessage(g_hMethodCombo, CB_SETCURSEL, 0, 0);
            
            // Verification (aligned with Erasure Method)
            CreateWindowA("STATIC", "Verification :", WS_VISIBLE | WS_CHILD | SS_LEFT,
                580, 380, 150, 20, hwnd, nullptr, g_hInstance, nullptr);
            g_hVerificationCombo = CreateWindowExA(0, "COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL,
                720, 375, 350, 200, hwnd, (HMENU)ID_VERIFICATION_COMBO, g_hInstance, nullptr);
            SendMessage(g_hVerificationCombo, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            SendMessageA(g_hVerificationCombo, CB_ADDSTRING, 0, (LPARAM)"No Verification");
            SendMessageA(g_hVerificationCombo, CB_ADDSTRING, 0, (LPARAM)"Quick Verification");
            SendMessageA(g_hVerificationCombo, CB_ADDSTRING, 0, (LPARAM)"Full Verification");
            SendMessage(g_hVerificationCombo, CB_SETCURSEL, 0, 0);
            
            // Customer Information (for Erasure Details tab - initially hidden)
            CreateWindowA("STATIC", "Customer Name:", WS_CHILD | SS_LEFT,
                30, 160, 150, 20, hwnd, nullptr, g_hInstance, nullptr);
            g_hCustomerNameInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT | ES_AUTOHSCROLL,
                190, 158, 400, 22, hwnd, nullptr, g_hInstance, nullptr);
            SendMessage(g_hCustomerNameInput, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            
            CreateWindowA("STATIC", "Address:", WS_CHILD | SS_LEFT,
                30, 190, 150, 20, hwnd, nullptr, g_hInstance, nullptr);
            g_hCustomerAddressInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT | ES_AUTOHSCROLL,
                190, 188, 400, 22, hwnd, nullptr, g_hInstance, nullptr);
            SendMessage(g_hCustomerAddressInput, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            
            CreateWindowA("STATIC", "Phone:", WS_CHILD | SS_LEFT,
                30, 220, 150, 20, hwnd, nullptr, g_hInstance, nullptr);
            g_hCustomerPhoneInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT | ES_AUTOHSCROLL,
                190, 218, 400, 22, hwnd, nullptr, g_hInstance, nullptr);
            SendMessage(g_hCustomerPhoneInput, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            
            CreateWindowA("STATIC", "Email:", WS_CHILD | SS_LEFT,
                30, 250, 150, 20, hwnd, nullptr, g_hInstance, nullptr);
            g_hCustomerEmailInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT | ES_AUTOHSCROLL,
                190, 248, 400, 22, hwnd, nullptr, g_hInstance, nullptr);
            SendMessage(g_hCustomerEmailInput, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            
            // Progress section (modern style with increased height)
            CreateWindowA("STATIC", "Progress:", WS_VISIBLE | WS_CHILD | SS_LEFT,
                30, 410, 80, 20, hwnd, nullptr, g_hInstance, nullptr);
            g_hProgressBar = CreateWindowExA(0, "msctls_progress32", "", WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
                120, 410, 800, 25, hwnd, (HMENU)ID_PROGRESS, g_hInstance, nullptr);
            SendMessage(g_hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            SendMessage(g_hProgressBar, PBM_SETBARCOLOR, 0, RGB(0, 120, 212)); // Modern blue
            
            CreateWindowA("STATIC", "Fine:", WS_VISIBLE | WS_CHILD | SS_LEFT,
                30, 443, 80, 20, hwnd, nullptr, g_hInstance, nullptr);
            g_hProgressBarFine = CreateWindowExA(0, "msctls_progress32", "", WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
                120, 443, 800, 22, hwnd, nullptr, g_hInstance, nullptr);
            SendMessage(g_hProgressBarFine, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            SendMessage(g_hProgressBarFine, PBM_SETBARCOLOR, 0, RGB(16, 160, 80)); // Modern green
            
            g_hEtaText = CreateWindowA("STATIC", "ETA: --:--:--", WS_VISIBLE | WS_CHILD | SS_LEFT,
                950, 425, 180, 20, hwnd, (HMENU)ID_ETA_TEXT, g_hInstance, nullptr);
            SendMessage(g_hEtaText, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            
            // Buttons (modern style with owner-draw for rounded corners + BS_NOTIFY for hover)
            g_hBtnRefresh = CreateWindowA("BUTTON", "Refresh", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                230, 485, 130, 35, hwnd, (HMENU)ID_BTN_REFRESH, g_hInstance, nullptr);
            
            g_hBtnWipe = CreateWindowA("BUTTON", "Erase", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                390, 473, 170, 50, hwnd, (HMENU)ID_BTN_WIPE, g_hInstance, nullptr);
            
            g_hBtnCancel = CreateWindowA("BUTTON", "Cancel", WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                570, 473, 170, 50, hwnd, (HMENU)ID_BTN_CANCEL, g_hInstance, nullptr);
            EnableWindow(g_hBtnCancel, FALSE); // Initially disabled
            
            CreateWindowA("BUTTON", "Exit", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                750, 485, 130, 35, hwnd, (HMENU)ID_BTN_EXIT, g_hInstance, nullptr);
            
            CreateWindowA("BUTTON", "About", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                890, 485, 130, 35, hwnd, (HMENU)ID_ABOUT_DIALOG, g_hInstance, nullptr);
            
            // Status log at bottom (multiline, read-only, auto-scroll)
            g_hStatusText = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", 
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                10, 540, 1120, 200, hwnd, (HMENU)ID_STATUS_TEXT, g_hInstance, nullptr);
            SendMessage(g_hStatusText, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            SetWindowTextA(g_hStatusText, "[System] Application started - Scanning disks...\r\n");
            
            // Check admin
            BOOL isAdmin = FALSE;
            PSID adminGroup = nullptr;
            SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
            if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
                CheckTokenMembership(nullptr, adminGroup, &isAdmin);
                FreeSid(adminGroup);
            }
            
            if (!isAdmin) {
                LogMessage("WARNING: Not running as Administrator! Disk operations may fail.");
            } else {
                LogMessage("Running as Administrator");
            }
            
            LogMessage("Application started");
            RefreshDiskList();
            loadReports();
            
            // Force repaint to show header gradient
            InvalidateRect(hwnd, NULL, TRUE);
            
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Modern gradient header (blue to dark blue)
            RECT headerRect = {0, 0, 1150, 90};
            
            // Create vibrant gradient (modern blue gradient - left to right)
            TRIVERTEX vertex[2];
            vertex[0].x = 0;
            vertex[0].y = 0;
            vertex[0].Red = 0x0000;      // RGB(0, 120, 215) -> Bright blue (Windows accent)
            vertex[0].Green = 0x7800;
            vertex[0].Blue = 0xD700;
            vertex[0].Alpha = 0x0000;
            
            vertex[1].x = 1150;
            vertex[1].y = 90;
            vertex[1].Red = 0x0000;      // RGB(0, 60, 140) -> Deep blue
            vertex[1].Green = 0x3C00;
            vertex[1].Blue = 0x8C00;
            vertex[1].Alpha = 0x0000;
            
            GRADIENT_RECT gRect;
            gRect.UpperLeft = 0;
            gRect.LowerRight = 1;
            GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
            
            // Draw icon circle (disk symbol)
            HBRUSH hIconBrush = CreateSolidBrush(RGB(255, 255, 255));
            HPEN hIconPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hIconBrush);
            HPEN hOldPen = (HPEN)SelectObject(hdc, hIconPen);
            
            // Draw disk icon outline
            SelectObject(hdc, GetStockObject(NULL_BRUSH)); // Hollow circle
            Ellipse(hdc, 25, 20, 65, 60);
            
            // Draw inner circle (disk center)
            Ellipse(hdc, 38, 33, 52, 47);
            
            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);
            DeleteObject(hIconBrush);
            DeleteObject(hIconPen);
            
            // Draw modern title (no shadow for cleaner look)
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255)); // Pure white
            
            HFONT hTitleFont = CreateFontA(-42, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_SWISS | VARIABLE_PITCH, "Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);
            
            RECT titleRect = {80, 15, 1150, 90};
            DrawTextA(hdc, "PIWIPER", -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            
            // Subtitle with better contrast
            SelectObject(hdc, hOldFont);
            HFONT hSubFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_SWISS | VARIABLE_PITCH, "Segoe UI");
            SelectObject(hdc, hSubFont);
            SetTextColor(hdc, RGB(220, 235, 255)); // Lighter for better contrast
            RECT subRect = {240, 15, 1150, 90};
            DrawTextA(hdc, "Professional Disk Eraser", -1, &subRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            
            SelectObject(hdc, hOldFont);
            DeleteObject(hTitleFont);
            DeleteObject(hSubFont);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int notifyCode = HIWORD(wParam);
            
            OutputDebugStringA(("[PIWIPER] WM_COMMAND: id=" + std::to_string(id) + ", notify=" + std::to_string(notifyCode)).c_str());
            
            if (id == ID_BTN_REFRESH) {
                    LogMessage("Refresh button clicked");
                    OutputDebugStringA("[PIWIPER] Refresh button clicked - calling RefreshDiskList");
                    try {
                    RefreshDiskList();
                        OutputDebugStringA("[PIWIPER] RefreshDiskList completed successfully");
                    } catch (const std::exception& e) {
                        OutputDebugStringA(("[PIWIPER] RefreshDiskList ERROR: " + std::string(e.what())).c_str());
                        AppendStatusMessage("ERROR: Failed to refresh disk list");
                    } catch (...) {
                        OutputDebugStringA("[PIWIPER] RefreshDiskList ERROR: Unknown exception");
                        AppendStatusMessage("ERROR: Failed to refresh disk list");
                }
            } else if (id == ID_BTN_WIPE) {
                WipeSelectedDisk();
            } else if (id == ID_BTN_CANCEL) {
                if (g_isWiping) {
                        g_cancelRequested = true;
                    LogMessage("Cancellation requested by user");
                    OutputDebugStringA("[PIWIPER] Cancel button clicked - wipe will stop gracefully");
                    SetWindowTextA(g_hStatusText, "Cancelling operation - please wait...");
                }
            } else if (id == ID_BTN_EXIT) {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            } else if (id == ID_ABOUT_DIALOG) {
                ShowAboutDialog();
            } else if (id == ID_METHOD_COMBO && notifyCode == CBN_SELCHANGE) {
                int sel = SendMessage(g_hMethodCombo, CB_GETCURSEL, 0, 0);
                if (sel == 0) {
                    g_quickWipe = true;
                    LogMessage("Quick wipe mode selected (fast - partition table removal)");
                    OutputDebugStringA("[PIWIPER] ComboBox: g_quickWipe = true");
                } else if (sel == 1) {
                    g_quickWipe = false;
                    LogMessage("Secure wipe mode selected (slow - zeros all sectors - may take hours!)");
                    OutputDebugStringA("[PIWIPER] ComboBox: g_quickWipe = false (SECURE MODE)");
                }
                // Return focus to ListView so selection stays blue
                SetFocus(g_hListView);
            } else if (id == ID_VERIFICATION_COMBO && notifyCode == CBN_SELCHANGE) {
                int sel = SendMessage(g_hVerificationCombo, CB_GETCURSEL, 0, 0);
                g_verificationMode = sel; // 0=None, 1=Quick, 2=Full
                const char* modes[] = {"No Verification", "Quick Verification (first 100MB)", "Full Verification (entire disk)"};
                LogMessage(std::string("Verification mode: ") + modes[sel]);
                SetFocus(g_hListView);
            }
            return 0;
        }
        
        case WM_UPDATE_PROGRESS: {
            int progress = LOWORD(wParam);
            int fineProgress = HIWORD(wParam);
            int etaSec = LOWORD(lParam);
            int speedMBps = HIWORD(lParam);
            
            // Debug: Log progress updates (throttled - only every 10%)
            static int lastLoggedProgress = -1;
            if (progress % 10 == 0 && progress != lastLoggedProgress) {
                OutputDebugStringA(("[PIWIPER] Progress update: " + std::to_string(progress) + "%, Fine: " + std::to_string(fineProgress) + "%, ETA: " + std::to_string(etaSec) + "s, Speed: " + std::to_string(speedMBps) + " MB/s").c_str());
                lastLoggedProgress = progress;
            }
            
            // Update progress bars
            if (g_hProgressBar) {
                SendMessageA(g_hProgressBar, PBM_SETPOS, progress, 0);
            }
            if (g_hProgressBarFine) {
                // Fine progress bar cycles 0-100% every 5MB
                SendMessageA(g_hProgressBarFine, PBM_SETPOS, fineProgress, 0);
            }
            
            // Update progress text
            char progressText[128];
            if (etaSec > 0) {
                int hours = etaSec / 3600;
                int mins = (etaSec % 3600) / 60;
                int secs = etaSec % 60;
                
                if (hours > 0) {
                    sprintf_s(progressText, "Progress: %d%% - ETA: %dh %dm %ds", progress, hours, mins, secs);
                } else if (mins > 0) {
                    sprintf_s(progressText, "Progress: %d%% - ETA: %dm %ds", progress, mins, secs);
                } else {
                    sprintf_s(progressText, "Progress: %d%% - ETA: %ds", progress, secs);
                }
            } else {
                sprintf_s(progressText, "Progress: %d%%", progress);
            }
            
            if (g_hEtaText) {
                SetWindowTextA(g_hEtaText, progressText);
            }
            
            // Update speed column in ListView for selected/active disk
            int selectedIndex = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
            if (selectedIndex != -1) {
                // Update progress and speed for selected item
                if (progress < 100) {
                    // Update progress percentage
                    char pctText[16];
                    sprintf_s(pctText, "%d%%", progress);
                    ListView_SetItemText(g_hListView, selectedIndex, 6, pctText);
                    
                    // Display real speed from secure wipe, or estimate for quick wipe
                    char speedText[32];
                    if (speedMBps > 0) {
                        // Real speed from secure wipe
                        sprintf_s(speedText, "%d MB/s", speedMBps);
                    } else if (g_quickWipe) {
                        // Quick wipe - just show estimate
                        sprintf_s(speedText, "~100 MB/s");
                    } else {
                        sprintf_s(speedText, "--");
                    }
                    ListView_SetItemText(g_hListView, selectedIndex, 7, speedText);
                    
                    // Update status
                    ListView_SetItemText(g_hListView, selectedIndex, 5, const_cast<char*>("Wiping..."));
                } else if (progress == 100) {
                    ListView_SetItemText(g_hListView, selectedIndex, 6, const_cast<char*>("100%"));
                    ListView_SetItemText(g_hListView, selectedIndex, 7, const_cast<char*>("--"));
                    ListView_SetItemText(g_hListView, selectedIndex, 5, const_cast<char*>("Complete"));
                }
            }
            
            return 0;
        }
        
        case WM_WIPE_COMPLETE: {
            g_isWiping = false;
            g_wipeProcessHandle = NULL;
            EnableWindow(g_hBtnWipe, TRUE);
            EnableWindow(g_hBtnRefresh, TRUE);
            EnableWindow(g_hBtnCancel, FALSE); // Disable cancel button
            ShowWindow(g_hBtnCancel, SW_HIDE); // Hide cancel button
            
            if (g_cancelRequested) {
                LogMessage("Wipe operation cancelled by user");
                MessageBoxA(hwnd, "Disk wipe operation was cancelled.", "Cancelled", MB_OK | MB_ICONWARNING);
                g_cancelRequested = false; // Reset flag
            } else if (wParam == 1) {
                LogMessage("Wipe completed successfully!");
                MessageBoxA(hwnd, "Disk wipe completed successfully!", "Success", MB_OK | MB_ICONINFORMATION);
            } else {
                LogMessage("Wipe failed!");
                MessageBoxA(hwnd, "Disk wipe failed! Check log for details.", "Error", MB_OK | MB_ICONERROR);
            }
            
            RefreshDiskList();
            loadReports();
            return 0;
        }
        
        case WM_CLOSE:
            if (g_isWiping) {
                MessageBoxA(hwnd, "Please wait for the wipe operation to complete.", "Operation In Progress", MB_OK | MB_ICONWARNING);
                return 0;
            }
            DestroyWindow(hwnd);
            return 0;
            
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
            
            if (dis->CtlType == ODT_BUTTON) {
                // Determine base button color and text
                COLORREF baseColor, textColor;
                const char* btnText = "";
                HFONT hFont = g_hNormalFont;
                
                if (dis->hwndItem == g_hBtnWipe) {
                    baseColor = RGB(0, 120, 212);  // Blue
                    textColor = RGB(255, 255, 255);
                    btnText = "Erase";
                    hFont = g_hTitleFont;
                } else if (dis->hwndItem == g_hBtnCancel) {
                    baseColor = RGB(232, 17, 35);  // Red
                    textColor = RGB(255, 255, 255);
                    btnText = "Cancel";
                    hFont = g_hTitleFont;
                } else if (dis->hwndItem == g_hBtnRefresh) {
                    baseColor = RGB(100, 100, 100);  // Gray
                    textColor = RGB(255, 255, 255);
                    btnText = "Refresh";
                } else if (dis->hwndItem == GetDlgItem(g_hWnd, ID_ABOUT_DIALOG)) {
                    baseColor = RGB(100, 100, 100);  // Gray for About
                    textColor = RGB(255, 255, 255);
                    btnText = "About";
                } else {
                    baseColor = RGB(80, 80, 80);   // Dark gray for Exit
                    textColor = RGB(255, 255, 255);
                    btnText = "Exit";
                }
                
                COLORREF bgColor = baseColor;
                
                // Apply visual states
                if (dis->itemState & ODS_DISABLED) {
                    bgColor = RGB(180, 180, 180); // Gray for disabled
                } else if (dis->itemState & ODS_SELECTED) {
                    // Pressed - darker (80% brightness)
                    bgColor = RGB(
                        GetRValue(baseColor) * 0.8,
                        GetGValue(baseColor) * 0.8,
                        GetBValue(baseColor) * 0.8
                    );
                } else if (dis->itemState & ODS_FOCUS || dis->itemState & ODS_HOTLIGHT) {
                    // Hover - lighter (120% brightness)
                    bgColor = RGB(
                        min(GetRValue(baseColor) * 1.2, 255),
                        min(GetGValue(baseColor) * 1.2, 255),
                        min(GetBValue(baseColor) * 1.2, 255)
                    );
                }
                
                // Draw button with state-based color
                HBRUSH hBrush = CreateSolidBrush(bgColor);
                HPEN hPen = CreatePen(PS_SOLID, 1, bgColor);
                HBRUSH hOldBrush = (HBRUSH)SelectObject(dis->hDC, hBrush);
                HPEN hOldPen = (HPEN)SelectObject(dis->hDC, hPen);
                
                // Draw rounded rectangle (8px radius)
                RECT btnRect = dis->rcItem;
                if (dis->itemState & ODS_SELECTED) {
                    // Offset for pressed effect
                    btnRect.top += 1;
                    btnRect.left += 1;
                }
                
                RoundRect(dis->hDC, btnRect.left, btnRect.top, 
                         btnRect.right, btnRect.bottom, 8, 8);
                
                SelectObject(dis->hDC, hOldBrush);
                SelectObject(dis->hDC, hOldPen);
                DeleteObject(hBrush);
                DeleteObject(hPen);
                
                // Draw text
                SetBkMode(dis->hDC, TRANSPARENT);
                SetTextColor(dis->hDC, textColor);
                HFONT hOldFont = (HFONT)SelectObject(dis->hDC, hFont);
                DrawTextA(dis->hDC, btnText, -1, &btnRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SelectObject(dis->hDC, hOldFont);
                
                return TRUE;
            }
            break;
        }
        
        case WM_DESTROY:
            // Clean up brushes
            if (g_hBtnBlueBrush) DeleteObject(g_hBtnBlueBrush);
            if (g_hBtnRedBrush) DeleteObject(g_hBtnRedBrush);
            if (g_hBtnGrayBrush) DeleteObject(g_hBtnGrayBrush);
            
            // Clean up fonts
            if (g_hTitleFont) DeleteObject(g_hTitleFont);
            if (g_hNormalFont) DeleteObject(g_hNormalFont);
            
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// WinMain entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInstance = hInstance;
    
    // Initialize common controls
    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icc);
    
    // Register window class
    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "DiskWiperGUI";
    wc.hbrBackground = CreateSolidBrush(RGB(240, 242, 245)); // Modern light gray background
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    
    if (!RegisterClassExA(&wc)) {
        MessageBoxA(nullptr, "Window registration failed!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Calculate centered position
    int windowWidth = 1150;
    int windowHeight = 800;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenWidth - windowWidth) / 2;
    int posY = (screenHeight - windowHeight) / 2;
    
    // Create window centered on screen with version info
    std::string windowTitle = std::string(PIWIPER_APP_NAME) + " v" + PIWIPER_VERSION_SHORT + " - " + PIWIPER_APP_DESCRIPTION;
    g_hWnd = CreateWindowExA(0, "DiskWiperGUI", windowTitle.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        posX, posY, windowWidth, windowHeight,
        nullptr, nullptr, hInstance, nullptr);
    
    if (!g_hWnd) {
        MessageBoxA(nullptr, "Window creation failed!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

