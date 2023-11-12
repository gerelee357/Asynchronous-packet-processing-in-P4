# Asynchronous Packet Processing in P4

## Introduction

In this repository, I introduce an optimization possibility in P4 by incorporating asynchronous packet processing, enabling the invocation and execution of external functions during packet processing in the data plane. This solution aims to enhance data plane packet processing performance and improve link utilization.

## Implementation

I demonstrated this method in practice by integrating an external DPDK (Data Plane Development Kit) compression function. The implementation was carried out on ELTE's DPDK-based virtual switch with the P4 pipeline. The developed solution provides a general approach in P4 that is adaptable to various external functions.

## Optimization Techniques

In addition to the basic asynchronous compression function, I proposed and implemented further optimization methods, including:

- **Compression Based on Payload:** Tailoring compression strategies to payload characteristics.
  
- **In-Network Caching:** Utilizing caching mechanisms within the network for improved performance.

- **Offload External Function:** Investigating possibilities to offload certain external functions for enhanced efficiency.

## Publication

This work has been presented at the 13th Joint Conference on Mathematics and Computer Science (MaCS 2020), organized by ELTE. The publication is currently under process [here](link-to-your-publication).

## How to Run


1. Installing T4P4S (https://github.com/P4ELTE/t4p4s)

Clone the T4P4S repository from GitHub and install the specified version using the following command:

  ```bash
  git clone https://github.com/P4ELTE/t4p4s.git
  cd t4p4s
  git checkout 716df8b
  ```

After cloning the repository and checking out the specific version, you can proceed with the installation according to the provided instructions in README.

2. Before executing the application, run the following commands in the shell:

```bash
export RTE_SDK=/usr/local/share/dpdk/
export RTE_TARGET=x86_64-native-linuxapp-gcc

sudo sh -c 'echo 64 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages'
```



3. Create a new directory at the following location: `/usr/local/share/dpdk/examples/`.
4. Copy the files `test_compressdev.c` and `Makefile` into this new folder.
5. Execute the test_compressdev.c file following the instructions outlined in the README within the t4p4s directory.





