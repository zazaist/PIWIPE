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
#include <richedit.h>
#include "version.h"
#include "piwiper_resource.h"

// RichEdit constants
#ifndef EM_SETTEXTMODE
#define EM_SETTEXTMODE 0x0444
#endif
#ifndef TM_PLAINTEXT
#define TM_PLAINTEXT 0x0001
#endif

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
    std::string method;
    std::string notes;
    std::string filename;
};

// Global variables
HINSTANCE g_hInstance;
HWND g_hWnd;
bool g_aboutDialogOpen = false;
int g_activeTab = 1028; // Track active tab (ID_TAB_ERASURE)

// Form data structure
struct FormData {
    std::string companyName;
    std::string technicalPerson;
    std::string position;
    std::string phone;
    std::string email;
    std::string address;
    std::string issuingCompanyName;
    std::string issuingTechnicianName;
    std::string issuingLocation;
    std::string issuingPhone;
    std::string issuingEmail;
};

FormData g_formData;
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
HWND g_hBtnExit;
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
bool g_quickWipe = true; // Default to quick wipe for convenience
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
#define ID_TECHNICAL_PERSON_INPUT 1022
#define ID_POSITION_INPUT 1023
#define ID_PHONE_INPUT 1024
#define ID_EMAIL_INPUT 1042
#define ID_ADDRESS_LABEL 1043
#define ID_ADDRESS_INPUT 1044
#define ID_ISSUING_CERT_LABEL 1045
#define ID_COMPANY_DETAILS_LABEL 1046
#define ID_ISSUING_COMPANY_LABEL 1047
#define ID_ISSUING_COMPANY_INPUT 1048
#define ID_ISSUING_TECHNICIAN_LABEL 1049
#define ID_ISSUING_TECHNICIAN_INPUT 1050
#define ID_ISSUING_LOCATION_LABEL 1051
#define ID_ISSUING_LOCATION_INPUT 1052
#define ID_ISSUING_PHONE_LABEL 1053
#define ID_ISSUING_PHONE_INPUT 1054
#define ID_ISSUING_EMAIL_LABEL 1055
#define ID_ISSUING_EMAIL_INPUT 1056
#define ID_ABOUT_DIALOG 1027
#define ID_TAB_ERASURE 1028
#define ID_TAB_DETAILS 1029
#define ID_TAB_REPORT 1030
#define ID_ADVANCED_OPTIONS_LABEL 1031
#define ID_ERASURE_METHOD_LABEL 1032
#define ID_VERIFICATION_LABEL 1033
#define ID_PROGRESS_LABEL 1034
#define ID_FINE_LABEL 1035
#define ID_FORM_TITLE_LABEL 1036
#define ID_COMPANY_NAME_LABEL 1037
#define ID_TECHNICAL_PERSON_LABEL 1038
#define ID_POSITION_LABEL 1039
#define ID_PHONE_LABEL 1040
#define ID_EMAIL_LABEL 1041
#define ID_SAVE_FORM_BUTTON 1057
#define ID_REPORT_TITLE_LABEL 1058
#define ID_DISK_MODEL_LABEL 1059
#define ID_DISK_SERIAL_LABEL 1060
#define ID_DISK_SIZE_LABEL 1061
#define ID_ERASURE_METHOD_LABEL2 1062
#define ID_ERASURE_DATE_LABEL 1063
#define ID_ERASURE_TIME_LABEL 1064
#define ID_ERASURE_DURATION_LABEL 1065
#define ID_VERIFICATION_STATUS_LABEL 1066
#define ID_COMPANY_NAME_LABEL2 1067
#define ID_TECHNICAL_PERSON_LABEL2 1068
#define ID_POSITION_LABEL2 1069
#define ID_PHONE_LABEL2 1070
#define ID_EMAIL_LABEL2 1071
#define ID_PREVIEW_BUTTON 1072
#define ID_EXPORT_PDF_BUTTON 1073
#define ID_PREVIEW_DIALOG 1074
#define ID_REPORT_PREVIEW_AREA 1075

// Report data structure
struct ReportData {
    std::string diskModel;
    std::string diskSerial;
    std::string diskSize;
    std::string busType;
    std::string erasureMethod;
    std::string erasureDate;
    std::string erasureTime;
    std::string erasureDuration;
    std::string verificationStatus;
    std::string companyName;
    std::string technicalPerson;
    std::string position;
    std::string phone;
    std::string email;
    std::string address;
};

ReportData g_reportData;

// Report data save/load functions
// Forward declarations
void UpdateReportFields();
void UpdateReportPreview();

void SaveReportData() {
    std::ofstream file("report_data.txt");
    if (file.is_open()) {
        file << g_reportData.diskModel << std::endl;
        file << g_reportData.diskSerial << std::endl;
        file << g_reportData.diskSize << std::endl;
        file << g_reportData.busType << std::endl;
        file << g_reportData.erasureMethod << std::endl;
        file << g_reportData.erasureDate << std::endl;
        file << g_reportData.erasureTime << std::endl;
        file << g_reportData.erasureDuration << std::endl;
        file << g_reportData.verificationStatus << std::endl;
        file << g_reportData.companyName << std::endl;
        file << g_reportData.technicalPerson << std::endl;
        file << g_reportData.position << std::endl;
        file << g_reportData.phone << std::endl;
        file << g_reportData.email << std::endl;
        file.close();
    } else {
    }
}

void LoadReportData() {
    std::ifstream file("report_data.txt");
    if (file.is_open()) {
        std::getline(file, g_reportData.diskModel);
        std::getline(file, g_reportData.diskSerial);
        std::getline(file, g_reportData.diskSize);
        std::getline(file, g_reportData.busType);
        std::getline(file, g_reportData.erasureMethod);
        std::getline(file, g_reportData.erasureDate);
        std::getline(file, g_reportData.erasureTime);
        std::getline(file, g_reportData.erasureDuration);
        std::getline(file, g_reportData.verificationStatus);
        std::getline(file, g_reportData.companyName);
        std::getline(file, g_reportData.technicalPerson);
        std::getline(file, g_reportData.position);
        std::getline(file, g_reportData.phone);
        std::getline(file, g_reportData.email);
        file.close();
    } else {
    }
}

// Form data save/load functions
void SaveFormData(bool showMessage = false) {
    std::ofstream file("form_data.txt");
    if (file.is_open()) {
        file << g_formData.companyName << std::endl;
        file << g_formData.technicalPerson << std::endl;
        file << g_formData.position << std::endl;
        file << g_formData.phone << std::endl;
        file << g_formData.email << std::endl;
        file << g_formData.address << std::endl;
        file << g_formData.issuingCompanyName << std::endl;
        file << g_formData.issuingTechnicianName << std::endl;
        file << g_formData.issuingLocation << std::endl;
        file << g_formData.issuingPhone << std::endl;
        file << g_formData.issuingEmail << std::endl;
        file.close();
        // Update report fields and preview with new form data
        UpdateReportFields();
        UpdateReportPreview();
        
        if (showMessage) {
            MessageBoxA(g_hWnd, "Form data saved successfully!", "Save Complete", MB_OK | MB_ICONINFORMATION);
        }
    } else {
        if (showMessage) {
            MessageBoxA(g_hWnd, "ERROR: Could not save form data!", "Save Error", MB_OK | MB_ICONERROR);
        }
    }
}

