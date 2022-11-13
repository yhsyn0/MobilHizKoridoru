# Bilgisayarlı Görü Yardımıyla Mobil Hız Koridoru Sistemi

<p align="center">
  <img width="415" height="410" src="https://user-images.githubusercontent.com/43849766/201532780-50a50f80-efed-4ec7-afdd-bf35439dab78.jpeg">
  <h4 align="center">
    <i>
      This project was developed as undergraduate final project within ISAR Lab.<br />
      https://github.com/ISAR-Lab
    </i> 
  </h4>
</p>

  Burada bulunan dosyalar TÜBİTAK 2209-A Üniversite Öğrencileri Araştırma Projeleri Destekleme Programı kapsamında 1919B012100901 nolu projenin yazılımsal kısmına aittir. Proje, https://github.com/DoubangoTelecom/ultimateALPR-SDK adresindeki Ultimate ALPR SDK'si üzerine geliştirme yapılarak oluşturulmuştur.
  
  mainRecognizer programı aracı, plakayı ve plaka metnini tespit etmekte ve tespit ettikten sonra ana programı kesintiye uğratmamak adına veritabanına gönderelecek bilgiler passTerminal programına gönderilmektedir. 
  passTerminal, writeTerminal ve sql.py programları ana tanıma programının kesintiye uğramaması adına Fire&Forget tarzı bir yaklaşım benimseyerek, veritabanına iletilen bilginin başarılı/başarısız geri dönüşünü beklemeden sadece iletim yapan bir programlar zinciridir.
  sql.py programında bulunan veri tabanı bilgileri güvenlik amacıyla silinmiştir.
  
  Hız koridoru sisteminde, giriş ve çıkış olmak üzere tek bir fark hariç birbirinin aynısı iki sistem (Nvidia Jetson Nano w/ IMX219-77 Camera and WiFi-Bluetooth) bulunmaktadır. Çıkış sisteminin giriş sisteminden tek farkı, ekstra olarak aradaki mesafe bilgisini de veritabanına göndermektedir. 
  
  Programın detaylı kullanımı ultimateALPR-SDK sayfasında açıklanmıştır.
