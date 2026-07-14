# Historical Object Library Format

Each entry is a separate chunked `.object` file. The outer chunk is `0x7777`.
The canonical inner version chunk is `0x0900`, with build-1935 value `0x0010`.
Source-confirmed metadata chunks include flags `0x0903`, surfaces v1/v2/v3
`0x0905`/`0x0906`/`0x0907`, mesh container `0x0910`, library version `0x0911`,
bones `0x0913`/`0x0921`, motions `0x0916`, and motion references `0x0924`.

The first bridge decodes version, flags, library version, mesh child count,
surface count, motion presence, static/skeletal classification, and inert
texture/shader/material strings from confirmed surface layouts. Legacy v1
surface records are retained as partial. Unknown chunks are counted.

Object bounds are deliberately unavailable: build-1935 `UpdateBox()` derives
them after loading mesh payloads. Vertex/index data, meshes, textures,
shaders, motions, thumbnails, sounds, and renderer resources are never loaded.

