# STM32F103 Klima Kontrol Paneli — OLED Arayüzü

**STM32F103C8T6** ("Blue Pill") üzerinde, **0.96" SSD1306 OLED** ekranı I²C üzerinden süren, bare-metal bir klima kontrol arayüzü. HAL, LL veya hiçbir grafik kütüphanesi kullanılmadan, baştan sona register seviyesinde yazıldı.

> Kişisel proje — **[Ersanny](https://github.com/Ersanny)** tarafından, düşük seviyeli gömülü geliştirme pratiği olarak sıfırdan yazılmıştır.

---

## Proje ne yapıyor

OLED ekran üzerinde gerçek zamanlı bir klima kontrol paneli görüntülenir:

- **Ortada büyük rakam** — hedef sıcaklık (set değeri)
- **Sol alt** — fan hızı yüzdesi
- **Sağ alt** — mevcut oda sıcaklığı
- **Sol üst ikon** — set ve oda sıcaklığı arasındaki farka göre **soğutma / ısıtma / bekleme** modları arasında otomatik geçiş yapar
- **Açılış animasyonu** — GitHub logosu soldan, "Ersanny" yazısı sağdan kayarak gelir, 1.5 saniye durur, ters yönde kayarak çıkar → ana arayüz başlar

Şu anda sıcaklık ve fan değerleri kodda sabit; gerçek sensör ve buton girdisi eklenmek üzere altyapı hazır.

---

## Donanım

| Bileşen                  | Detay                                              |
|--------------------------|-----------------------------------------------------|
| MCU                      | STM32F103C8T6 (Cortex-M3, 64 KB Flash, 20 KB RAM)   |
| Ekran                    | SSD1306 0.96" OLED, 128 × 64, I²C                   |
| Saat                     | 8 MHz HSI (dış kristal yok)                         |
| I²C hatları              | PB6 (SCL), PB7 (SDA), 100 kHz                       |
| Programlayıcı            | ST-Link V2 (Keil μVision)                           |

---

## Mimari

Kod üç katmana bölünmüştür; her katman ince bir API ile ayrılmıştır.

```
┌─────────────────────────────────────────────────┐
│ main.c          ── uygulama döngüsü             │
│                    SSD1306_DrawUI(set,room,fan) │
├─────────────────────────────────────────────────┤
│ ssd1306.c/.h    ── ekran sürücüsü + UI render   │
│                    frame buffer, fontlar, katman│
├─────────────────────────────────────────────────┤
│ i2c.c/.h        ── register seviyesinde I²C,    │
│                    timeout korumalı primitifler │
└─────────────────────────────────────────────────┘
```

### Frame buffer mimarisi

Sürücü ekranın **1024 baytlık (8 × 128) RAM kopyasını** tutar. Tüm çizim fonksiyonları yalnızca RAM'e yazar; `SSD1306_Flush()` ardından tüm buffer'ı **8 I²C transaction'ında** (sayfa başına 1) gönderir.

Bu neden önemli: her byte için ayrı Start/Stop kullanan saf bir yaklaşım, frame başına ~1024 transaction gerektirirdi. Buffer tabanlı tasarım yaklaşık **128× daha hızlı** olduğu gibi, birden fazla katmanı (arka plan → ikon → rakamlar → küçük yazılar) titremesiz çizmeye olanak verir.

### Katmanlı UI çizimi

`SSD1306_DrawUI()` ekranı dört sıralı katman halinde inşa eder:

1. **Arka plan bitmap'i** — `ui_sogutma / ui_isitma / ui_bekleme` arasından otomatik moda göre seçilir
2. **Büyük rakamlar** — set sıcaklığı, ortalanmış
3. **Sağ alt sayı** — mevcut oda sıcaklığı
4. **Sol alt sayı** — fan yüzdesi

Katmanlar sırayla buffer'a çizilir, ardından tek bir flush ile OLED'e gönderilir.

### Hata yayılım zinciri

Her I²C primitifi `I2C_Status` döndürür (OK veya TIMEOUT). Durum çağrı zinciri boyunca yukarı doğru taşınır:

```
I2C_WriteByte → SSD1306_Flush → SSD1306_DrawUI → main
```

Bus takılırsa (örneğin OLED bağlantısı koparsa), her `while (!flag)` döngüsü ~10 ms sonra çıkar ve hata `main()` seviyesine kadar yükselir; orada gerekirse kurtarma aksiyonu alınır. MCU asla cevapsız bir donanımı beklerken kilitlenmez.

### Otomatik mod tespiti

Uygulama sürücüye mod parametresi göndermez — `DrawUI` modu girdilerden türetir:

```c
if      (room_temp > set_temp) mod = SOGUTMA;
else if (room_temp < set_temp) mod = ISITMA;
else                            mod = BEKLEME;
```

Tek doğru kaynak, caller-driver uyumsuzluğu riski yok.

---

## Kilit fonksiyonlar

| Fonksiyon                 | Görevi                                                |
|---------------------------|-------------------------------------------------------|
| `I2C_Init()`              | GPIOB pinlerini ayarlar, I²C1'i 100 kHz standart moda alır |
| `I2C_WriteByte()`         | TXE/BTF timeout korumalı tek byte gönderir            |
| `SSD1306_Init()`          | 100 ms güç-açma gecikmesi + 28 komutluk init dizisi   |
| `SSD1306_Flush()`         | Tüm frame buffer'ı 8 sayfa transaction'ı ile gönderir |
| `SSD1306_DrawBigDigit()`  | 18 × 32 px büyük rakamı (x, page) konumuna çizer      |
| `SSD1306_WriteChar()`     | 10 × 12 px rakamı iki sayfaya yayılarak çizer         |
| `SSD1306_DrawUI()`        | Dört katmanlı klima arayüzünü oluşturur ve gönderir   |
| `SSD1306_Splash()`        | Açılış animasyonunu yürütür (slide-in / dur / slide-out) |

---

## Önemli tasarım kararları

**`Init`'te 100 ms güç-açma gecikmesi.** Bu gecikme olmadan SSD1306, güç verildikten sonra ekrana bazen 180° dönmüş yazıyordu — çünkü oryantasyon komutları geldiğinde charge pump henüz hazır değildi. `SSD1306_Init`'in en başına eklenen 100 ms busy-wait sorunu kalıcı olarak çözdü.

**İki font, ikisi de Adafruit GFX'ten dönüştürülmüş.** Açık kaynak font üreticileri GFX formatında veri çıkarır (satır-satır, MSB-packed). SSD1306 ise sütun-sütun, page-packed veri ister. Küçük bir Python betiği iki formatı transpoze eder.

**Fontlar sadece rakam.** Hem `fontOS40` (büyük) hem `fontOS16` (küçük) sadece `0`–`9` içerir. UI'da çalışma sırasında harfe ihtiyaç yok (ekrandaki yazılar önceden render edilmiş bitmap'ler), bu yüzden harfleri çıkararak Flash tasarrufu yapıldı.

