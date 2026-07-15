# wxSDKEditor Render Scene Submission

`EditorRenderScene` is a renderer-neutral list of `EditorRenderInstance`
records derived from the current editor model. Each record contains logical
path, normalized asset ID, transform, object-space bounds, readiness, selection,
visibility, and fallback state. It does not contain wxWidgets or graphics API
types.

`BuildEditorRenderScene` walks model order, resolves historical object
references against `EditorRenderAssetRegistry`, and attaches proven bounds and
readiness. The viewport renderer interface exposes `SubmitScene` as a default
no-op so existing software preview behavior remains compatible while a future
backend can consume the neutral submission.

The lawful production audit resolved all 15,416 SceneObject references and all
had registry bounds. Instances reference shared cached geometry by asset ID;
they never duplicate vertex/index arrays. The CPU wireframe backend consumes
these submissions while skeletal/missing/failed assets retain bounds fallback.
