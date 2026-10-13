# Transport Layer Optimization for Real-Time Healthcare Telemetry in Congested Wireless Networks

## Project Overview

This project evaluates the performance of TCP and UDP for real-time healthcare telemetry in a congested wireless network using NS-3.

The simulation models healthcare traffic and competing background traffic to investigate how congestion affects network performance.

## Objectives

- Compare TCP and UDP under different background traffic loads.
- Measure throughput, packet delay, jitter, and packet loss.
- Analyse the impact of wireless network congestion on healthcare telemetry.
- Generate graphs and result files for performance comparison.

## Tools and Technologies

- NS-3.47 network simulator
- C++
- Ubuntu on Windows Subsystem for Linux (WSL)
- FlowMonitor
- Python and Matplotlib for result visualisation

## Experimental Setup

The experiments compare TCP and UDP at background traffic rates of 1 Mbps, 4 Mbps, and 8 Mbps.

The simulated network includes a healthcare traffic source, a wireless access point/gateway, and a background traffic source.

## Performance Metrics

- **Throughput:** Rate of data successfully received.
- **Packet Delay:** Time taken for packets to reach the destination.
- **Jitter:** Variation in packet delay.
- **Packet Loss:** Packets reported as lost during the simulation.

## Repository Contents

- `healthcare-wireless/` — NS-3 simulation source code.
- `run-tcp.sh` — Script to run TCP experiments.
- `run-udp.sh` — Script to run UDP experiments.
- `healthcare-final-results/` — CSV results, graphs, and result visualisation script.

## How to Run

Install and configure NS-3.47 first, then place the simulation source directory inside the NS-3 `scratch` directory.

Run the experiments from the NS-3.47 root directory using the supplied scripts after ensuring their paths and permissions are correct.

## Note

The simulation uses a Wi-Fi network to represent a healthcare telemetry scenario. It is a network simulation and does not represent testing on real medical devices or a dedicated Wireless Body Area Network standard.