void LoadFormData() {
    std::ifstream file("form_data.txt");
    if (file.is_open()) {
        std::getline(file, g_formData.companyName);
        std::getline(file, g_formData.technicalPerson);
        std::getline(file, g_formData.position);
        std::getline(file, g_formData.phone);
        std::getline(file, g_formData.email);
        std::getline(file, g_formData.address);
        // Try to load Issuing Certificate fields (for backward compatibility)
        std::string line;
        if (std::getline(file, line)) {
            g_formData.issuingCompanyName = line;
            if (std::getline(file, line)) {
                g_formData.issuingTechnicianName = line;
                if (std::getline(file, line)) {
                    g_formData.issuingLocation = line;
                    if (std::getline(file, line)) {
                        g_formData.issuingPhone = line;
                        if (std::getline(file, line)) {
                            g_formData.issuingEmail = line;
                        } else {
                            g_formData.issuingEmail = "";
                        }
                    } else {
                        g_formData.issuingPhone = "";
                        g_formData.issuingEmail = "";
                    }
                } else {
                    g_formData.issuingLocation = "";
                    g_formData.issuingPhone = "";
                    g_formData.issuingEmail = "";
                }
            } else {
                // Old format: only company, location, phone
                g_formData.issuingTechnicianName = "";
                g_formData.issuingLocation = line;
                if (std::getline(file, line)) {
                    g_formData.issuingPhone = line;
                    g_formData.issuingEmail = "";
                } else {
                    g_formData.issuingLocation = "";
                    g_formData.issuingPhone = "";
                    g_formData.issuingEmail = "";
                }
            }
        } else {
            // Default values if not found (backward compatibility)
            g_formData.issuingCompanyName = "";
            g_formData.issuingTechnicianName = "";
            g_formData.issuingLocation = "";
            g_formData.issuingPhone = "";
            g_formData.issuingEmail = "";
        }
        file.close();
        
        // Check if any field is empty, if so fill with example data
        bool isEmpty = g_formData.companyName.empty() || 
                      g_formData.technicalPerson.empty() || 
                      g_formData.position.empty() || 
                      g_formData.phone.empty() || 
                      g_formData.email.empty() || 
                      g_formData.address.empty();
        
        if (isEmpty) {
            g_formData.companyName = "Example Company Ltd.";
            g_formData.technicalPerson = "John Smith";
            g_formData.position = "Senior Technician";
            g_formData.phone = "+1-555-0123";
            g_formData.email = "john.smith@example.com";
            g_formData.address = "123 Business Street, Suite 100, New York, NY 10001";
            g_formData.issuingCompanyName = "";
            g_formData.issuingTechnicianName = "";
            g_formData.issuingLocation = "";
            g_formData.issuingPhone = "";
            g_formData.issuingEmail = "";
            // Save the example data
            SaveFormData(false);
        } else {
        }
    } else {
        g_formData.companyName = "Example Company Ltd.";
        g_formData.technicalPerson = "John Smith";
        g_formData.position = "Senior Technician";
        g_formData.phone = "+1-555-0123";
        g_formData.email = "john.smith@example.com";
        g_formData.address = "";
        g_formData.issuingCompanyName = "";
        g_formData.issuingTechnicianName = "";
        g_formData.issuingLocation = "";
        g_formData.issuingPhone = "";
        g_formData.issuingEmail = "";
        // Save the example data
        SaveFormData(false);
    }
}

void UpdateFormFields() {
    HWND hCompany = GetDlgItem(g_hWnd, ID_CUSTOMER_NAME_INPUT);
    if (hCompany) {
        SetWindowTextA(hCompany, g_formData.companyName.c_str());
    }
    
    HWND hTechnical = GetDlgItem(g_hWnd, ID_TECHNICAL_PERSON_INPUT);
    if (hTechnical) {
        SetWindowTextA(hTechnical, g_formData.technicalPerson.c_str());
    }
    
    HWND hPosition = GetDlgItem(g_hWnd, ID_POSITION_INPUT);
    if (hPosition) {
        SetWindowTextA(hPosition, g_formData.position.c_str());
    }
    
    HWND hPhone = GetDlgItem(g_hWnd, ID_PHONE_INPUT);
    if (hPhone) {
        SetWindowTextA(hPhone, g_formData.phone.c_str());
    }
    
    HWND hEmail = GetDlgItem(g_hWnd, ID_EMAIL_INPUT);
    if (hEmail) {
        SetWindowTextA(hEmail, g_formData.email.c_str());
    }
    
    HWND hAddress = GetDlgItem(g_hWnd, ID_ADDRESS_INPUT);
    if (hAddress) {
        SetWindowTextA(hAddress, g_formData.address.c_str());
    }
    
    // Update Issuing Certificate fields
    HWND hIssuingCompany = GetDlgItem(g_hWnd, ID_ISSUING_COMPANY_INPUT);
    if (hIssuingCompany) {
        SetWindowTextA(hIssuingCompany, g_formData.issuingCompanyName.c_str());
    }
    
    HWND hIssuingTechnician = GetDlgItem(g_hWnd, ID_ISSUING_TECHNICIAN_INPUT);
    if (hIssuingTechnician) {
        SetWindowTextA(hIssuingTechnician, g_formData.issuingTechnicianName.c_str());
    }
    
    HWND hIssuingLocation = GetDlgItem(g_hWnd, ID_ISSUING_LOCATION_INPUT);
    if (hIssuingLocation) {
        SetWindowTextA(hIssuingLocation, g_formData.issuingLocation.c_str());
    }
    
    HWND hIssuingPhone = GetDlgItem(g_hWnd, ID_ISSUING_PHONE_INPUT);
    if (hIssuingPhone) {
        SetWindowTextA(hIssuingPhone, g_formData.issuingPhone.c_str());
    }
    
    HWND hIssuingEmail = GetDlgItem(g_hWnd, ID_ISSUING_EMAIL_INPUT);
    if (hIssuingEmail) {
        SetWindowTextA(hIssuingEmail, g_formData.issuingEmail.c_str());
    }
}

void UpdateReportFields() {
    // Update report data with form data
    g_reportData.companyName = g_formData.companyName;
    g_reportData.technicalPerson = g_formData.technicalPerson;
    g_reportData.position = g_formData.position;
    g_reportData.phone = g_formData.phone;
    g_reportData.email = g_formData.email;
    g_reportData.address = g_formData.address;
    
    // Note: Static labels removed - only preview area is used now
    // All report data is displayed in the preview area via UpdateReportPreview()
}

// Preview Report Dialog Procedure
INT_PTR CALLBACK PreviewDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG: {
            // Set report content
            SetDlgItemTextA(hDlg, 1001, g_reportData.erasureDate.c_str());
            SetDlgItemTextA(hDlg, 1002, "1.0.0.0");
            SetDlgItemTextA(hDlg, 1003, "");
            SetDlgItemTextA(hDlg, 1004, g_reportData.companyName.c_str());
            SetDlgItemTextA(hDlg, 1005, "________________");
            SetDlgItemTextA(hDlg, 1006, "Desktop");
            SetDlgItemTextA(hDlg, 1007, "9 Hub");
            SetDlgItemTextA(hDlg, 1008, "5 Device");
            SetDlgItemTextA(hDlg, 1009, g_reportData.diskModel.c_str());
            SetDlgItemTextA(hDlg, 1010, g_reportData.diskSerial.c_str());
            SetDlgItemTextA(hDlg, 1011, g_reportData.diskSize.c_str());
            SetDlgItemTextA(hDlg, 1012, g_reportData.erasureMethod.c_str());
            SetDlgItemTextA(hDlg, 1013, g_reportData.verificationStatus.c_str());
            return TRUE;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    return FALSE;
}

void ShowPreviewDialog() {
    // Show a simple message box with report information for now
    std::string reportText = "PIWIPER Report Preview\n\n";
    reportText += "Report Date: " + g_reportData.erasureDate + "\n";
    reportText += "Software Version: 1.0.0.0\n\n";
    reportText += "Customer Details:\n";
    reportText += "Company Name: " + g_reportData.companyName + "\n";
    reportText += "Technical Person: " + g_reportData.technicalPerson + "\n";
    reportText += "Position: " + g_reportData.position + "\n";
    reportText += "Phone: " + g_reportData.phone + "\n";
    reportText += "Email: " + g_reportData.email + "\n\n";
    reportText += "Hardware Information:\n";
    reportText += "Disk Model: " + g_reportData.diskModel + "\n";
    reportText += "Disk Serial: " + g_reportData.diskSerial + "\n";
    reportText += "Erasure Method: " + g_reportData.erasureMethod + "\n";
    reportText += "Verification: " + g_reportData.verificationStatus + "\n";
    
    MessageBoxA(g_hWnd, reportText.c_str(), "Report Preview", MB_OK | MB_ICONINFORMATION);
}

