# Project Ezra

**Ezra — Space Automated Agent**  
**Mission:** ST-1 spacecraft operations and control

Ezra is a proposed onboard autonomous agent for the ST-1 space mission. Her primary purpose is to operate and manage the spacecraft, while ST-1's mission objective centers on Sentinel-Daemon monitoring and management.

## Identity and lineage

Ezra is a distinct agent derived from Digit's Core foundation. In their agent relationship, **Ezra is Digit's younger sister**. Ezra's mission is spaceflight; she does not inherit Digit's mission-specific modules.

## Architecture and ownership

- **STN-LABZ owns the Core.** Core-owned features, methods, and public APIs use the `stnlabz_` namespace.
- **Ezra owns her mission-specific modules.** Those modules extend the Core through its defined interfaces; they do not govern, replace, or modify Core.
- **Core responsibilities** include the agent foundation, identity validation, lifecycle/runtime, policy and authority evaluation, logging, and generic module infrastructure such as discovery, verification, qualification, activation, and inventory.
- **Mission capabilities** belong in independent Ezra modules, not as hardcoded service-specific behavior within Core.
- Agent architecture is intended to preserve its semantics across platform-specific implementations. Cross-platform implementation and spacecraft qualification remain unproven.

The repository contains an imported and adapted Core foundation. It does **not** represent a completed or flight-qualified spacecraft control system.

## ST-1 mission direction

The following are mission goals or subjects under investigation, **not claims of implemented capability**:

- Whole-spacecraft health and control, from onboard computing and storage to spacecraft structure
- Power generation, distribution, consumption, and thermal management; compact reactor concepts remain speculative
- Propulsion, propellant inventory, orbit management, commanded maneuvers, and potential deorbit operations
- Autonomous fault detection, classification, authorized mitigation, recovery, and escalation
- Telemetry collection, retention, and automatic ground synchronization when a supported connection is available
- On-demand communications, with Starlink connectivity under consideration and not established
- Sentinel-Daemon monitoring and management as the ST-1 mission objective
- Possible cameras for Earth imagery, spacecraft inspection, and observation of other orbital objects

Ground communication and command interfaces are under discussion. Safety-critical spacecraft actions require engineered authority boundaries, suitable flight hardware, validation, and operator authorization.

## Current engineering status

**Early Core adaptation / research stage.** The Core source has undergone identity and namespace cleanup, including removal of hardcoded IRC/Slack hotload notifications. Source changes are not evidence of successful compilation, integration, mission-readiness, or flight qualification.

Known open items include an unresolved `core_activation.h` dependency and the need for comprehensive build, interface, and behavioral validation. The ST-1 hardware, power source, propulsion architecture, communication services, and mission-module implementations are not established by this repository.

## Development authority

Discussion, research, and speculative planning are not authorization to implement. Human direction determines requirements, changes, validation, and acceptance. Proposed designs should be checked against previously established decisions and revalidated before implementation.
