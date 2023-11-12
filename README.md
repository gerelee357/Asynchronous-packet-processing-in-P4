# Asynchronous Packet Processing Optimization in P4

## Introduction

In this repository, I introduce an optimization possibility in P4 by incorporating asynchronous packet processing, enabling the invocation and execution of external functions during packet processing in the data plane. This innovation aims to enhance data plane packet processing performance and improve link utilization.

## Implementation

I demonstrated this method in practice by integrating an external DPDK (Data Plane Development Kit) compression function. The implementation was carried out on ELTE's DPDK-based virtual switch with the P4 pipeline. The developed solution provides a general approach in P4 that is adaptable to various external functions.

## Optimization Techniques

In addition to the basic asynchronous compression function, I proposed and implemented further optimization methods, including:

- **Compression Based on Payload:** Tailoring compression strategies to payload characteristics.
  
- **In-Network Caching:** Utilizing caching mechanisms within the network for improved performance.

- **Offload External Function:** Investigating possibilities to offload certain external functions for enhanced efficiency.

## Publication

This work has been presented at the 13th Joint Conference on Mathematics and Computer Science (MaCS 2020), organized by ELTE. The publication is currently under process [here](link-to-your-publication).

## How to Use

[Provide instructions on how others can use or contribute to your project]

