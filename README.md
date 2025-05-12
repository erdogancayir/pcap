# 🧠 Multi-threaded Packet Sniffer in C (libpcap-based)

A high-performance, multi-threaded network packet sniffer written in C using the `libpcap` library. It supports real-time traffic capture from a network interface or offline analysis of `.pcap` files. The program extracts Ethernet, IP, TCP/UDP, and HTTP information, tracks TCP connections, and writes structured output to a text file.

---

## 📌 Features

- 🖥️ Live packet capture from any system interface (e.g., `eth0`, `wlan0`)
- 📁 Offline `.pcap` file analysis support
- 📦 Extracts:
  - Source & destination **MAC** addresses
  - Source & destination **IP** addresses
  - Source & destination **ports** (TCP/UDP)
- 🌐 Detects **HTTP GET** and **POST** requests
  - Extracts `Host:` and `User-Agent:` headers
- 🧵 **Multi-threaded architecture**
  - Producer thread: captures and parses packets
  - Consumer thread: writes processed data to file
- 📊 **TCP connection tracking**
  - Total packets IN/OUT
  - Connection duration in milliseconds
- 🧠 Designed for high-speed with efficient queue and hash map structures
- ✋ Graceful shutdown on `SIGINT` (Ctrl+C)

---

## 🛠️ Requirements

- GCC (or any POSIX-compliant C compiler)
- [`libpcap`](https://www.tcpdump.org/) development package

### 🐧 Install on Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential libpcap-dev

git clone https://github.com/yourusername/packet-sniffer.git
cd packet-sniffer

`make` or `make debug`
```

### 🚀 Usage
- You can run the sniffer on a live interface or analyze a .pcap file. An output file path is always required.

#### 🔴 Live Capture Example

``` bash
sudo ./packet_sniffer -i eth0 -o output.txt
```

#### 📂 Offline PCAP Analysis Example

``` bash
./packet_sniffer -f capture.pcap -o output.txt
```

📝 Output Format
- Each captured packet is written to the output file in this format:

``` bash
MAC: 00:1a:2b:3c:4d:5e -> aa:bb:cc:dd:ee:ff
IP : 192.168.1.10 -> 172.217.0.46
PORT: 54321 -> 443
HTTP HOST       : www.example.com
HTTP USER-AGENT : Mozilla/5.0 ...
---------------------------------------
```

- At the end of each TCP connection (when FIN or RST is seen), a summary like the following is printed:

``` bash
[CONNECTION CLOSED] 192.168.1.4:54321 <-> 172.217.0.46:443
Packets IN : 10
Packets OUT: 12
Duration   : 532ms
```

## 🔧 Technical Design

This project was carefully designed to handle packet capture and processing **without introducing latency or data loss**, especially under high throughput. Below are the key architectural choices and how they solve potential bottlenecks.

---

### 🔁 Ring Buffer (Bounded Queue)

A **ring buffer** (also known as a circular queue) was implemented to manage packet flow between threads. This data structure:

- Enables constant-time enqueue and dequeue operations
- Avoids memory fragmentation and dynamic reallocations
- Has a fixed memory footprint, ideal for real-time systems

```text
Producer (Sniffer Thread)  --->  [ Ring Buffer ]  --->  Consumer (Writer Thread)
```

Each captured packet is parsed and enqueued; the writer dequeues and flushes to disk.

### 🧵 Multi-threaded Pipeline (Producer–Consumer Model)

To prevent the packet parsing and file I/O from blocking each other:

- Producer Thread captures and parses packets via pcap_loop()
- Consumer Thread waits on the ring buffer and writes to disk

Benefits:

- Sniffing is non-blocking, even if disk I/O is slow
- Threads communicate via pthread_mutex and pthread_cond
- Graceful shutdown is achieved with a shared done flag and SIGINT handling

### 🧵 Efficient TCP Connection Tracking

Each unique TCP connection (source IP/port <-> destination IP/port) is tracked using a hash map for:

- Fast lookups
- Counting incoming/outgoing packets
- Measuring connection lifetime (start to FIN/RST)

Upon termination, the sniffer prints a per-connection summary including packet counts and duration in milliseconds.

### 🧵 Graceful Shutdown

The application handles SIGINT (Ctrl+C) safely by:

- Breaking the pcap_loop() using pcap_breakloop()
- Marking the queue as done
- Unblocking any waiting producer or consumer threads
- Joining all threads before exit

This ensures no memory leaks, lost packets, or hanging resources.

## 🧼 Bottleneck Prevention

Common network sniffer bottlenecks and how this project mitigates them:

```markdown
| ⚠️ Bottleneck              | 🛠️ Mitigation Strategy                                              |
|----------------------------|---------------------------------------------------------------------|
| Disk I/O delay             | Dedicated writer thread with internal queue                         |
| Packet overflow            | Bounded queue blocks producer only when full                        |
| Signal handling            | pcap_breakloop() and 'done' flag enable graceful shutdown           |
| Memory allocation          | Queue buffer is preallocated to avoid dynamic allocation delay      |
| Parsing overhead           | Parsing is done directly in the producer thread for CPU cache usage |
```

## 🛡️ Thread Safety

All shared data structures (packet_queue, tcp_tracker) are protected with:

- pthread_mutex for mutual exclusion
- pthread_cond to signal queue state (full/empty)

Volatile flags (sig_atomic_t) for inter-thread signaling

## 📊 Architecture Diagram

```mermaid
flowchart LR
    subgraph Input Source
        A1[Live Interface] --> B[Sniffer Thread]
        A2[PCAP File] --> B
    end

    subgraph Packet Capture & Parsing
        B[Sniffer Thread]
        B --> C[Extract Headers - MAC, IP, Ports]
        C --> D[Detect HTTP - GET/POST, Host, UA]
        D --> E[Track TCP Connections - Stats]
        E --> F[Enqueue to Ring Buffer]
    end

    subgraph Queue
        F[Bounded Ring Buffer] <--> G[Writer Thread]
    end

    subgraph Output
        G --> H[Write to File]
        E --> I[Print TCP Connection Summary]
    end

    subgraph Shutdown
        J[Ctrl+C SIGINT]
        J --> K[pcap_breakloop]
        K --> B
        J --> L[Mark Queue Done]
        L --> G
    end