**Splash bileşenleri `static`.** `github_logo`, `ersanny_text`, `draw_logo`, `draw_ersanny` ve `splash_delay_ms` hepsi dosya-yereldir — başka modüllere sızamazlar.

**Açılışta "Reset and Run".** Keil flash download ayarlarında *Reset and Run* aktif; çip yüklemeden sonra yeni kodu otomatik başlatır. Bu olmadan MCU manuel reset'e kadar tanımsız bir durumda kalıyordu.

---

## Kaynak kullanımı

| Kaynak  | Kullanım | Toplam | Doluluk |
|---------|----------|--------|---------|
| Flash   | ~5.5 KB  | 64 KB  | ~%8.6   |
| RAM     | ~1 KB    | 20 KB  | ~%5     |

Sensörler, butonlar, RTOS, kalıcı bellek veya başka her şey için bol miktarda kapasite mevcut.

---

## Kullanılan araçlar

- **[Keil μVision](https://www.keil.com/)** — IDE, derleyici, ST-Link flash & debug
- **[oleddisplay.squix.ch](https://oleddisplay.squix.ch/)** — Font üretimi (Adafruit GFX formatında)
- **[image2cpp](https://javl.github.io/image2cpp/)** — UI ikonları ve GitHub logosu için bitmap-to-C-array dönüşümü
- **[piskelapp](https://www.piskelapp.com/p/create/sprite/)** — Pixel-art logo tasarımı
- **Python** — GFX → SSD1306 sayfa formatı font dönüşümü için yazılan betikler
- **STM32F10x reference manual & SSD1306 datasheet** — register ve komut seviyesinde dokümantasyon

---

## Dosya yapısı

```
.
├── main.c          # Giriş noktası, uygulama döngüsü
├── i2c.h / i2c.c   # Register seviyesinde I²C sürücüsü
├── ssd1306.h       # Genel API
└── ssd1306.c       # Sürücü: frame buffer, fontlar, UI çizimi, splash
```

## Lisans

Kişisel / eğitim amaçlı proje. Özgürce kullanın, fork'layın, öğrenin.

— **Ersanny**
