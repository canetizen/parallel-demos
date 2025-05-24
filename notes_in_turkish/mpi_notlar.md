# MPI Temel Fonksiyonlar

## 1. `MPI_Init`

### Açıklama:

MPI ortamını başlatır. Tüm MPI programlarının başında çağrılır.

### İmza:

```c
int MPI_Init(int *argc, char ***argv);
```

### Parametreler:

* `argc`: Komut satırı argüman sayısına işaretçi.
* `argv`: Komut satırı argüman dizisine işaretçi.

### Örnek:

```c
MPI_Init(&argc, &argv);
```

### Çıktı:

```
(MPI başlatıldı.)
```

---

## 2. `MPI_Finalize`

### Açıklama:

MPI ortamını sonlandırır. Programın sonunda çağrılır.

### İmza:

```c
int MPI_Finalize(void);
```

### Parametreler:

* Yok.

### Örnek:

```c
MPI_Finalize();
```

### Çıktı:

```
(MPI sonlandırıldı.)
```

---

## 3. `MPI_Comm_rank`

### Açıklama:

İşlemcinin sıra numarasını (rank) verir.

### İmza:

```c
int MPI_Comm_rank(MPI_Comm comm, int *rank);
```

### Parametreler:

* `comm`: İletişimci (örneğin `MPI_COMM_WORLD`).
* `rank`: İşlemcinin sıra numarasını döndürecek değişken.

### Örnek:

```c
int rank;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
printf("My rank is %d\n", rank);
```

### Çıktı:

```
My rank is 0
My rank is 1
My rank is 2
My rank is 3
```

---

## 4. `MPI_Comm_size`

### Açıklama:

İletişimcideki toplam işlemci sayısını verir.

### İmza:

```c
int MPI_Comm_size(MPI_Comm comm, int *size);
```

### Parametreler:

* `comm`: İletişimci.
* `size`: Toplam işlemci sayısını döndürecek değişken.

### Örnek:

```c
int size;
MPI_Comm_size(MPI_COMM_WORLD, &size);
printf("There are %d processes.\n", size);
```

### Çıktı:

```
There are 4 processes.
```

---

## 5. `MPI_Send`

### Açıklama:

Bir işlemciye veri gönderir (bloklayıcı).

### İmza:

```c
int MPI_Send(const void *buf, int count, MPI_Datatype datatype,
             int dest, int tag, MPI_Comm comm);
```

### Parametreler:

* `buf`: Gönderilecek veri.
* `count`: Eleman sayısı.
* `datatype`: MPI veri tipi (örneğin `MPI_INT`).
* `dest`: Hedef işlemci (rank).
* `tag`: Mesaj etiketi.
* `comm`: İletişimci.

### Örnek:

```c
if (rank == 0) {
    int data = 42;
    MPI_Send(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
}
```

### Çıktı:

(Veri gönderildi, alıcı işlemde `MPI_Recv` ile alınmalı.)

---

## 6. `MPI_Recv`

### Açıklama:

Bir işlemciden veri alır (bloklayıcı).

### İmza:

```c
int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
             int source, int tag, MPI_Comm comm, MPI_Status *status);
```

### Parametreler:

* `buf`: Alınan verinin yazılacağı tampon.
* `count`: Maksimum eleman sayısı.
* `datatype`: Veri tipi.
* `source`: Gönderen işlemci (rank).
* `tag`: Mesaj etiketi.
* `comm`: İletişimci.
* `status`: Alım durumu bilgisi.

### Örnek:

```c
if (rank == 1) {
    int data;
    MPI_Recv(&data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Rank 1 received %d from Rank 0\n", data);
}
```

### Çıktı:

```
Rank 1 received 42 from Rank 0
```

---

## 7. `MPI_Bcast`

### Açıklama:

Bir işlem veriyi tüm işlemcilere yayınlar.

### İmza:

```c
int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype,
              int root, MPI_Comm comm);
```

### Parametreler:

* `buffer`: Yayınlanacak/alınacak veri.
* `count`: Eleman sayısı.
* `datatype`: Veri tipi.
* `root`: Yayın yapan işlem.
* `comm`: İletişimci.

### Örnek:

```c
int x;
if (rank == 0) x = 77;
MPI_Bcast(&x, 1, MPI_INT, 0, MPI_COMM_WORLD);
printf("Rank %d received %d\n", rank, x);
```

### Çıktı:

```
Rank 0 received 77
Rank 1 received 77
Rank 2 received 77
Rank 3 received 77
```

---

## 8. `MPI_Gather`

### Açıklama:

Her işlem verisini gönderir, kök işlem hepsini toplar.

### İmza:

```c
int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
               void *recvbuf, int recvcount, MPI_Datatype recvtype,
               int root, MPI_Comm comm);
```

### Parametreler:

* `sendbuf`: Gönderilecek veri.
* `sendcount`: Gönderilecek eleman sayısı.
* `sendtype`: Veri tipi.
* `recvbuf`: Alınan verilerin toplanacağı tampon (sadece root için geçerli).
* `recvcount`: Kişi başı alınacak eleman.
* `recvtype`: Veri tipi.
* `root`: Verileri toplayan işlem.
* `comm`: İletişimci.

### Örnek:

```c
int x = rank;
int gathered[4];
MPI_Gather(&x, 1, MPI_INT, gathered, 1, MPI_INT, 0, MPI_COMM_WORLD);
if (rank == 0) {
    printf("Gathered: %d %d %d %d\n", gathered[0], gathered[1], gathered[2], gathered[3]);
}
```

### Çıktı:

```
Gathered: 0 1 2 3
```

---

## 9. `MPI_Allgather`

### Açıklama:

Her işlem veri gönderir, tüm işlemler hepsini alır.

### İmza:

```c
int MPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                  void *recvbuf, int recvcount, MPI_Datatype recvtype,
                  MPI_Comm comm);
```

### Parametreler:

* `sendbuf`: Gönderilecek veri.
* `sendcount`: Eleman sayısı.
* `sendtype`: Veri tipi.
* `recvbuf`: Tüm verilerin alınacağı tampon.
* `recvcount`: Kişi başı alınan eleman sayısı.
* `recvtype`: Veri tipi.
* `comm`: İletişimci.

### Örnek:

```c
int val = rank;
int all_vals[4];
MPI_Allgather(&val, 1, MPI_INT, all_vals, 1, MPI_INT, MPI_COMM_WORLD);
printf("Rank %d sees: %d %d %d %d\n", rank, all_vals[0], all_vals[1], all_vals[2], all_vals[3]);
```

### Çıktı:

```
Rank 0 sees: 0 1 2 3
Rank 1 sees: 0 1 2 3
Rank 2 sees: 0 1 2 3
Rank 3 sees: 0 1 2 3
```

---

## 10. `MPI_Reduce`

### Açıklama:

Her işlem verisini gönderir, kök işlem belirtilen bir işlemle (örn. toplam) sonucu alır.

### İmza:

```c
int MPI_Reduce(const void *sendbuf, void *recvbuf, int count,
               MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
```

### Parametreler:

* `sendbuf`: Gönderilecek veri.
* `recvbuf`: Sonucun yazılacağı tampon (sadece root işlemde kullanılır).
* `count`: Eleman sayısı.
* `datatype`: Veri tipi (örn. `MPI_INT`).
* `op`: Uygulanacak işlem (örn. `MPI_SUM`, `MPI_MAX`).
* `root`: Sonucu alacak işlem (rank).
* `comm`: İletişimci.

### Örnek:

```c
int x = rank;
int sum;
MPI_Reduce(&x, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
if (rank == 0) {
    printf("Sum of all ranks = %d\n", sum);
}
```

### Çıktı:

```
Sum of all ranks = 6
```

---

## 11. `MPI_Allreduce`

### Açıklama:

Tüm işlemcilerde veriler işlenir (ör. toplanır) ve sonuç **her işlemciye** gönderilir.

### İmza:

```c
int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
```

### Parametreler:

* `sendbuf`: Gönderilecek veri.
* `recvbuf`: Sonucun yazılacağı tampon.
* `count`: Eleman sayısı.
* `datatype`: Veri tipi.
* `op`: Uygulanacak işlem.
* `comm`: İletişimci.

### Örnek:

```c
int x = rank;
int total;
MPI_Allreduce(&x, &total, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
printf("Rank %d sees total = %d\n", rank, total);
```

### Çıktı:

```
Rank 0 sees total = 6
Rank 1 sees total = 6
Rank 2 sees total = 6
Rank 3 sees total = 6
```

---

## 12. `MPI_Scatter`

### Açıklama:

Kök işlemdeki veriler parçalara bölünür ve her işlem bir parça alır.

### İmza:

```c
int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, int recvcount, MPI_Datatype recvtype,
                int root, MPI_Comm comm);
```

### Parametreler:

* `sendbuf`: Gönderilecek verinin bulunduğu dizi (sadece root için).
* `sendcount`: Her işlem için gönderilecek eleman sayısı.
* `sendtype`: Gönderilen veri tipi.
* `recvbuf`: Alınan verinin yazılacağı tampon.
* `recvcount`: Alınacak eleman sayısı.
* `recvtype`: Alınan veri tipi.
* `root`: Veriyi gönderen işlem.
* `comm`: İletişimci.

### Örnek:

```c
int data[4] = {10, 20, 30, 40};
int part;
MPI_Scatter(data, 1, MPI_INT, &part, 1, MPI_INT, 0, MPI_COMM_WORLD);
printf("Rank %d received %d\n", rank, part);
```

### Çıktı:

```
Rank 0 received 10
Rank 1 received 20
Rank 2 received 30
Rank 3 received 40
```

---

## 13. `MPI_Alltoall`

### Açıklama:

Her işlem diğer tüm işlemlere farklı veri gönderir. Herkes herkese.

### İmza:

```c
int MPI_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 MPI_Comm comm);
```

### Parametreler:

* `sendbuf`: Tüm işlemlere gönderilecek verilerin bulunduğu tampon.
* `sendcount`: Her işlemciye gönderilecek eleman sayısı.
* `sendtype`: Gönderilen veri tipi.
* `recvbuf`: Alınan verilerin yazılacağı tampon.
* `recvcount`: Her işlemciden alınacak eleman sayısı.
* `recvtype`: Alınan veri tipi.
* `comm`: İletişimci.

### Örnek:

```c
int sendbuf[4] = {rank*10 + 0, rank*10 + 1, rank*10 + 2, rank*10 + 3};
int recvbuf[4];
MPI_Alltoall(sendbuf, 1, MPI_INT, recvbuf, 1, MPI_INT, MPI_COMM_WORLD);
printf("Rank %d received: %d %d %d %d\n", rank, recvbuf[0], recvbuf[1], recvbuf[2], recvbuf[3]);
```

### Çıktı:

```
Rank 0 received: 0 10 20 30
Rank 1 received: 1 11 21 31
Rank 2 received: 2 12 22 32
Rank 3 received: 3 13 23 33
```
---

## 14. `MPI_Barrier`

### Açıklama:

`MPI_Barrier`, bir senkronizasyon fonksiyonudur. Tüm işlemciler bu bariyere ulaşmadan hiçbiri devam edemez. Yani her işlem bu fonksiyonu çağırana kadar, çağıran işlemler bekletilir. Bu, özellikle işlemler arasında **zamanlamayı hizalamak** veya bir bloğun tamamlanmasını garantilemek için kullanılır.

---

### İmza:

```c
int MPI_Barrier(MPI_Comm comm);
```

---

### Parametreler:

* `comm`: Bariyerin uygulanacağı iletişimci (genellikle `MPI_COMM_WORLD` kullanılır).

---

### Örnek:

Aşağıdaki örnekte, işlem 0 bir mesaj bastıktan sonra `MPI_Barrier` ile diğer işlemlerin aynı noktaya gelmesini bekliyor. Tüm işlemler senkronize olduktan sonra diğer mesajlar yazdırılıyor.

```c
int rank;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

if (rank == 0) {
    printf("Rank 0 is ready. Waiting for others...\n");
}

MPI_Barrier(MPI_COMM_WORLD);  // Senkronizasyon noktası

printf("Rank %d passed the barrier.\n", rank);
```

---

### Çıktı:

```
Rank 0 is ready. Waiting for others...
Rank 3 passed the barrier.
Rank 2 passed the barrier.
Rank 0 passed the barrier.
Rank 1 passed the barrier.
```
---

## Ek Açıklamalar
### Tag
MPI'de tag, gönderilen mesajlara verilen bir etikettir ve alıcı işlemin mesajları ayırt etmesini sağlar. Aynı kaynak işlemciden farklı içerikte mesajlar gönderildiğinde alıcı, tag değerine bakarak hangi mesajı alacağını belirleyebilir. Bu, özellikle sıralı mesajlar veya farklı görevler için mesajlar gönderildiğinde önemlidir.

```C
int data;
if (rank == 0) {
    int a = 100, b = 200;
    MPI_Send(&a, 1, MPI_INT, 1, 10, MPI_COMM_WORLD); // tag 10
    MPI_Send(&b, 1, MPI_INT, 1, 20, MPI_COMM_WORLD); // tag 20
}
else if (rank == 1) {
    MPI_Recv(&data, 1, MPI_INT, 0, 20, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Received with tag 20: %d\n", data);
    MPI_Recv(&data, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Received with tag 10: %d\n", data);
}

/*
Output: 
Received with tag 20: 200
Received with tag 10: 100

Tag sayesinde inputun alınma sırası alıcı tarafından belirlenmiş oldu.
*/
```

### MPI Status
MPI_Status, bir mesaj alındığında mesajla ilgili bilgileri (kimden geldiği, hangi etiketle geldiği, hata kodu gibi) tutan bir yapıdır ve özellikle MPI_Recv gibi fonksiyonlarla birlikte kullanılır; bu yapı sayesinde alıcı işlem, gelen mesajın kaynak işlemcisini (MPI_SOURCE), mesajın etiketini (MPI_TAG) ve mesajın kaç veri içerdiğini (ek olarak MPI_Get_count ile) öğrenebilir.

```C
int data;
MPI_Status status;
MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
printf("Received %d from rank %d with tag %d\n", data, status.MPI_SOURCE, status.MPI_TAG);

/*
Output:
Received 42 from rank 0 with tag 99
*/
```