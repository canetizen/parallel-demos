# Genel Notlar

## Deadlock Oluşma Koşulları
Deadlock’un oluşabilmesi için aşağıdaki **4 koşulun** aynı anda sağlanması gerekir:

1. **Karşılıklı dışlama (Mutual Exclusion):** En az bir kaynak yalnızca bir iş parçacığı tarafından kullanılabilir.
2. **Kaynak bekleme (Hold and Wait):** Bir iş parçacığı, bazı kaynakları tutarken başka bir kaynağı bekler.
3. **Öncelikli bırakma yok (No Preemption):** Bir kaynak zorla geri alınamaz; sadece kullanan iş parçacığı tarafından bırakılır.
4. **Döngüsel bekleme (Circular Wait):** İş parçacıkları, birbirinden kaynak bekleyen döngüsel bir yapı oluşturur.

---

## **Amdahl Yasası (Amdahl’s Law)**

**Tanım:**

Amdahl Yasası, bir programın sadece belirli bir kısmının paralel hale getirilebildiğini varsayarak, teorik maksimum hızlanmayı ifade eder. Formülü:

$$
\text{Hızlanma (Speedup)} = \frac{1}{(1 - P) + \frac{P}{N}}
$$

Burada:

* $P$: Programın paralel hale getirilebilen kısmı (0 ile 1 arasında)
* $(1 - P)$: Programın sadece seri çalışabilen kısmı
* $N$: İşlemci sayısı

**Yorum:**

Ne kadar çok işlemci eklenirse eklensin, seri kalan kısım nedeniyle hızlanma sınırlıdır. Bu yasa, işlemci sayısı arttıkça **azalan verim** problemini vurgular.

---

## **Gustafson Yasası (Gustafson’s Law)**

**Tanım:**

Gustafson Yasası, işlemci sayısı arttıkça **problemin büyüklüğünün de arttığını** varsayarak, daha gerçekçi bir hızlanma modeli önerir. Formülü:

$$
\text{Hızlanma (Speedup)} = N - (1 - P) \times (N)
$$

Burada:

* $P$: Paralel hale getirilebilen kısmın oranı
* $N$: İşlemci sayısı

**Yorum:**

Bu yasa, daha büyük problemler çözüldükçe ve işlemci sayısı arttıkça hızlanmanın da artabileceğini söyler. Gerçek dünya uygulamaları için daha uygundur.

---

### **Amdahl ve Gustafson Yasaları Arasındaki Farklar**

| Özellik                          | Amdahl Yasası                    | Gustafson Yasası                      |
| -------------------------------- | -------------------------------- | ------------------------------------- |
| Paralelleştirilemeyen kısma odak | Evet                             | Hayır                                 |
| Problem boyutu                   | Sabit                            | Artabilir (ölçeklenebilir)            |
| Hızlanma sınırı                  | Üst sınır vardır                 | İşlemci sayısı ile orantılı artabilir |
| Pratik uygulama                  | Teorik analizlerde kullanışlıdır | Gerçek sistemlerde daha uygulanabilir |

---

## False Sharing
**False sharing**, paralel programlamada birden fazla iş parçacığının (thread) **farklı bellek adreslerine** erişmesine rağmen, bu adreslerin aynı **cache line** içinde bulunması nedeniyle oluşan ciddi performans problemidir.

Modern CPU'larda bellekten alınan veri L1, L2 ve L3 cache seviyelerinde tutulur:

* **L1 Cache**: Her çekirdeğe özel, en küçük ama en hızlı cache seviyesidir.
* **L2 Cache**: Genellikle her çekirdeğe özel ya da birkaç çekirdek arasında paylaşılan orta büyüklükte cache seviyesidir.
* **L3 Cache**: CPU içindeki tüm çekirdeklerin paylaştığı, daha büyük ama daha yavaş cache seviyesidir.

CPU'lar veriyi tipik olarak 64 byte'lık bloklar hâlinde (**cache line**) işler. Birden fazla çekirdek aynı cache line'daki veriye eşzamanlı yazmaya çalıştığında, CPU'lar cache tutarlılığını (**cache coherence**, örn. MESI protokolü) sağlamak için sürekli **cache invalidation** gerçekleştirir. Bu invalidation işlemleri özellikle L1 ve L2 cache seviyelerinde sıkça gerçekleştiği için ciddi performans kayıplarına sebep olur.

