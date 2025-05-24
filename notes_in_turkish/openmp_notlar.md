# OpenMP Temel Direktifler

## 1. `#pragma omp parallel`

Bu yönerge, belirtilen kod bloğunu paralel olarak çalıştırır. Her iş parçacığı bu bloğu kendi başına çalıştırır.

**Örnek:**

```c
#include <stdio.h>
#include <omp.h>

int main() {
    #pragma omp parallel
    {
        printf("Hello from thread %d\n", omp_get_thread_num());
    }
    return 0;
}
```

**Olası çıktı:**

```
Hello from thread 0
Hello from thread 1
Hello from thread 2
Hello from thread 3
```

(Sıra değişebilir, çünkü iş parçacıkları paralel çalışır.)

---

## 2. `omp_set_num_threads(int n)`, 3. `omp_get_thread_num()`

Kullanılacak/kullanılan iş parçacığı sayısını belirler/döndürür. omp_set_num_threads, `#pragma omp parallel`'den önce çağrılmalıdır.

**Örnek:**

```c
#include <stdio.h>
#include <omp.h>

int main() {
    omp_set_num_threads(4);
    #pragma omp parallel
    {
        printf("Thread %d is running\n", omp_get_thread_num());
    }
    return 0;
}
```

---

## 4. `#pragma omp critical`

Aynı anda sadece bir iş parçacığının çalıştırmasını istediğiniz kod bloğu için kullanılır. Genellikle yazma işlemlerinde kullanılır.

**Örnek:**

```c
#include <stdio.h>
#include <omp.h>

int main() {
    int x = 0;

    #pragma omp parallel
    {
        #pragma omp critical
        {
            x++;
        }
    }

    printf("x = %d\n", x);
    return 0;
}
```

**Çıktı:**

```
x = <iş parçacığı sayısı>
```

---

## 5. `#pragma omp atomic`

`critical`'a benzer şekilde, paylaşılan bir değişkene atomik (kesintisiz) bir işlem uygulanmasını sağlar ama daha verimlidir.

**Örnek:**

```c
#include <stdio.h>
#include <omp.h>

int main() {
    int x = 0;

    #pragma omp parallel
    {
        #pragma omp atomic
        x++;
    }

    printf("x = %d\n", x);
    return 0;
}
```

---

## 6. `#pragma omp for`

Bir döngüyü iş parçacıkları arasında bölmek için kullanılır. Genelde `#pragma omp parallel` içinde olur.

**Örnek:**

```c
#include <stdio.h>
#include <omp.h>

int main() {
    omp_set_num_threads(4);
    int i;

    #pragma omp parallel
    {
        #pragma omp for
        for (i = 0; i < 8; i++) {
            printf("i = %d, thread = %d\n", i, omp_get_thread_num());
        }
    }
    return 0;
}
```

---

## 7. `#pragma omp for collapse(n)`

İç içe döngüleri tek bir döngü gibi ele alır ve paralelleştirir. `n` iç içe döngü sayısıdır.

**Örnek:**

```c
#include <stdio.h>
#include <omp.h>

int main() {
    int i, j;

    #pragma omp parallel for collapse(2)
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            printf("i = %d, j = %d, thread = %d\n", i, j, omp_get_thread_num());
        }
    }
    return 0;
}
```

---

## 8. `#pragma omp reduction(operation:variable)`

Paylaşılan bir değişkenin, her iş parçacığında kendi lokal kopyasında işlem yapmasını ve sonunda bunları birleştirmesini sağlar.

**Örnek:**

```c
#include <stdio.h>
#include <omp.h>

int main() {
    int sum = 0;

    #pragma omp parallel for reduction(+:sum)
    for (int i = 1; i <= 100; i++) {
        sum += i;
    }

    printf("Sum = %d\n", sum);
    return 0;
}
```

**Çıktı:**

```
Sum = 5050
```

---

## 9. `#pragma omp barrier`
OpenMP'de #pragma omp barrier, iş parçacıkları arasında senkronizasyon sağlamak için kullanılır. Bu yönergeye ulaştıklarında, tüm iş parçacıkları orada bekler, ta ki hepsi aynı noktaya ulaşana kadar. Böylece bir iş parçacığının diğerinden önce ilerleyip hatalı işlem yapması engellenir.

