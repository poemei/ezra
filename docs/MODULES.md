# Ezra Modules

**Modules are not Core.** Core provides common identity, lifecycle, security/authority evaluation, generic routing/dispatch, logging, and module-management infrastructure. Each independently developed module owns its own behavior and registers its commands, routes, or services through approved interfaces. A module must not rewrite Core or the shared ABI to satisfy a module-specific requirement.

## Current module sources

| Module | Location | Responsibility | Evidence / status |
|---|---|---|---|
| IRC | `modules/irc/` | TLS IRC session, SASL authentication, channel messaging, and transport services | Operator observed Ezra joining IRC; subsequent versions require their own validation |
| Ollama | `modules/ollama/` | Local model generation and chat command/controller integration | Source ported; end-to-end operation not confirmed in the present record |
| Sentinel Observer | `modules/sentinel/` | Read Sentinel-Daemon log and quarantine *inventory*, classify changed observations, send summaries through IRC | Operator observed initial baseline; newer quarantine and four-hour behavior await full tests |

## Boundaries
- IRC owns transport; it does not make spacecraft control decisions.
- Ollama generates responses; it is not the authority for commands or the source of verified telemetry.
- Sentinel Observer observes `/var/log/sentinel/sentinel.log` and enumerates regular files directly beneath `/var/log/sentinel/quarantine`. It must not open or transmit quarantined payloads, modify Sentinel records, release quarantine, or enforce security actions.
- Sentinel-Daemon operates independently; loss or failure of Ezra must not prevent Sentinel or spacecraft controllers from carrying out their own assigned responsibilities.
- Module-to-module interaction uses Core-mediated services, not direct binding to another module's implementation.
- Only an explicit authorized interface may expose spacecraft commands. Observation and messaging do not imply command authority.

## Development workflow
From the repository root, for the named module:

```sh
make -C modules/sentinel clean all test
sudo make -C modules/sentinel install
```

Use the equivalent path for IRC or Ollama. **Do not install if testing fails.** Elevated privileges are for protected installation locations, not for running Ezra or compiling source. Ezra's runtime account is `stnchain` on the operator's current server.

For new module development, inspect the supplied `example.zip`, Module Creation Guide, AI Module Creation standard, and `AGENTS.md` as applicable. Preserve the approved MVC separation and operator-controlled acceptance gates.

## Known issues and limitations
- Initial Sentinel Observer revision omitted a standalone `test` target; later quarantine discovery work added a focused test target, whose execution and sufficiency require verification.
- Four-hour timing, reliable state persistence, log rotation, and changing quarantine inventory need fault-focused end-to-end checks.
- Installation under `/opt/ezra` must preserve appropriate writable-state ownership and protected credentials; the earlier state-permission defect was corrected manually by the operator.
- Ground operation is not evidence of spacecraft or radiation qualification.
