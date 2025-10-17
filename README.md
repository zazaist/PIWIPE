# PIWIPER - Professional Disk Eraser

Modern, güvenli ve kullanıcı dostu disk silme aracı. Harici/USB diskleri güvenli bir şekilde silmek için tasarlanmıştır.

## ⚠️ UYARI
Bu işlem **geri döndürülemez**. Yanlış diski seçmeniz durumunda tüm veriler kalıcı olarak silinir. Devam etmeden önce doğru diski hedeflediğinizden kesinlikle emin olun.

## ✨ Özellikler

### 🎨 Modern Arayüz
- **Gradient header** ve modern tasarım
- **Segoe UI** fontları ile profesyonel görünüm
- **Rounded buttons** ve hover efektleri
- **Responsive layout** ve merkezi konumlandırma

### 💿 Disk Yönetimi
- **Akıllı OS disk tespiti** - Sistem diskleri otomatik gizlenir
- **Detaylı disk bilgileri** - Model, seri numarası, boyut, bus tipi
- **Real-time disk tarama** ve güncelleme

### 🔄 Silme Modları
- **Quick Wipe** - Partition table temizleme (hızlı, ~3 saniye)
- **Secure Wipe** - Tüm sektörlere sıfır yazma (güvenli, saatler sürebilir)

### 📊 İlerleme Takibi
- **Dual progress bars** - Coarse ve fine progress
- **Real-time speed display** - MB/s cinsinden hız
- **ETA calculation** - Tahmini süre gösterimi
- **Smooth animations** - Hassas progress güncellemeleri

### ✅ Doğrulama
- **Quick Verification** - İlk 100MB kontrolü
- **Full Verification** - Tüm disk kontrolü
- **Zero-byte verification** - Silme işleminin doğruluğu

### 🎛️ Kontroller
- **Cancel button** - İşlemi güvenli şekilde iptal etme
- **Status log** - Timestamp'li detaylı log
- **Customer information** - Müşteri bilgileri girişi

## 🚀 Kurulum ve Kullanım

### Gereksinimler
- **Windows 10/11**
- **Visual Studio 2022** (derleme için)
- **Administrator hakları** (disk erişimi için)

### Derleme
```cmd
# Developer Command Prompt açın
cd "C:\Users\ozgur\OneDrive\Desktop\piwiper"
.\build-gui.bat
```

### Çalıştırma
```cmd
# Administrator olarak çalıştırın
.\piwiper-gui.exe
```

## 📋 Kullanım Adımları

1. **Uygulamayı başlatın** (Administrator olarak)
2. **Disk listesini kontrol edin** - OS diskleri otomatik gizlenir
3. **Silme modunu seçin:**
   - **Quick Wipe** - Hızlı (partition table)
   - **Secure Wipe** - Güvenli (tüm sektörler)
4. **Doğrulama seçin:**
   - **No Verification** - Doğrulama yok
   - **Quick Verification** - İlk 100MB
   - **Full Verification** - Tüm disk
5. **Hedef diski seçin** ve **Erase** butonuna basın
6. **Onay verin** - Disk bilgilerini kontrol edin
7. **İlerlemeyi takip edin** - Progress bar, speed, ETA
8. **İsteğe bağlı iptal** - Cancel butonu ile güvenli durdurma

## 📁 Dosya Yapısı

```
piwiper/
├── src/
│   ├── main_gui.cpp          # Ana GUI kodu
│   ├── piwiper.rc            # Resource dosyası
│   ├── piwiper.ico           # Uygulama ikonu
│   └── piwiper-gui.manifest  # Administrator manifest
├── build-gui.bat             # Derleme scripti
├── create_simple_icon.bat    # İkon oluşturma
└── README.md                 # Bu dosya
```

## 🔧 Teknik Detaylar

### Performans Optimizasyonları
- **Optimized disk scanning** - WMIC kullanımı
- **Efficient progress updates** - 1MB chunks ile smooth updates
- **Memory management** - Gereksiz değişkenler temizlendi
- **Code optimization** - 200+ satır kod azaltıldı

### Güvenlik Özellikleri
- **OS disk protection** - C: drive tespiti ile sistem koruması
- **Interactive confirmation** - Detaylı onay diyalogları
- **UAC elevation** - Administrator hakları zorunlu
- **Error handling** - Kapsamlı hata yönetimi

### Raporlama
- **Automatic reports** - Her silme işlemi için detaylı rapor
- **WipeLogs klasörü** - Otomatik rapor saklama
- **Serial number tracking** - Disk takibi
- **Timestamp logging** - Zaman damgalı kayıtlar

## 📊 Performans Metrikleri

| Özellik | Değer |
|---------|-------|
| **Dosya boyutu** | ~360KB |
| **Memory kullanımı** | %30+ azalma |
| **Kod satırı** | 200+ satır azalma |
| **Derleme süresi** | %40+ hızlanma |
| **Quick wipe** | ~3 saniye |
| **Secure wipe** | Disk boyutuna göre |

## 🛡️ Güvenlik Notları

### SSD/NVMe Diskler
- **Üretici araçları önerilir** - Samsung Magician, Crucial Storage Executive
- **Secure Erase/Sanitize** - Fabrika seviyesinde silme
- **Zero-fill limitation** - Remap edilmiş blokları garanti etmez

### Risk Azaltma
- **Disk bilgilerini iki kez kontrol edin**
- **Diğer diskleri fiziksel olarak ayırın**
- **Sadece harici diskleri hedefleyin**
- **Backup alın** (mümkünse)

## 🐛 Sorun Giderme

### Derleme Sorunları
```cmd
# Visual Studio environment yükleyin
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```

### Çalıştırma Sorunları
- **Administrator olarak çalıştırın**
- **Antivirus yazılımını geçici olarak devre dışı bırakın**
- **UAC ayarlarını kontrol edin**

### Disk Erişim Sorunları
- **Disk kullanımda değil** - Diğer uygulamaları kapatın
- **Write protection** - Disk korumasını kaldırın
- **USB bağlantısı** - Fiziksel bağlantıyı kontrol edin

## 📝 Sürüm Geçmişi

### v2.0 (Current)
- ✅ Modern UI/UX tasarımı
- ✅ Dual progress bars
- ✅ Real-time speed display
- ✅ Verification system
- ✅ Performance optimizations
- ✅ Code cleanup (200+ lines removed)
- ✅ Memory optimization

### v1.0
- ✅ Basic GUI interface
- ✅ Disk listing
- ✅ Quick/Secure wipe modes
- ✅ Progress tracking

## 📄 Lisans

Bu proje eğitim amaçlı geliştirilmiştir. Kullanım tamamen kendi sorumluluğunuzdadır.

## 🤝 Katkıda Bulunma

1. Fork yapın
2. Feature branch oluşturun (`git checkout -b feature/amazing-feature`)
3. Commit yapın (`git commit -m 'Add amazing feature'`)
4. Push yapın (`git push origin feature/amazing-feature`)
5. Pull Request oluşturun

## 📞 Destek

Sorunlar için GitHub Issues kullanın veya geliştirici ile iletişime geçin.

---

**⚡ PIWIPER - Professional, Modern, Güvenli Disk Eraser**
