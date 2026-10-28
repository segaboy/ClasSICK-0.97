# Windows CI profile analyzer: original bounded artifact

Windows11/PowerShell project ClasSICK0.97,baseline8cf6df67d1a416a7d2e011940860272c11107467.
Owner authorizes profiling/shortening slowWindowsCI without removing ANY required
check;desktop/VM/emulator/reference/hardware tests remain PAUSED. Codex alone
edits/integrates/executes. Existing reviewed-public/original-only worker scope.
ZERO tools/commands/edits/tests/web/delegation;return stdout proposals only.

One primary deliverable:usable standard-library Python3 profile analyzer plus
meaningful synthetic tests,not a general framework. JSON response keys:
files ({repo-relative-path:complete-text}),review (<=250words),tests_executed:[],
blockers:[],decisions_needed:[]. At most2 files,total360 changed lines.
Paths scripts/Profile-WindowsCI.py and tests/ci-profile.py. GPL-3.0-or-later,
copyright2026DeanHowell. Codex will integrate/test;do not claim proposed tests pass.

CLI --log FILE --start ISO_UTC --end ISO_UTC --output FILE. Parse original GH CLI
raw job log whose step label may be UNKNOWN STEP;timestamps determine selected
half-open interval. Handle7fraction digits,Z,monotonic lines,blank/noisy lines.
No network/subprocess/third-party packages. Refuse missing/invalid bounds,empty
selection,existing output and malformed/incomplete CTest groups;never report
success from only exit0/100% summary. Each Test project path opens a profile,
per-case Test #... Passed/Failed seconds lines identify executed names,
100%/Nfailed tests outofN and Total Test time(real)=Xs close it. Preserve failed
and skipped/error evidence;nonpassing profile returns nonzero,report diagnostic
JSON if structurally complete. Do not conflate summed case durations with elapsed
wall time under parallel execution. Group complete repeated profile paths safely
or refuse ambiguity;do not mix counts/duplicate names within a single group.
Output schema_version,source sha256,bounds,complete profile name/path/counts,
case-name/outcome identity hash,reported realCTestseconds,topslowcases,
totalreportedCTestwallseconds,selectedintervalseconds,unattributedseconds.
Unattributed includes builds/config/objectaudits;it is NOT proven waste.
No claim of historical compatibility,speedup,duplicate checks orquota saving.

Tests:rawGH prefixes/UNKNOWN STEP,2profiles,differentpathseparators,sequential/
out-of-order testcase completion timestamps stillmonotoniclog;failed/missingcase,
no100%/noend,totalcountmismatch,duplicatecase,existing-output refusal,validparallel
case-seconds sum > realelapsed notclassified as error,invalidbounds. Use tempdir.
Exact synthetic cases should verify expected math and preserve failed evidence.

Separately review metadata below for obvious scheduling hazards of a bounded
CTEST_PARALLEL_LEVEL=2 experiment:preserve ALL9profiles,alltests,debugtwins,
sanitizer defect-controls,object/bootmedia/fault audits;no caches/skips/--rerun-failed.
Lead verified pinned CMake4.4.4 prose:the envvar sets concurrentCTestjobs unless
explicit--parallel/presetjobs overrides. No such overrides in two current calls.
Use scope-restored envvar in the one CIstep;CTest honors DEPENDS and FIXTURES.
No currently declared RUN_SERIAL orRESOURCE_LOCK. Test catalog is discovery;
absence of locks does not prove safety. Missing built executables may mean empty
commands in this configure-only catalog. Do not infer test independence from that.
Recommend targeted shared-output review and comparable sequential/2job runs.

Baseline successful run40586299701,Windowsjob122478458200,sourceebfbfb9. Slowstage
21:27:54Z..21:41:48Z=834s.9profiles/3325cases;reportedrealCTestsum409.48s approx;
~49%wall. No removedchecks or2jobmeasurement yet. Currentsource unchanged in
relevant runtime/tests since thisrun exceptinstructiondocuments. Actual markers
and2case examples follow;input is original project evidence,notmanualcode.

Exact selected catalog/log marker identities and capture metadata are in ../windows-ci-profile.json;the frozen fullpacket/transcript stay outsideGit.
