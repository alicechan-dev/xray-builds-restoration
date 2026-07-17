# wxSDKEditor active scene working set

The working set is renderer-neutral session state keyed by normalized asset ID.
It records instance count, selection priority, availability, static eligibility,
skeletal deferral, and missing references. It rebuilds whenever the active
render scene changes and never enters snapshot output.

Switching documents replaces render instances immediately. Bounded CPU/GPU
caches may retain reusable assets, but stale instances, markers, labels, and
selection do not survive scene replacement. Decode happens before drawing; the
frame performs bounded uploads, visibility submission, shadow pass, shaded
pass, optional wireframe, world overlay, labels, and one Present.
