# wxSDKEditor automatic scene assets

After a historical level opens successfully, normalized SceneObject references
enter the render scene. Configuration/index loading rebuilds that scene,
resolves and deduplicates references, and creates the active working set
automatically. No Load click is required after initial configuration.

The index may describe all entries, but CPU decode requests are only for static
assets in the active scene. D3D11 uploads are limited to four per frame and only
for visible/selected current-scene instances. Missing references use fallbacks,
malformed objects are isolated, and skeletal objects remain deferred. Converted
and reopened v5 snapshots use the same refresh path without dirty/history
changes.