void UpdateReportPreview() {
    // Update the preview area with report content - Plain text format for EDIT control
    std::string reportText = "";
    
    // Header - Certificate Style
    reportText += "                    PIWIPER ERASE CERTIFICATE\n";
    reportText += "                   Professional Disk Eraser\n\n";
    
    // Get current date and time
    SYSTEMTIME st;
    GetLocalTime(&st);
    char dateStr[32], timeStr[32];
    sprintf_s(dateStr, "%02d/%02d/%04d", st.wDay, st.wMonth, st.wYear);
    sprintf_s(timeStr, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    
    reportText += "Date: " + std::string(dateStr) + "\n";
    reportText += "Time: " + std::string(timeStr) + "\n\n";
    
    // Company Information
    reportText += "COMPANY INFORMATION\n";
    reportText += "Licensed to: " + g_reportData.companyName + "\n";
    reportText += "Company Name: " + g_reportData.companyName + "\n";
    reportText += "Technician Name: " + g_reportData.technicalPerson + "\n";
    reportText += "Position: " + g_reportData.position + "\n";
    reportText += "Phone: " + g_reportData.phone + "\n";
    reportText += "Email: " + g_reportData.email + "\n";
    reportText += "Address: " + g_reportData.address + "\n\n";
    
    // Issuing Certificate - Use form data if available, otherwise use defaults
    reportText += "ISSUING CERTIFICATE\n";
    std::string issuingCompany = g_formData.issuingCompanyName.empty() ? "PIWIPER Professional Disk Eraser" : g_formData.issuingCompanyName;
    std::string issuingTechnician = g_formData.issuingTechnicianName.empty() ? g_reportData.technicalPerson : g_formData.issuingTechnicianName;
    std::string issuingLocation = g_formData.issuingLocation.empty() ? "Professional Data Erasure Services" : g_formData.issuingLocation;
    std::string issuingPhone = g_formData.issuingPhone.empty() ? g_reportData.phone : g_formData.issuingPhone;
    std::string issuingEmail = g_formData.issuingEmail.empty() ? g_reportData.email : g_formData.issuingEmail;
    reportText += "Company Name: " + issuingCompany + "\n";
    reportText += "Technician Name: " + issuingTechnician + "\n";
    reportText += "Company Location: " + issuingLocation + "\n";
    reportText += "Company Phone: " + issuingPhone + "\n";
    reportText += "Company Email: " + issuingEmail + "\n\n";
    
    // Disk Erase Attributes
    reportText += "DISK ERASE\n";
    reportText += "Attributes: Whole Disk Erasure\n";
    reportText += "Erase Method: " + (g_reportData.erasureMethod.empty() ? "" : g_reportData.erasureMethod) + "\n";
    reportText += "Verification: " + (g_reportData.verificationStatus.empty() ? "" : g_reportData.verificationStatus) + "\n";
    reportText += "Process Integrity: Uninterrupted erase\n\n";
    
    // Device Details
    reportText += "DEVICE DETAILS\n";
    reportText += "Name: " + (g_reportData.diskModel.empty() ? "" : g_reportData.diskModel) + "\n";
    reportText += "Manufacturer: " + (g_reportData.diskModel.empty() ? "" : g_reportData.diskModel) + "\n";
    reportText += "Model: " + (g_reportData.diskModel.empty() ? "" : g_reportData.diskModel) + "\n";
    reportText += "Serial Number: " + (g_reportData.diskSerial.empty() ? "" : g_reportData.diskSerial) + "\n";
    reportText += "Capacity: " + (g_reportData.diskSize.empty() ? "N/A" : g_reportData.diskSize) + "\n";
    reportText += "Hard Disk Type: " + (g_reportData.busType.empty() ? "N/A" : g_reportData.busType) + "\n\n";
    
    // Results
    reportText += "RESULTS\n";
    reportText += "Erase Range: ";
    if (!g_reportData.diskModel.empty()) reportText += "Whole Disk";
    reportText += "\n";
    reportText += "Name: " + (g_reportData.diskModel.empty() ? "" : g_reportData.diskModel) + "\n";
    reportText += "Started at: " + (g_reportData.erasureDate.empty() ? "" : g_reportData.erasureDate) + " " + (g_reportData.erasureTime.empty() ? "" : g_reportData.erasureTime) + "\n";
    reportText += "Duration: " + (g_reportData.erasureDuration.empty() ? "" : g_reportData.erasureDuration) + "\n";
    reportText += "Errors: ";
    if (!g_reportData.diskModel.empty()) reportText += "No Errors";
    reportText += "\n";
    reportText += "Result: ";
    if (!g_reportData.diskModel.empty()) reportText += "Erased";
    reportText += "\n";
    reportText += "Status: ";
    if (!g_reportData.diskModel.empty()) reportText += "Success";
    reportText += "\n\n";
    
    // Footer
    reportText += "I hereby state that data erasure has been carried out in\n";
    reportText += "accordance with the instructions given by software provider.\n\n";
    reportText += "Erased by PIWIPER Professional Disk Eraser\n";
    reportText += "PIWIPER Professional Data Erasure Services\n";
    
    // Set the plain text content using SetWindowTextA
    HWND hPreview = GetDlgItem(g_hWnd, ID_REPORT_PREVIEW_AREA);
    if (hPreview) {
        SetWindowTextA(hPreview, reportText.c_str());
    }
}

void GetFormDataFromFields() {
    char buffer[256];
    
    GetWindowTextA(GetDlgItem(g_hWnd, ID_CUSTOMER_NAME_INPUT), buffer, sizeof(buffer));
    g_formData.companyName = buffer;
    
    GetWindowTextA(GetDlgItem(g_hWnd, ID_TECHNICAL_PERSON_INPUT), buffer, sizeof(buffer));
    g_formData.technicalPerson = buffer;
    
    GetWindowTextA(GetDlgItem(g_hWnd, ID_POSITION_INPUT), buffer, sizeof(buffer));
    g_formData.position = buffer;
    
    GetWindowTextA(GetDlgItem(g_hWnd, ID_PHONE_INPUT), buffer, sizeof(buffer));
    g_formData.phone = buffer;
    
    GetWindowTextA(GetDlgItem(g_hWnd, ID_EMAIL_INPUT), buffer, sizeof(buffer));
    g_formData.email = buffer;
    
    GetWindowTextA(GetDlgItem(g_hWnd, ID_ADDRESS_INPUT), buffer, sizeof(buffer));
    g_formData.address = buffer;
    
    // Get Issuing Certificate fields
    GetWindowTextA(GetDlgItem(g_hWnd, ID_ISSUING_COMPANY_INPUT), buffer, sizeof(buffer));
    g_formData.issuingCompanyName = buffer;
    
    GetWindowTextA(GetDlgItem(g_hWnd, ID_ISSUING_TECHNICIAN_INPUT), buffer, sizeof(buffer));
    g_formData.issuingTechnicianName = buffer;
    
    GetWindowTextA(GetDlgItem(g_hWnd, ID_ISSUING_LOCATION_INPUT), buffer, sizeof(buffer));
    g_formData.issuingLocation = buffer;
    
    GetWindowTextA(GetDlgItem(g_hWnd, ID_ISSUING_PHONE_INPUT), buffer, sizeof(buffer));
    g_formData.issuingPhone = buffer;
    
    GetWindowTextA(GetDlgItem(g_hWnd, ID_ISSUING_EMAIL_INPUT), buffer, sizeof(buffer));
    g_formData.issuingEmail = buffer;
}


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
    // Append to status log if available
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
    auto start = std::chrono::steady_clock::now(); // Track start time for duration calculation
    
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
                
                while (bytesWritten < totalBytes && !g_cancelRequested) {
                    DWORD toWrite = (DWORD)((totalBytes - bytesWritten) > chunkSize ? chunkSize : (totalBytes - bytesWritten));
                    
                    if (!WriteFile(hDisk, zeroBuffer, toWrite, &written, nullptr)) {
                        DWORD err = GetLastError();
                        notes = "Write failed at " + std::to_string(bytesWritten / (1024*1024)) + " MB. Error: " + std::to_string(err);
                        LogMessage("ERROR: " + notes);
                success = false;
                break;
            }
            
                    bytesWritten += written;
                    
                    
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
    
    // Update g_reportData for report preview
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    
    g_reportData.diskModel = wp->model;
    g_reportData.diskSerial = wp->serial;
    char sizeBuf[64];
    sprintf_s(sizeBuf, "%.2f GB", wp->sizeGB);
    g_reportData.diskSize = sizeBuf;
    g_reportData.busType = wp->busType;
    
    // Erasure method
    if (g_quickWipe) {
        g_reportData.erasureMethod = "Quick Wipe (Partition Table Removal)";
    } else {
        g_reportData.erasureMethod = "Secure Wipe (Zero All Sectors)";
    }
    
    // Verification status
    if (g_verificationMode == 0) {
        g_reportData.verificationStatus = "None";
    } else if (g_verificationMode == 1) {
        g_reportData.verificationStatus = "Quick Verification";
    } else {
        g_reportData.verificationStatus = "Full Verification";
    }
    
    // Date and time
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    struct tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    char dateBuf[64], timeBuf[64];
    strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", &timeinfo);
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &timeinfo);
    g_reportData.erasureDate = dateBuf;
    g_reportData.erasureTime = timeBuf;
    
    // Duration
    int hours = duration.count() / 3600;
    int minutes = (duration.count() % 3600) / 60;
    int seconds = duration.count() % 60;
    if (hours > 0) {
        sprintf_s(sizeBuf, "%d:%02d:%02d", hours, minutes, seconds);
    } else {
        sprintf_s(sizeBuf, "%d:%02d", minutes, seconds);
    }
    g_reportData.erasureDuration = sizeBuf;
    
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
    // Prevent multiple instances of About dialog
    if (g_aboutDialogOpen) {
        return;
    }
    
    g_aboutDialogOpen = true;
    
    std::string aboutText = "PIWIPER - Professional Disk Eraser\n\n";
    aboutText += "Version: " + std::string(PIWIPER_VERSION_STRING) + "\n";
    aboutText += "Build Date: " + std::string(PIWIPER_BUILD_DATE) + " " + std::string(PIWIPER_BUILD_TIME) + "\n";
    aboutText += "Build Config: " + std::string(PIWIPER_BUILD_CONFIG) + "\n\n";
    aboutText += std::string(PIWIPER_APP_DESCRIPTION) + "\n\n";
    aboutText += "Features:\n";
    aboutText += "- Modern UI with gradient design\n";
    aboutText += "- Dual progress tracking\n";
    aboutText += "- Real-time speed display\n";
    aboutText += "- Verification system\n";
    aboutText += "- Comprehensive backup system\n";
    aboutText += "- OS disk protection\n\n";
    aboutText += std::string(PIWIPER_APP_COPYRIGHT) + "\n";
    aboutText += "All rights reserved.\n\n";
    aboutText += "WARNING: This tool permanently destroys data!\n";
    aboutText += "Use with extreme caution and proper authorization.";
    
    MessageBox(g_hWnd, aboutText.c_str(), "About PIWIPER", MB_OK | MB_ICONINFORMATION);
    
    g_aboutDialogOpen = false;
}

