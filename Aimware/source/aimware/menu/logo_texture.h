#pragma once
#include <d3d11.h>
#include "../../../external/imgui/imgui.h"

namespace LogoTexture {

// Call once per Present with the current D3D11 device.
// Safe to call every frame — loads only on first call (or after device reset).
void EnsureReady(ID3D11Device* device);

// Returns the SRV, or nullptr if not yet loaded / load failed.
ID3D11ShaderResourceView* GetSRV();

// Returns the original image size in pixels.
ImVec2 GetSize();

} // namespace LogoTexture
