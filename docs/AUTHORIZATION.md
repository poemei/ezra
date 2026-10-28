# Ezra Authorization

## Governing authority
The human developer and mission operator retain authority for development scope, deployment, accepted behavior, and spacecraft mission decisions. Ezra's autonomy is bounded by explicitly authorized duties. Observing a condition, generating text, passing qualification, or possessing a communications channel does **not** authorize changing spacecraft state.

## System boundaries
- **Core:** identity validation, generic module lifecycle, routing/dispatch infrastructure, mandatory logs, and common authorization boundary.
- **Modules:** independent capabilities that publish/withdraw routes or services through Core-provided interfaces. Modules cannot assume authority merely because they are loaded.
- **Sentinel-Daemon:** separate monitoring, enforcement, and recovery responsibilities. Ezra may report authorized Sentinel observations but must not silently replace or override Sentinel.
- **Spacecraft controllers:** safety-critical functions and independent recovery mechanisms must not depend on Ezra being continuously available.
- **Shared ABI:** common cross-project interface; Ezra-specific work does not authorize changes to `stnlabz/ABI`.

## Observer authorization
The Sentinel Observer is authorized in its stated design to read Sentinel's designated log and inventory quarantine metadata, retain a local read checkpoint, and produce change-based reports over an available authorized IRC transport. **It is not authorized** to read or publish quarantine file contents, change quarantine state, remove files, enforce sanctions, or issue controller commands.

IRC messages and language-model output are not automatically trusted instructions. External input requires appropriate origin verification, authorization, bounds checking, and audit evidence before any action.

## Changes and acceptance
1. The operator names and authorizes a change.
2. The implementation remains within that scope and respects Core/module boundaries.
3. Tests and failures are retained as evidence; qualification and activation are separate.
4. The operator performs end-to-end evaluation and explicitly accepts or rejects the revision.
5. Additional features or changes require new operator authorization.

Never claim operator acceptance, flight qualification, or legal certification from a successful build, log event, or AI interpretation. Preserve evidence of failures and correct demonstrated root causes rather than masking symptoms.

## Flight-specific controls
Loss of ground communications must not automatically invalidate legitimate onboard continuity, nor grant Ezra additional authority. Critical decisions, overrides, updates, and recovery sequences require project-defined policy, verification, and human-approved authorization boundaries before deployment. Final flight controls remain to be designed and validated.
