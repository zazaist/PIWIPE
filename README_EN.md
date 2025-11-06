# PIWIPER - Professional Disk Eraser

Modern, secure and user-friendly disk wiping tool. Designed for safely erasing all types of drives (external, internal, USB, SSD, HDD) - a professional data destruction utility.

**PIWIPER** is a Windows-based disk cleaning software developed with a focus on data security and privacy. It is specifically designed for situations where sensitive data needs to be permanently deleted.

> **Türkçe Versiyon:** [README.md](README.md) | **English Version:** This file

## ⚠️ WARNING
This operation is **irreversible**. If you select the wrong disk, all data will be permanently deleted. Make absolutely sure you have targeted the correct disk before proceeding.

## ✨ Features

### 🎨 Modern Interface
- **Gradient header** and modern design
- **Segoe UI** fonts for professional appearance
- **Rounded buttons** and hover effects
- **Responsive layout** and centered positioning

### 💿 Disk Management
- **Smart OS disk detection** - System disks are automatically hidden
- **All disk types supported** - HDD, SSD, USB, external, internal drives
- **Detailed disk information** - Model, serial number, size, bus type, status
- **Real-time disk scanning** and updates
- **Disk health monitoring** - Health status and availability check

### 🔄 Wiping Methods
- **Quick Wipe** - Fast partition table removal (recommended for most cases)
- **Full Wipe** - Complete disk overwrite with random data
- **Custom methods** - User-defined wiping patterns

### 🎯 Use Cases
- **Corporate data destruction** - Secure cleaning of company computers
- **Personal data protection** - Permanent deletion of sensitive personal data
- **Disk reuse** - Secure cleaning of old drives
- **Data security compliance** - GDPR, CCPA, and other data protection regulations
- **Forensic cleaning** - Data cleaning before forensic analysis
- **Test environments** - Cleaning development and test drives

### 📊 Progress Tracking
- **Dual progress bars** - Overall and detailed progress
- **Real-time speed display** - MB/s and ETA
- **Percentage indicators** - Visual progress representation
- **Status messages** - Detailed operation feedback

### 🛡️ Safety Features
- **OS disk protection** - Prevents accidental system disk selection
- **Confirmation dialogs** - Multiple safety checks
- **Operation logging** - Detailed wipe operation logs
- **Error handling** - Comprehensive error management

### 🔧 Technical Features
- **Low-level disk access** - Direct disk manipulation
- **Multi-threaded operations** - Non-blocking UI during wipe
- **Memory management** - Efficient resource usage
- **Cross-platform compatibility** - Windows 10/11 support

## 🚀 Installation

### Prerequisites
- Windows 10 or Windows 11
- Administrator privileges
- Visual Studio Build Tools 2022 (for compilation)

### Build Instructions

1. **Clone the repository:**
   ```bash
   git clone https://github.com/zazaist/PIWIPE.git
   cd PIWIPE
   ```

2. **Install Visual Studio Build Tools:**
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Install "C++ build tools" workload

3. **Compile the application:**
   ```batch
   build-gui.bat
   ```

4. **Run as Administrator:**
   ```batch
   piwiper-gui.exe
   ```

## 📖 Usage

### Basic Usage

1. **Launch the application** as Administrator
2. **Select target disk** from the list (OS disks are hidden)
3. **Choose wiping method:**
   - Quick Wipe (recommended)
   - Full Wipe (thorough)
4. **Click "Start Wipe"** button
5. **Monitor progress** in real-time
6. **Wait for completion** and verification

### Advanced Usage

- **Custom wipe patterns** - Define your own wiping sequences
- **Batch operations** - Process multiple disks
- **Log analysis** - Review detailed operation logs
- **Settings configuration** - Customize application behavior

## 📁 File Structure

```
PIWIPER/
├── src/
│   ├── main_gui.cpp          # Main GUI application
│   ├── version.h             # Version information
│   ├── version.rc            # Version resources
│   ├── piwiper.rc            # Application resources
│   ├── piwiper_resource.h    # Resource definitions
│   └── piwiper-gui.manifest  # Administrator manifest
├── build-gui.bat             # Build script
├── version-manager.bat       # Version management tool
├── backup-manager.bat        # Backup management tool
├── README.md                 # Turkish documentation
├── README_EN.md              # English documentation
└── piwiper-gui.exe           # Compiled executable
```

