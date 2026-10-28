# Ezra Roadmap

**Authority:** Human project owner. A roadmap is not permission to implement a step.

## Foundation — in development
- Linux Core identity, lifecycle, mandatory logging, and generic module management.
- Keep Core unaware of individual modules' mission logic.
- Modules register and withdraw their own commands/routes and services through the established interfaces.
- Maintain the shared ABI as an independent dependency; do not introduce Ezra-specific changes there.

## Ground integration — active development
- IRC communications with secure authentication and auditable operator interaction.
- Ollama model interface as a separate module; validate actual requests and responses.
- Sentinel Observer: read-only host-log summaries at approximately four-hour intervals and change-only follow-ups.
- Sentinel quarantine inventory: detect new entries without opening or distributing quarantined payloads.
- Automated unit, negative, concurrency, and end-to-end tests; close known test and installer-ownership gaps.

## Hardware-in-the-loop — research
- Integrate an educational CubeSat and Linux single-board computer with real controller telemetry.
- Test power interruption, temperature, moisture, communications outages, restart, and component loss with independent evidence collection.
- Verify essential controller functions remain safe when Ezra is unavailable.
- Record environmental conditions, hardware revisions, software versions, observations, faults, root causes, repairs, and retests.

## ST-1 flight research — not established
- Define flight hardware, power, communications, radiation/thermal constraints, watchdogs, and independent spacecraft-control safety boundaries.
- Keep Sentinel-Daemon's enforcement and recovery authority distinct from Ezra's AI functions.
- Demonstrate controlled operation during loss of ground communications and loss of individual modules.
- Obtain appropriate independent review, qualification evidence, and human authorization before flight commitments.

## Acceptance gates
1. Authorized scope and revision identified.
2. Build and automated tests recorded.
3. Module qualification and activation recorded separately.
4. End-to-end behavior observed in the operator-selected client.
5. Explicit operator acceptance documented.
6. Next step begins only on separate operator authorization.
