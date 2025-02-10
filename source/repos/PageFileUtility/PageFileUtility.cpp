#include <iostream>
#include <cstdlib>
#include <string>
#include <windows.h>
#include <vector>

void show_banner() {
    std::cout << "-----------------------------------\n";
    std::cout << "Author: Yoxi1\n";
    std::cout << "GitHub: https://github.com/Yoxi228\n";
    std::cout << "-----------------------------------\n";
}

void show_recommendation(int ram_size) {
    if (ram_size == 16) {
        std::cout << "Recommended pagefile size: 30,000 MB\n";
    }
    else if (ram_size == 32) {
        std::cout << "Recommended pagefile size: 40,000 MB\n";
    }
    else if (ram_size == 64) {
        std::cout << "Recommended pagefile size: 60,000 MB\n";
    }
    else {
        std::cout << "Recommended pagefile size: choose manually.\n";
    }
}

// Function to get available drives in the system
std::vector<std::wstring> get_available_drives() {
    std::vector<std::wstring> drives;
    DWORD drive_mask = GetLogicalDrives(); // Get the drives bitmask

    for (char drive = 'A'; drive <= 'Z'; ++drive) {
        if (drive_mask & (1 << (drive - 'A'))) {
            drives.push_back(std::wstring(1, drive) + L":\\");
        }
    }

    return drives;
}

bool set_pagefile_size(const std::wstring& drive, int min_size, int max_size) {
    // Open registry key to edit pagefile settings
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
        0, KEY_SET_VALUE, &hKey);

    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to open the registry.\n";
        return false;
    }

    // Format the value for PagingFiles: drive:\\pagefile.sys MIN MAX
    std::wstring pagefile_value = drive + L"pagefile.sys " + std::to_wstring(min_size) + L" " + std::to_wstring(max_size);

    // Set the value for PagingFiles
    result = RegSetValueEx(hKey, L"PagingFiles", 0, REG_SZ, (const BYTE*)pagefile_value.c_str(),
        (pagefile_value.size() + 1) * sizeof(wchar_t));

    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to set the pagefile size in the registry.\n";
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

bool ask_for_reboot() {
    char choice;
    std::cout << "Do you want to restart the system to apply the changes? (y/n): ";
    std::cin >> choice;
    return (choice == 'y' || choice == 'Y');
}

int main() {
    show_banner();

    int ram_size;
    std::cout << "Enter the size of your RAM (in GB): ";
    std::cin >> ram_size;

    show_recommendation(ram_size);

    int min_size, max_size;
    std::cout << "Enter the minimum pagefile size (in MB): ";
    std::cin >> min_size;

    std::cout << "Enter the maximum pagefile size (in MB): ";
    std::cin >> max_size;

    // Get available drives
    std::vector<std::wstring> drives = get_available_drives();

    if (drives.empty()) {
        std::cout << "No available drives found.\n";
        return 1;
    }

    // Display available drives to the user
    std::cout << "Select a drive for the pagefile:\n";
    for (size_t i = 0; i < drives.size(); ++i) {
        std::wcout << i + 1 << ". " << drives[i] << std::endl;
    }

    int drive_choice;
    std::cout << "Enter the number of the drive you want to select: ";
    std::cin >> drive_choice;

    if (drive_choice < 1 || drive_choice > drives.size()) {
        std::cout << "Invalid choice.\n";
        return 1;
    }

    std::wstring selected_drive = drives[drive_choice - 1];

    // Set the pagefile size on the selected drive
    if (set_pagefile_size(selected_drive, min_size, max_size)) {
        std::cout << "Pagefile size in the registry was successfully updated.\n";
    }
    else {
        std::cout << "Failed to update the pagefile size.\n";
    }

    if (ask_for_reboot()) {
        std::cout << "Restarting the system...\n";
        system("shutdown /r /t 0");
    }

    return 0;
}