## 🔧 Technical Details

### Architecture
- **Language:** C++17
- **Framework:** WinAPI (Windows API)
- **UI Library:** Native Windows controls
- **Compiler:** MSVC (Microsoft Visual C++)
- **Build System:** Batch scripts

### Dependencies
- **Windows SDK** - For WinAPI functions
- **Common Controls** - For modern UI elements
- **Shell32.lib** - For disk operations
- **Advapi32.lib** - For system functions

### Performance
- **Memory Usage:** ~10-15 MB
- **CPU Usage:** Low during idle, high during wipe
- **Disk I/O:** Direct disk access for maximum speed
- **Network:** No network dependencies

## 🛡️ Security Notes

### Data Destruction
- **Quick Wipe:** Removes partition table (fast, effective)
- **Full Wipe:** Overwrites entire disk with random data
- **Verification:** Ensures complete data removal

### Safety Measures
- **OS Protection:** System disks cannot be selected
- **Confirmation:** Multiple safety dialogs
- **Logging:** All operations are logged
- **Error Handling:** Comprehensive error management

### Best Practices
- **Backup important data** before wiping
- **Verify disk selection** multiple times
- **Use in controlled environment** only
- **Follow data destruction policies**

## 🐛 Troubleshooting

### Common Issues

**Problem:** Application won't start
- **Solution:** Run as Administrator

**Problem:** No disks visible
- **Solution:** Check disk connections and permissions

**Problem:** Wipe operation fails
- **Solution:** Ensure disk is not in use by other applications

**Problem:** Slow performance
- **Solution:** Close unnecessary applications

### Error Codes

| Code | Description | Solution |
|------|-------------|----------|
| 5 | Access Denied | Run as Administrator |
| 32 | File in Use | Close other disk applications |
| 87 | Invalid Parameter | Check disk selection |
| 1119 | Cannot Access | Verify disk permissions |

## 📈 Version History

### v2.1.1 (Current)
- ✅ **Report tab improvements** - All fields automatically populated
- ✅ **Company Information fields** - Missing fields added
- ✅ **Automatic report update** - Report updates after disk wipe
- ✅ **Attributes field** - Whole Disk Erasure information added
- ✅ **Capacity and Hard Disk Type** - Device Details fields populated
- ✅ **COMPANY DETAILS removed** - Duplicate section cleaned up
- ✅ **Backup script fixes** - Hanging issues resolved

### v2.1.0
- **Comprehensive versioning system** with semantic versioning
- **About dialog** with detailed version information
- **Version resources** for Windows file properties
- **Version manager tool** for easy version updates
- **Release notes generator** for documentation
- **Git tag integration** for version tracking
- **Build system enhancement** with version integration

### v2.0.0
- **Modern UI redesign** with gradient header
- **Dual progress tracking** system
- **Real-time speed display** and ETA
- **Enhanced safety features** and OS protection
- **Comprehensive logging** system
- **Backup management** tools

### v1.0.0
- **Initial release** with basic functionality
- **Disk detection** and selection
- **Basic wiping** operations
- **Simple UI** interface

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📞 Support

For support, questions, or bug reports:

- **GitHub Issues:** [Create an issue](https://github.com/zazaist/PIWIPE/issues)
- **Email:** [Your email here]
- **Documentation:** Check this README and code comments

## ⚠️ Disclaimer

This tool is designed for legitimate data destruction purposes only. Users are responsible for:

- **Legal compliance** with local data protection laws
- **Proper authorization** before wiping any disk
- **Data backup** before performing any destructive operations
- **Understanding risks** associated with data destruction

The developers are not responsible for any data loss or misuse of this tool.

---

**PIWIPER** - Professional Disk Eraser  
*Secure. Reliable. Professional.*

**Version:** 2.1.0.1  
**Build Date:** [Current Build Date]  
**Copyright:** © 2025 PIWIPER Development Team