// Wipe selected disk
void WipeSelectedDisk() {
    static bool isProcessing = false;
    
    if (isProcessing) {
        return; // Prevent double calls
    }
    
    if (g_isWiping) {
        MessageBoxA(g_hWnd, "A wipe operation is already in progress!", "Busy", MB_OK | MB_ICONWARNING);
        return;
    }
    
    isProcessing = true;
    
    int selected = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
    if (selected == -1) {
        MessageBoxA(g_hWnd, "Please select a disk to wipe.", "No Selection", MB_OK | MB_ICONINFORMATION);
        isProcessing = false;
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
    
    if (!selectedDisk) {
        isProcessing = false;
        return;
    }
    
    // First confirmation
    std::string msg = "WARNING: This will PERMANENTLY DESTROY all data on:\n\n";
    msg += "Disk #" + std::to_string(selectedDisk->number) + "\n";
    msg += "Size: " + std::to_string((int)selectedDisk->sizeGB) + " GB\n";
    msg += "Model: " + selectedDisk->model + "\n";
    msg += "Serial: " + selectedDisk->serial + "\n\n";
    msg += "This action is IRREVERSIBLE!\n\nAre you sure you want to continue?";
    
    int result = MessageBoxA(g_hWnd, msg.c_str(), "CONFIRM DISK WIPE", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
    if (result != IDYES) {
        LogMessage("Wipe cancelled by user");
        isProcessing = false;
        return;
    }
    
    // No additional confirmation needed - user already selected and confirmed
    
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
    
    isProcessing = false; // Reset flag after starting thread
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
                "Verdana");              // FaceName
            
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
                "Verdana");              // FaceName
            
            HFONT hNormalFont = g_hNormalFont;
            
            // Header is drawn in WM_PAINT (no controls needed here)
            
            // Tab control (moved down to account for 90px header)
            // Tab Control removed - using single page layout
            
            // ListView
            // ListView for disk selection (moved down for tabs)
            g_hListView = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, "", 
                WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                20, 130, 1060, 200, hwnd, (HMENU)ID_LISTVIEW, g_hInstance, nullptr);
            
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
            
            // Advanced Options Panel (moved down for tabs)
            CreateWindowA("STATIC", "Advanced Options", WS_VISIBLE | WS_CHILD | SS_LEFT,
                20, 350, 200, 25, hwnd, (HMENU)ID_ADVANCED_OPTIONS_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_ADVANCED_OPTIONS_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            
            // Erasure Method (moved down for tabs)
            CreateWindowA("STATIC", "Erasure Method :", WS_VISIBLE | WS_CHILD | SS_LEFT,
                30, 380, 150, 20, hwnd, (HMENU)ID_ERASURE_METHOD_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_ERASURE_METHOD_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            g_hMethodCombo = CreateWindowExA(0, "COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL,
                190, 375, 350, 200, hwnd, (HMENU)ID_METHOD_COMBO, g_hInstance, nullptr);
            SendMessage(g_hMethodCombo, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            SendMessageA(g_hMethodCombo, CB_ADDSTRING, 0, (LPARAM)"Quick Wipe (Partition Table Removal)");
            SendMessageA(g_hMethodCombo, CB_ADDSTRING, 0, (LPARAM)"Secure Wipe (Zero All Sectors)");
            SendMessage(g_hMethodCombo, CB_SETCURSEL, 0, 0); // Default to Quick Wipe for convenience
            
            // Verification (moved down for tabs)
            CreateWindowA("STATIC", "Verification :", WS_VISIBLE | WS_CHILD | SS_LEFT,
                580, 380, 150, 20, hwnd, (HMENU)ID_VERIFICATION_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_VERIFICATION_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            g_hVerificationCombo = CreateWindowExA(0, "COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL,
                720, 375, 350, 200, hwnd, (HMENU)ID_VERIFICATION_COMBO, g_hInstance, nullptr);
            SendMessage(g_hVerificationCombo, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            SendMessageA(g_hVerificationCombo, CB_ADDSTRING, 0, (LPARAM)"No Verification");
            SendMessageA(g_hVerificationCombo, CB_ADDSTRING, 0, (LPARAM)"Quick Verification");
            SendMessageA(g_hVerificationCombo, CB_ADDSTRING, 0, (LPARAM)"Full Verification");
            SendMessage(g_hVerificationCombo, CB_SETCURSEL, 0, 0);
            
            // Customer Information removed - no tab control
            
            // Progress section (modern style with increased height) - moved down for tabs
            CreateWindowA("STATIC", "Progress:", WS_VISIBLE | WS_CHILD | SS_LEFT,
                30, 410, 80, 20, hwnd, (HMENU)ID_PROGRESS_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_PROGRESS_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            g_hProgressBar = CreateWindowExA(0, "msctls_progress32", "", WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
                120, 410, 800, 25, hwnd, (HMENU)ID_PROGRESS, g_hInstance, nullptr);
            SendMessage(g_hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            SendMessage(g_hProgressBar, PBM_SETBARCOLOR, 0, RGB(0, 120, 212)); // Modern blue
            SendMessage(g_hProgressBar, PBM_SETPOS, 0, 0); // Initialize to 0
            
            CreateWindowA("STATIC", "Fine:", WS_VISIBLE | WS_CHILD | SS_LEFT,
                30, 443, 80, 20, hwnd, (HMENU)ID_FINE_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_FINE_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            g_hProgressBarFine = CreateWindowExA(0, "msctls_progress32", "", WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
                120, 443, 800, 22, hwnd, nullptr, g_hInstance, nullptr);
            SendMessage(g_hProgressBarFine, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            SendMessage(g_hProgressBarFine, PBM_SETBARCOLOR, 0, RGB(16, 160, 80)); // Modern green
            SendMessage(g_hProgressBarFine, PBM_SETPOS, 0, 0); // Initialize to 0
            
            g_hEtaText = CreateWindowA("STATIC", "ETA: --:--:--", WS_VISIBLE | WS_CHILD | SS_LEFT,
                950, 425, 180, 20, hwnd, (HMENU)ID_ETA_TEXT, g_hInstance, nullptr);
            SendMessage(g_hEtaText, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            
            // Buttons (modern style with owner-draw for rounded corners + BS_NOTIFY for hover)
            // Buttons centered and properly aligned
            // Small buttons (Refresh, Exit) - top row, Exit on right (moved down for tabs)
            g_hBtnRefresh = CreateWindowA("BUTTON", "Refresh", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                50, 490, 120, 35, hwnd, (HMENU)ID_BTN_REFRESH, g_hInstance, nullptr);
            
            g_hBtnExit = CreateWindowA("BUTTON", "Exit", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                1000, 490, 120, 35, hwnd, (HMENU)ID_BTN_EXIT, g_hInstance, nullptr);
            
            // Header button (About as small icon)
            CreateWindowA("BUTTON", "i", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                1080, 25, 40, 40, hwnd, (HMENU)ID_ABOUT_DIALOG, g_hInstance, nullptr);
            
            // Tab buttons below header (like in the image) - proper tab appearance
            CreateWindowA("BUTTON", "Erasure", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                20, 90, 120, 30, hwnd, (HMENU)ID_TAB_ERASURE, g_hInstance, nullptr);
            
            CreateWindowA("BUTTON", "Erasure Details", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                150, 90, 120, 30, hwnd, (HMENU)ID_TAB_DETAILS, g_hInstance, nullptr);
            
            CreateWindowA("BUTTON", "Report", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                280, 90, 120, 30, hwnd, (HMENU)ID_TAB_REPORT, g_hInstance, nullptr);
            
            // Erasure Details Form (initially hidden)
            CreateWindowA("STATIC", "COMPANY INFORMATION", WS_CHILD | SS_LEFT,
                30, 130, 300, 25, hwnd, (HMENU)ID_FORM_TITLE_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_FORM_TITLE_LABEL), WM_SETFONT, (WPARAM)g_hTitleFont, TRUE);
            
            // Company Name
            CreateWindowA("STATIC", "Company Name:", WS_CHILD | SS_LEFT,
                30, 160, 120, 20, hwnd, (HMENU)ID_COMPANY_NAME_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_COMPANY_NAME_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT,
                160, 158, 300, 25, hwnd, (HMENU)ID_CUSTOMER_NAME_INPUT, g_hInstance, nullptr);
            
            // Technical Person
            CreateWindowA("STATIC", "Technical Person:", WS_CHILD | SS_LEFT,
                30, 190, 120, 20, hwnd, (HMENU)ID_TECHNICAL_PERSON_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_TECHNICAL_PERSON_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT,
                160, 188, 300, 25, hwnd, (HMENU)ID_TECHNICAL_PERSON_INPUT, g_hInstance, nullptr);
            
            // Position
            CreateWindowA("STATIC", "Position:", WS_CHILD | SS_LEFT,
                30, 220, 120, 20, hwnd, (HMENU)ID_POSITION_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_POSITION_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT,
                160, 218, 300, 25, hwnd, (HMENU)ID_POSITION_INPUT, g_hInstance, nullptr);
            
            // Phone
            CreateWindowA("STATIC", "Phone:", WS_CHILD | SS_LEFT,
                30, 250, 120, 20, hwnd, (HMENU)ID_PHONE_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_PHONE_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT,
                160, 248, 300, 25, hwnd, (HMENU)ID_PHONE_INPUT, g_hInstance, nullptr);
            
            // Email
            CreateWindowA("STATIC", "Email:", WS_CHILD | SS_LEFT,
                30, 280, 120, 20, hwnd, (HMENU)ID_EMAIL_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_EMAIL_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT,
                160, 278, 300, 25, hwnd, (HMENU)ID_EMAIL_INPUT, g_hInstance, nullptr);
            
            // Address
            CreateWindowA("STATIC", "Address:", WS_CHILD | SS_LEFT,
                30, 320, 120, 20, hwnd, (HMENU)ID_ADDRESS_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_ADDRESS_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT | ES_MULTILINE,
                160, 318, 300, 50, hwnd, (HMENU)ID_ADDRESS_INPUT, g_hInstance, nullptr);
            
            // Issuing Certificate Section (Right side) - Initially hidden, shown only in Details tab
            CreateWindowA("STATIC", "ISSUING CERTIFICATE", WS_CHILD | SS_LEFT,
                600, 130, 250, 25, hwnd, (HMENU)ID_ISSUING_CERT_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_ISSUING_CERT_LABEL), WM_SETFONT, (WPARAM)g_hTitleFont, TRUE);
            
            // Issuing Company Name
            CreateWindowA("STATIC", "Company Name:", WS_CHILD | SS_LEFT,
                600, 160, 150, 25, hwnd, (HMENU)ID_ISSUING_COMPANY_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_ISSUING_COMPANY_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT,
                760, 158, 300, 25, hwnd, (HMENU)ID_ISSUING_COMPANY_INPUT, g_hInstance, nullptr);
            
            // Issuing Technician Name
            CreateWindowA("STATIC", "Technician Name:", WS_CHILD | SS_LEFT,
                600, 190, 150, 25, hwnd, (HMENU)ID_ISSUING_TECHNICIAN_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_ISSUING_TECHNICIAN_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT,
                760, 188, 300, 25, hwnd, (HMENU)ID_ISSUING_TECHNICIAN_INPUT, g_hInstance, nullptr);
            
            // Issuing Company Location
            CreateWindowA("STATIC", "Company Location:", WS_CHILD | SS_LEFT,
                600, 220, 150, 25, hwnd, (HMENU)ID_ISSUING_LOCATION_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_ISSUING_LOCATION_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT,
                760, 218, 300, 25, hwnd, (HMENU)ID_ISSUING_LOCATION_INPUT, g_hInstance, nullptr);
            
            // Issuing Company Phone
            CreateWindowA("STATIC", "Company Phone:", WS_CHILD | SS_LEFT,
                600, 250, 150, 25, hwnd, (HMENU)ID_ISSUING_PHONE_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_ISSUING_PHONE_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT,
                760, 248, 300, 25, hwnd, (HMENU)ID_ISSUING_PHONE_INPUT, g_hInstance, nullptr);
            
            // Issuing Company Email
            CreateWindowA("STATIC", "Company Email:", WS_CHILD | SS_LEFT,
                600, 280, 150, 25, hwnd, (HMENU)ID_ISSUING_EMAIL_LABEL, g_hInstance, nullptr);
            SendMessage(GetDlgItem(hwnd, ID_ISSUING_EMAIL_LABEL), WM_SETFONT, (WPARAM)hNormalFont, TRUE);
            CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_LEFT,
                760, 278, 300, 25, hwnd, (HMENU)ID_ISSUING_EMAIL_INPUT, g_hInstance, nullptr);
            
            // Save Form Button
            CreateWindowA("BUTTON", "Save Form", WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                30, 450, 150, 40, hwnd, (HMENU)ID_SAVE_FORM_BUTTON, g_hInstance, nullptr);
            
            // Report Section (initially hidden) - BitRaser exact style
            // Right side header with logo (removed "Report :" text)
            
            // Report section - only preview area and buttons (removed all static labels)
            
            // Report Buttons (below preview area) - Initially hidden
            // Preview Report: 30-220 (190px wide), Export PDF: 230-380 (150px wide)
            CreateWindowA("BUTTON", "Preview Report", WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                30, 490, 190, 40, hwnd, (HMENU)ID_PREVIEW_BUTTON, g_hInstance, nullptr);
            
            CreateWindowA("BUTTON", "Export PDF", WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                230, 490, 150, 40, hwnd, (HMENU)ID_EXPORT_PDF_BUTTON, g_hInstance, nullptr);
            
            // Set fonts for report buttons
            SendMessage(GetDlgItem(hwnd, ID_PREVIEW_BUTTON), WM_SETFONT, (WPARAM)g_hNormalFont, TRUE);
            SendMessage(GetDlgItem(hwnd, ID_EXPORT_PDF_BUTTON), WM_SETFONT, (WPARAM)g_hNormalFont, TRUE);
            
            // Report Preview Area (initially hidden) - RichEdit Control
            // Width adjusted to fill available space (window width 1150 - left margin 30 - right margin 20 = 1100)
            CreateWindowA("RichEdit20A", "", WS_CHILD | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
                30, 130, 1100, 300, hwnd, (HMENU)ID_REPORT_PREVIEW_AREA, g_hInstance, nullptr);
            // Set monospace font for proper alignment
            HFONT hMonospaceFont = CreateFontA(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                FF_MODERN | FIXED_PITCH, "Consolas");
            SendMessage(GetDlgItem(hwnd, ID_REPORT_PREVIEW_AREA), WM_SETFONT, (WPARAM)hMonospaceFont, TRUE);
            
            // Main action buttons (Erase, Cancel) - center of window (moved down for tabs)
            g_hBtnWipe = CreateWindowA("BUTTON", "Erase", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                350, 480, 150, 50, hwnd, (HMENU)ID_BTN_WIPE, g_hInstance, nullptr);
            
            g_hBtnCancel = CreateWindowA("BUTTON", "Cancel", WS_CHILD | BS_OWNERDRAW | BS_NOTIFY,
                520, 480, 150, 50, hwnd, (HMENU)ID_BTN_CANCEL, g_hInstance, nullptr);
            EnableWindow(g_hBtnCancel, FALSE); // Initially disabled
            
            // Status log at bottom (multiline, read-only, auto-scroll) - moved down for tabs
            g_hStatusText = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", 
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                10, 550, 1120, 200, hwnd, (HMENU)ID_STATUS_TEXT, g_hInstance, nullptr);
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
            
            // Load form data from file
            LoadFormData();
            UpdateFormFields();
            
            // Load report data from file
            LoadReportData();
            
            RefreshDiskList();
            loadReports();
            
            // Hide Issuing Certificate elements initially (Erasure tab is active by default)
            ShowWindow(GetDlgItem(hwnd, ID_ISSUING_CERT_LABEL), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_ISSUING_COMPANY_LABEL), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_ISSUING_COMPANY_INPUT), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_ISSUING_TECHNICIAN_LABEL), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_ISSUING_TECHNICIAN_INPUT), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_ISSUING_LOCATION_LABEL), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_ISSUING_LOCATION_INPUT), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_ISSUING_PHONE_LABEL), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_ISSUING_PHONE_INPUT), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_ISSUING_EMAIL_LABEL), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_ISSUING_EMAIL_INPUT), SW_HIDE);
            
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
                OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_SWISS | VARIABLE_PITCH, "Verdana");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);
            
            RECT titleRect = {120, 15, 1150, 90};
            DrawTextA(hdc, "PIWIPER", -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            
            // Subtitle with better contrast
            SelectObject(hdc, hOldFont);
            HFONT hSubFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_SWISS | VARIABLE_PITCH, "Verdana");
            SelectObject(hdc, hSubFont);
            SetTextColor(hdc, RGB(220, 235, 255)); // Lighter for better contrast
            RECT subRect = {600, 15, 1150, 90};
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
            
            if (id == ID_BTN_REFRESH) {
                    LogMessage("Refresh button clicked");
                    try {
                    RefreshDiskList();
                    } catch (...) {
                        AppendStatusMessage("ERROR: Failed to refresh disk list");
                }
            } else if (id == ID_BTN_WIPE) {
                WipeSelectedDisk();
            } else if (id == ID_BTN_CANCEL) {
                if (g_isWiping) {
                        g_cancelRequested = true;
                    LogMessage("Cancellation requested by user");
                    SetWindowTextA(g_hStatusText, "Cancelling operation - please wait...");
                }
            } else if (id == ID_BTN_EXIT) {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            } else if (id == ID_ABOUT_DIALOG) {
                ShowAboutDialog();
            } else if (id == ID_TAB_ERASURE) {
                // Switch to Erasure tab - show ListView, Advanced Options, and buttons
                g_activeTab = 1028; // ID_TAB_ERASURE
                ShowWindow(g_hListView, SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_METHOD_COMBO), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_VERIFICATION_COMBO), SW_SHOW);
                ShowWindow(g_hProgressBar, SW_SHOW);
                ShowWindow(g_hProgressBarFine, SW_SHOW);
                ShowWindow(g_hEtaText, SW_SHOW);
                ShowWindow(g_hBtnWipe, SW_SHOW);
                ShowWindow(g_hBtnCancel, SW_HIDE); // Hide cancel button initially
                ShowWindow(g_hBtnRefresh, SW_SHOW); // Show Refresh button
                // Show Advanced Options labels
                ShowWindow(GetDlgItem(hwnd, ID_ADVANCED_OPTIONS_LABEL), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_ERASURE_METHOD_LABEL), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_VERIFICATION_LABEL), SW_SHOW);
                // Show Progress labels
                ShowWindow(GetDlgItem(hwnd, ID_PROGRESS_LABEL), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_FINE_LABEL), SW_SHOW);
                // Hide form controls and labels
                ShowWindow(GetDlgItem(hwnd, ID_FORM_TITLE_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_COMPANY_NAME_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_TECHNICAL_PERSON_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_POSITION_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_PHONE_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_EMAIL_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_SAVE_FORM_BUTTON), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CUSTOMER_NAME_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_TECHNICAL_PERSON_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_POSITION_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_PHONE_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_EMAIL_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ADDRESS_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ADDRESS_INPUT), SW_HIDE);
                // Hide Issuing Certificate elements
                ShowWindow(GetDlgItem(hwnd, ID_ISSUING_CERT_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ISSUING_COMPANY_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ISSUING_COMPANY_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ISSUING_TECHNICIAN_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ISSUING_TECHNICIAN_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ISSUING_LOCATION_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ISSUING_LOCATION_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ISSUING_PHONE_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ISSUING_PHONE_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ISSUING_EMAIL_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ISSUING_EMAIL_INPUT), SW_HIDE);
                // Hide report elements - only preview area and buttons (static labels removed)
                ShowWindow(GetDlgItem(hwnd, ID_PREVIEW_BUTTON), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_EXPORT_PDF_BUTTON), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_REPORT_PREVIEW_AREA), SW_HIDE);
                // Force redraw to update tab colors
                InvalidateRect(hwnd, nullptr, TRUE);
            } else if (id == ID_TAB_DETAILS) {
                // Switch to Erasure Details tab - hide ListView, Advanced Options, and buttons, show form
                g_activeTab = 1029; // ID_TAB_DETAILS
                ShowWindow(g_hListView, SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_METHOD_COMBO), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_VERIFICATION_COMBO), SW_HIDE);
                ShowWindow(g_hProgressBar, SW_HIDE);
                ShowWindow(g_hProgressBarFine, SW_HIDE);
                ShowWindow(g_hEtaText, SW_HIDE);
                ShowWindow(g_hBtnWipe, SW_HIDE);
                ShowWindow(g_hBtnCancel, SW_HIDE);
                ShowWindow(g_hBtnRefresh, SW_HIDE); // Hide Refresh button
                // Hide Advanced Options labels
                ShowWindow(GetDlgItem(hwnd, ID_ADVANCED_OPTIONS_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ERASURE_METHOD_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_VERIFICATION_LABEL), SW_HIDE);
                // Hide Progress labels
                ShowWindow(GetDlgItem(hwnd, ID_PROGRESS_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_FINE_LABEL), SW_HIDE);
                // Show form controls and labels
                ShowWindow(GetDlgItem(hwnd, ID_FORM_TITLE_LABEL), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_COMPANY_NAME_LABEL), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_TECHNICAL_PERSON_LABEL), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_POSITION_LABEL), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_PHONE_LABEL), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_EMAIL_LABEL), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_SAVE_FORM_BUTTON), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CUSTOMER_NAME_INPUT), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_TECHNICAL_PERSON_INPUT), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_POSITION_INPUT), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_PHONE_INPUT), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_EMAIL_INPUT), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_ADDRESS_LABEL), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_ADDRESS_INPUT), SW_SHOW);
                // Show Issuing Certificate elements
                HWND hCertLabel = GetDlgItem(hwnd, ID_ISSUING_CERT_LABEL);
                HWND hCompanyLabel = GetDlgItem(hwnd, ID_ISSUING_COMPANY_LABEL);
                HWND hCompanyInput = GetDlgItem(hwnd, ID_ISSUING_COMPANY_INPUT);
                HWND hTechnicianLabel = GetDlgItem(hwnd, ID_ISSUING_TECHNICIAN_LABEL);
                HWND hTechnicianInput = GetDlgItem(hwnd, ID_ISSUING_TECHNICIAN_INPUT);
                HWND hLocationLabel = GetDlgItem(hwnd, ID_ISSUING_LOCATION_LABEL);
                HWND hLocationInput = GetDlgItem(hwnd, ID_ISSUING_LOCATION_INPUT);
                HWND hPhoneLabel = GetDlgItem(hwnd, ID_ISSUING_PHONE_LABEL);
                HWND hPhoneInput = GetDlgItem(hwnd, ID_ISSUING_PHONE_INPUT);
                HWND hEmailLabel = GetDlgItem(hwnd, ID_ISSUING_EMAIL_LABEL);
                HWND hEmailInput = GetDlgItem(hwnd, ID_ISSUING_EMAIL_INPUT);
                
                if (hCertLabel) {
                    ShowWindow(hCertLabel, SW_SHOW);
                    SetWindowPos(hCertLabel, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    UpdateWindow(hCertLabel);
                }
                if (hCompanyLabel) {
                    ShowWindow(hCompanyLabel, SW_SHOW);
                    SetWindowPos(hCompanyLabel, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    UpdateWindow(hCompanyLabel);
                }
                if (hCompanyInput) {
                    ShowWindow(hCompanyInput, SW_SHOW);
                    SetWindowPos(hCompanyInput, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    UpdateWindow(hCompanyInput);
                }
                if (hTechnicianLabel) {
                    ShowWindow(hTechnicianLabel, SW_SHOW);
                    SetWindowPos(hTechnicianLabel, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    UpdateWindow(hTechnicianLabel);
                }
                if (hTechnicianInput) {
                    ShowWindow(hTechnicianInput, SW_SHOW);
                    SetWindowPos(hTechnicianInput, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    UpdateWindow(hTechnicianInput);
                }
                if (hLocationLabel) {
                    ShowWindow(hLocationLabel, SW_SHOW);
                    SetWindowPos(hLocationLabel, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    UpdateWindow(hLocationLabel);
                }
                if (hLocationInput) {
                    ShowWindow(hLocationInput, SW_SHOW);
                    SetWindowPos(hLocationInput, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    UpdateWindow(hLocationInput);
                }
                if (hPhoneLabel) {
                    ShowWindow(hPhoneLabel, SW_SHOW);
                    SetWindowPos(hPhoneLabel, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    UpdateWindow(hPhoneLabel);
                }
                if (hPhoneInput) {
                    ShowWindow(hPhoneInput, SW_SHOW);
                    SetWindowPos(hPhoneInput, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    UpdateWindow(hPhoneInput);
                }
                if (hEmailLabel) {
                    ShowWindow(hEmailLabel, SW_SHOW);
                    SetWindowPos(hEmailLabel, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    UpdateWindow(hEmailLabel);
                }
                if (hEmailInput) {
                    ShowWindow(hEmailInput, SW_SHOW);
                    SetWindowPos(hEmailInput, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    UpdateWindow(hEmailInput);
                }
                
                // Force redraw of the entire window
                InvalidateRect(hwnd, nullptr, TRUE);
                UpdateWindow(hwnd);
                // Hide report elements - only preview area and buttons (static labels removed)
                ShowWindow(GetDlgItem(hwnd, ID_PREVIEW_BUTTON), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_EXPORT_PDF_BUTTON), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_REPORT_PREVIEW_AREA), SW_HIDE);
                // Update form fields with loaded data
                UpdateFormFields();
                // Force redraw to update tab colors
                InvalidateRect(hwnd, nullptr, TRUE);
            } else if (id == ID_TAB_REPORT) {
                // Switch to Report tab - show report elements
                g_activeTab = 1030; // ID_TAB_REPORT
                ShowWindow(g_hListView, SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_METHOD_COMBO), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_VERIFICATION_COMBO), SW_HIDE);
                ShowWindow(g_hProgressBar, SW_HIDE);
                ShowWindow(g_hProgressBarFine, SW_HIDE);
                ShowWindow(g_hEtaText, SW_HIDE);
                ShowWindow(g_hBtnWipe, SW_HIDE);
                ShowWindow(g_hBtnCancel, SW_HIDE);
                ShowWindow(g_hBtnRefresh, SW_HIDE); // Hide Refresh button
                // Hide Advanced Options labels
                ShowWindow(GetDlgItem(hwnd, ID_ADVANCED_OPTIONS_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ERASURE_METHOD_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_VERIFICATION_LABEL), SW_HIDE);
                // Hide Progress labels
                ShowWindow(GetDlgItem(hwnd, ID_PROGRESS_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_FINE_LABEL), SW_HIDE);
                // Hide form controls and labels
                ShowWindow(GetDlgItem(hwnd, ID_FORM_TITLE_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_COMPANY_NAME_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_TECHNICAL_PERSON_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_POSITION_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_PHONE_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_EMAIL_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_SAVE_FORM_BUTTON), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CUSTOMER_NAME_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_TECHNICAL_PERSON_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_POSITION_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_PHONE_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_EMAIL_INPUT), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ADDRESS_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_ADDRESS_INPUT), SW_HIDE);
                // Show report elements - only preview area and buttons (static labels removed)
                ShowWindow(GetDlgItem(hwnd, ID_PREVIEW_BUTTON), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_EXPORT_PDF_BUTTON), SW_SHOW);
                // Show preview area
                ShowWindow(GetDlgItem(hwnd, ID_REPORT_PREVIEW_AREA), SW_SHOW);
                // Update report data
                UpdateReportFields();
                // Update preview area with report content
                UpdateReportPreview();
                // Force redraw to update tab colors
                InvalidateRect(hwnd, nullptr, TRUE);
            } else if (id == ID_SAVE_FORM_BUTTON) {
                static bool isSaving = false;
                if (isSaving) return 0; // Prevent double click
                isSaving = true;
                GetFormDataFromFields();
                SaveFormData(true); // Show message
                isSaving = false;
            } else if (id == ID_PREVIEW_BUTTON) {
                ShowPreviewDialog();
            } else if (id == ID_EXPORT_PDF_BUTTON) {
                MessageBoxA(hwnd, "PDF export functionality will be implemented soon!", "Export PDF", MB_OK | MB_ICONINFORMATION);
            } else if (id == ID_METHOD_COMBO && notifyCode == CBN_SELCHANGE) {
                int sel = SendMessage(g_hMethodCombo, CB_GETCURSEL, 0, 0);
                if (sel == 0) {
                    g_quickWipe = true;
                    LogMessage("Quick wipe mode selected (fast - partition table removal)");
                } else if (sel == 1) {
                    g_quickWipe = false;
                    LogMessage("Secure wipe mode selected (slow - zeros all sectors - may take hours!)");
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
            
            
            // Update progress bars
            if (g_hProgressBar) {
                SendMessageA(g_hProgressBar, PBM_SETPOS, progress, 0);
                // Force redraw
                InvalidateRect(g_hProgressBar, NULL, TRUE);
                UpdateWindow(g_hProgressBar);
            }
            if (g_hProgressBarFine) {
                // Fine progress bar cycles 0-100% every 5MB
                SendMessageA(g_hProgressBarFine, PBM_SETPOS, fineProgress, 0);
                // Force redraw
                InvalidateRect(g_hProgressBarFine, NULL, TRUE);
                UpdateWindow(g_hProgressBarFine);
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
            
            // Update report preview with latest wipe data
            UpdateReportPreview();
            
            RefreshDiskList();
            loadReports();
            return 0;
        }
        
        case WM_CLOSE:
            if (g_isWiping) {
                MessageBoxA(hwnd, "Please wait for the wipe operation to complete.", "Operation In Progress", MB_OK | MB_ICONWARNING);
                return 0;
            }
            // Save form data before closing
            GetFormDataFromFields();
            SaveFormData(false); // Silent save
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
                } else if (dis->hwndItem == g_hBtnExit) {
                    baseColor = RGB(80, 80, 80);     // Dark gray for Exit
                    textColor = RGB(255, 255, 255);
                    btnText = "Exit";
                } else if (dis->hwndItem == GetDlgItem(g_hWnd, ID_SAVE_FORM_BUTTON)) {
                    baseColor = RGB(0, 120, 212);    // Blue for Save
                    textColor = RGB(255, 255, 255);
                    btnText = "Save Form";
                    hFont = g_hTitleFont;
                } else if (dis->hwndItem == GetDlgItem(g_hWnd, ID_PREVIEW_BUTTON)) {
                    baseColor = RGB(0, 120, 212);    // Blue for Preview
                    textColor = RGB(255, 255, 255);
                    btnText = "Preview Report";
                    hFont = g_hTitleFont;
                } else if (dis->hwndItem == GetDlgItem(g_hWnd, ID_EXPORT_PDF_BUTTON)) {
                    baseColor = RGB(0, 120, 212);    // Blue for Export
                    textColor = RGB(255, 255, 255);
                    btnText = "Export PDF";
                    hFont = g_hTitleFont;
                } else if (dis->hwndItem == GetDlgItem(g_hWnd, ID_TAB_ERASURE)) {
                    // Check if this is the active tab
                    if (g_activeTab == 1028) { // ID_TAB_ERASURE
                        baseColor = RGB(0, 120, 212);  // Blue for active
                        textColor = RGB(255, 255, 255);
                    } else {
                        baseColor = RGB(240, 240, 240);  // Light gray for inactive
                        textColor = RGB(80, 80, 80);
                    }
                    btnText = "Erasure";
                } else if (dis->hwndItem == GetDlgItem(g_hWnd, ID_TAB_DETAILS)) {
                    // Check if this is the active tab
                    if (g_activeTab == 1029) { // ID_TAB_DETAILS
                        baseColor = RGB(0, 120, 212);  // Blue for active
                        textColor = RGB(255, 255, 255);
                    } else {
                        baseColor = RGB(240, 240, 240);  // Light gray for inactive
                        textColor = RGB(80, 80, 80);
                    }
                    btnText = "Erasure Details";
                } else if (dis->hwndItem == GetDlgItem(g_hWnd, ID_TAB_REPORT)) {
                    // Check if this is the active tab
                    if (g_activeTab == 1030) { // ID_TAB_REPORT
                        baseColor = RGB(0, 120, 212);  // Blue for active
                        textColor = RGB(255, 255, 255);
                    } else {
                        baseColor = RGB(240, 240, 240);  // Light gray for inactive
                        textColor = RGB(80, 80, 80);
                    }
                    btnText = "Report";
                } else if (dis->hwndItem == GetDlgItem(g_hWnd, ID_ABOUT_DIALOG)) {
                    baseColor = RGB(255, 255, 255);  // White background
                    textColor = RGB(0, 120, 215);    // Blue text
                    btnText = "i";
                } else {
                    baseColor = RGB(80, 80, 80);   // Dark gray for other buttons
                    textColor = RGB(255, 255, 255);
                    btnText = "Refresh";
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
                
                // Draw button shape
                RECT btnRect = dis->rcItem;
                if (dis->itemState & ODS_SELECTED) {
                    // Offset for pressed effect
                    btnRect.top += 1;
                    btnRect.left += 1;
                }
                
                // Special drawing for different button types
                if (dis->hwndItem == GetDlgItem(g_hWnd, ID_ABOUT_DIALOG)) {
                    // Draw modern rounded square for About button (6px radius)
                    RoundRect(dis->hDC, btnRect.left, btnRect.top, 
                             btnRect.right, btnRect.bottom, 6, 6);
                } else if (dis->hwndItem == GetDlgItem(g_hWnd, ID_TAB_ERASURE) ||
                          dis->hwndItem == GetDlgItem(g_hWnd, ID_TAB_DETAILS) ||
                          dis->hwndItem == GetDlgItem(g_hWnd, ID_TAB_REPORT)) {
                    // Simple tab design - just rounded rectangles
                    RECT tabRect = btnRect;
                    
                    // Draw simple rounded rectangle
                    RoundRect(dis->hDC, tabRect.left, tabRect.top, 
                             tabRect.right, tabRect.bottom, 4, 4);
                } else {
                    // Draw rounded rectangle for all other buttons (8px radius)
                RoundRect(dis->hDC, btnRect.left, btnRect.top, 
                         btnRect.right, btnRect.bottom, 8, 8);
                }
                
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
    
    // Load RichEdit library
    LoadLibraryA("riched20.dll");
    
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

