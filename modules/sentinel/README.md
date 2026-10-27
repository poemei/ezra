# Ezra Sentinel Observer 1.0.0

Read-only observation of Sentinel-Daemon's host log at
/var/log/sentinel/sentinel.log.

- Initial baseline goes to IRC when its transport service is available.
- Checks every four hours; unchanged routine scan results remain silent.
- Follow-up reports summarize nonvolatile file changes, process exits, warning/error severity,
  and coverage-error changes. They are observations, not threat determinations.
- Routine changes under /run/, /var/log/, /tmp/ and process-count fluctuations alone
  are intentionally ignored.
- The scan cursor and last acknowledged baseline are kept in ./state/sentinel.state.
- Log rotation is handled by resynchronizing to the new log. Historical lines in the
  rotated-in file are not treated as fresh alerts.
- Failure to read the log denies module start; failed IRC delivery does not acknowledge
  the checkpoint. On delivery failure, the module retries in 60 seconds.
- Sentinel monitoring and enforcement remain completely independent of Ezra.

Build: make -C modules/sentinel
Install: sudo make -C modules/sentinel install PREFIX=/opt/ezra MODULE_USER=... MODULE_GROUP=...

The Ezra service user requires read-only access to the log and write access to
/opt/ezra/state. Do not make Sentinel logs world-readable.

Acceptance: verify initial IRC report, no repeated identical report, changed-event
report after a four-hour interval, persistence after service restart, and
negative cases for unreadable log and unavailable IRC. Qualification is not
flight readiness or operator acceptance.
