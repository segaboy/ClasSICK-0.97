# Same endpoints, opposite output

Original engineering example,27October2026. X axis,invert0,button up.

```mermaid
%% SPDX-License-Identifier: GPL-3.0-or-later
%% Copyright (C) 2026 Dean Howell.
flowchart LR
  A["Start phase01"] --> B["Full sequence01→11→10→00"]
  B --> C["Emitted X unit−1"]
  A --> D["Endpoint-only01→00"]
  D --> E["Emitted X unit+1"]
  C --> F["Synthetic model example;not a hardware trace"]
  E --> F
```

Full-cadence output−1;endpoint-only output+1. Synthetic model units,not
a captured hardware trace,timing measurement or a release pass.

[Study and executed cases](../input-sampling-study.md). The original
[SVG generator](../../../tools/research/render_sampling_diagram.py) exports a
standalone illustration outside the repository for posting.

Export a new standalone SVG outside the repo (Python3 stdlib only):

```powershell
python -B tools/research/render_sampling_diagram.py --output C:/ClasSICK/endpoint-sampling.svg
```
