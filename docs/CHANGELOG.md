# Ezra Changelog

## 2026-10-28 — Documentation foundation
- Established separate records for changes, future direction, modules, and authorization.
- Documentation only; no Core or module behavior changed.

## 2026-10-28 — Sentinel Observer quarantine inventory
- Added read-only inventory of `/var/log/sentinel/quarantine` on the Sentinel review cycle.
- Added persisted quarantine snapshot and unit-test target for discovery and deduplication.
- Operator build, test, hotload, and follow-up notification acceptance have not been recorded for this revision.

## 2026-10-27 — Sentinel Observer 1.0.0
- Introduced four-hour log review of `/var/log/sentinel/sentinel.log`, change-triggered IRC reports, and persisted scan baseline.
- Operator observed an initial IRC baseline report from `st1_ezra`: entries=101098, processes=124, coverage_errors=0.
- Initial activation failed because `/opt/ezra/state` was not writable by `stnchain`; operator corrected permissions and observed the report.
- Four-hour interval, report suppression, rotation, and all failure cases remain unaccepted without end-to-end evidence.

## 2026-10-27 — Core and connectivity
- Adapted Digit's IRC and Ollama modules for Ezra-specific namespaces and paths.
- Added `/opt/ezra` installation and conditional systemd unit creation that preserves an existing unit.
- Aligned Ezra's module-host routing and hotload behavior with Digit's newer Core implementation.
- Operator confirmed Ezra's IRC connection after correcting the configured NickServ account name.
- Ollama end-to-end behavior has not been independently established here.

## Earlier — Core foundation
- Established Ezra's Linux Core, Agent Root, runtime, and Rictus-derived module-management integration.
- Operator confirmed successful Core compilation and launch.
- Shared STN-LABZ ABI remains a separate dependency; Ezra-specific changes do not justify modifying it.

## Evidence rule
Entries distinguish committed source, automated tests, runtime qualification, observed client behavior, and explicit operator acceptance. A successful build or `QUALIFICATION_GREEN` does not establish mission or flight qualification.