**Örnek:**
```C
#include <stdio.h>
#include <omp.h>

int main() {
    int a = 0, b = 0;

    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();

        // Sadece thread 0 a'yı değiştirir
        if (tid == 0) {
            a = 42;
            printf("Thread %d: a is set to %d\n", tid, a);
        }

        // Diğer thread'ler burada bekler, a'nın güncellenmesini garanti eder
        #pragma omp barrier

        // Tüm thread'ler senkronize bir şekilde buradan devam eder
        b = a;

        printf("Thread %d: b = %d\n", tid, b);
    }

    return 0;
}

```

**Çıktı:**

```
Thread 0: a is set to 42
Thread 1: b = 42
Thread 3: b = 42
Thread 2: b = 42
Thread 0: b = 42
```

---


## Ek Notlar

### critical vs atomic
#pragma omp atomic, paylaşılan tek bir değişken üzerinde basit ve kısa işlemler (örneğin x++, x += y) yapıldığında #pragma omp critical'a göre daha verimlidir çünkü daha hafif bir kilitleme mekanizması kullanır ve donanım seviyesinde optimize edilir; buna karşın critical, daha karmaşık işlemleri veya birden fazla değişkenin güncellenmesini destekler ama tüm iş parçacıklarını sıraya sokarak performansı düşürebilir, bu nedenle mümkün olan durumlarda atomic tercih edilmelidir.

### False Sharing
**False sharing**, paralel programlamada farklı iş parçacıklarının kendi bağımsız değişkenlerine eriştiği halde, bu değişkenlerin bellekte aynı **cache line** içinde yer alması nedeniyle performans kaybı yaşanmasıdır. CPU’lar verileri cache line denilen 64 byte’lık bloklar hâlinde önbelleğe alır. Eğer iki iş parçacığı, aynı cache line'da yer alan farklı bellek konumlarını güncelliyorsa, bu işlemciler birbirlerinin cache’ini geçersiz kılar (cache invalidation), sürekli senkronizasyon oluşur ve performans ciddi şekilde düşer.

Bu sorunu **verileri farklı cache line'lara yayarak (padding kullanarak)** çözebiliriz.

---

#### Örnek 1: False Sharing Olan Kod

Bu kodda `sum[]` dizisinin elemanları bellekte bitişik tutulduğu için false sharing oluşabilir.

```c
#include <stdio.h>
#include <omp.h>

#define NUM_THREADS 4
#define NUM_STEPS 1000000

double sum[NUM_THREADS];
double step;

int main() {
    int i, nthreads;
    double pi = 0.0;
    step = 1.0 / (double)NUM_STEPS;

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

**Problemin kaynağı:**
Tüm `sum[id]` değerleri bellekte bitişik dizildiğinden, farklı iş parçacıkları aynı cache line’ı paylaşıyor olabilir. Bu da false sharing’e neden olur.

---

#### Örnek 2: False Sharing Çözülmüş Kod (Padding Kullanılarak)

Bu örnekte her iş parçacığının kullandığı veri bellekte ayrı cache line’a düşecek şekilde padding uygulanarak false sharing engellenmiştir.

```c
#include <stdio.h>
#include <omp.h>

#define NUM_THREADS 4
#define NUM_STEPS 1000000
#define CACHE_LINE_SIZE 64

typedef struct {
    double value;
    char padding[CACHE_LINE_SIZE - sizeof(double)]; // padding ile hizalama
} PaddedDouble;

PaddedDouble sum[NUM_THREADS];
double step;

int main() {
    int i, nthreads;
    double pi = 0.0;
    step = 1.0 / (double)NUM_STEPS;

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

**Çözüm:**
Her `sum[i].value` değeri bellekte farklı bir cache line’a denk gelir, böylece iş parçacıkları birbirlerinin önbelleğine müdahale etmez. Performans kaybı önlenmiş olur.

---
#### critical ve atomic İle Çözüm
Bazı durumlarda, özellikle sık sık yazılan ortak bir veri varsa, critical kullanılarak da çözülebilir fakat bu yöntem performansı düşürür, çünkü iş parçacıkları sırayla çalışmak zorunda kalır.