### Sequential Çalışmada Cache Kullanımı

* Yalnızca **tek bir thread** olduğunda, veriye tek çekirdek üzerinden L1 cache'te hızlı erişilir.
* Herhangi bir invalidation gerçekleşmez ve veri genellikle L1 veya L2 cache içerisinde kalır.

### Paralel Çalışmada Cache ve False Sharing

* Farklı thread'ler farklı çekirdeklerde çalıştığında, her biri kendi L1 cache'ine sahiptir.
* Eğer farklı çekirdeklerde çalışan thread'ler, aynı cache line içerisinde bulunan farklı değişkenlere yazma yaparsa, çekirdeklerin cache'lerinde sürekli invalidation işlemleri meydana gelir. Bu durum, çekirdeklerin cache'lerindeki verilerin sürekli geçersizleşmesine ve yeniden yüklenmesine neden olarak performansı ciddi biçimde düşürür.

### False Sharing'in Önlenmesi

False sharing'i önlemek için aşağıdaki yöntemler kullanılabilir:

* Bellek hizalaması: Her thread'in kullandığı veriyi farklı cache line'lara yerleştirmek.
* Padding (dolgu verisi ekleyerek): Yapıları veya dizileri cache line sınırına hizalayarak farklı değişkenlerin farklı cache line'larda olmasını sağlamak.
* Thread-local storage veya private değişkenler kullanarak thread'lerin birbirlerinin verilerine erişmesini önlemek.

---

### Örnek 1: False Sharing Olan Kod

```c
#include <stdio.h>
#include <omp.h>

#define NUM_THREADS 4
#define NUM_STEPS 1000000

double sum[NUM_THREADS]; // Tehlike burada!
double step;

int main() {
    int i, nthreads;
    double pi = 0.0;
    step = 1.0 / NUM_STEPS;

    omp_set_num_threads(NUM_THREADS);

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nthrds = omp_get_num_threads();
        double x;

        if (id == 0) nthreads = nthrds;

        for (i = id, sum[id] = 0.0; i < NUM_STEPS; i += nthrds) {
            x = (i + 0.5) * step;
            sum[id] += 4.0 / (1.0 + x * x);
        }
    }

    for (i = 0; i < nthreads; i++) {
        pi += sum[i] * step;
    }

    printf("Pi = %.15f\n", pi);
    return 0;
}
```

**Sorun:** `sum[]` dizisi bitişik bellekte tutulduğu için elemanlar aynı cache line'a denk gelebilir. Bu da farklı çekirdeklerdeki thread'lerin birbirlerinin cache'ini geçersiz kılmasına yol açar.

---

### Örnek 2: Padding ile False Sharing Engellenmiş Kod

```c
#include <stdio.h>
#include <omp.h>

#define NUM_THREADS 4
#define NUM_STEPS 1000000
#define CACHE_LINE_SIZE 64

typedef struct {
    double value;
    char padding[CACHE_LINE_SIZE - sizeof(double)];
} PaddedDouble;

PaddedDouble sum[NUM_THREADS];
double step;

int main() {
    int i, nthreads;
    double pi = 0.0;
    step = 1.0 / NUM_STEPS;

    omp_set_num_threads(NUM_THREADS);

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nthrds = omp_get_num_threads();
        double x;

        if (id == 0) nthreads = nthrds;

        for (i = id, sum[id].value = 0.0; i < NUM_STEPS; i += nthrds) {
            x = (i + 0.5) * step;
            sum[id].value += 4.0 / (1.0 + x * x);
        }
    }

    for (i = 0; i < nthreads; i++) {
        pi += sum[i].value * step;
    }

    printf("Pi = %.15f\n", pi);
    return 0;
}
```
**Çözüm:** `sum[i].value`'lar ayrı cache line’lara hizalanmıştır. Böylece her thread'in eriştiği veri, diğerlerinden bağımsız olur ve false sharing engellenir.

---
